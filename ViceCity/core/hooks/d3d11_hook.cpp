#include "core/pch.hpp"
#include "core/hooks/d3d11_hook.hpp"
#include "core/memory/pattern_scanner.hpp"
#include "core/logger/logger.hpp"

namespace core::hooks {



bool D3D11Hook::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) return true;
    
    // Wait for D3D11 device creation
    HWND hwnd = FindWindowA("SDL_window", nullptr);
    if (!hwnd) {
        hwnd = GetForegroundWindow();
    }
    
    if (!CreateDeviceAndSwapChain()) {
        LOG_ERROR(Hooks, "Failed to create D3D11 device and swap chain");
        return false;
    }
    
    if (!HookSwapChain()) {
        LOG_ERROR(Hooks, "Failed to hook swap chain");
        return false;
    }
    
    CreateRenderTarget();
    
    m_initialized = true;
    LOG_INFO(Hooks, "D3D11Hook initialized successfully");
    return true;
}

void D3D11Hook::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) return;
    
    UnhookSwapChain();
    ReleaseRenderTarget();
    
    m_swapChain.Reset();
    m_device.Reset();
    m_context.Reset();
    m_renderTargetView.Reset();
    
    m_initialized = false;
    LOG_INFO(Hooks, "D3D11Hook shutdown");
}

bool D3D11Hook::CreateDeviceAndSwapChain() {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = FindWindowA("SDL_window", nullptr);
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        nullptr,
        &m_context
    );
    
    if (FAILED(hr)) {
        LOG_ERROR(Hooks, "D3D11CreateDeviceAndSwapChain failed: 0x{:X}", hr);
        return false;
    }
    
    return true;
}

bool D3D11Hook::HookSwapChain() {
    if (!m_swapChain) return false;
    
    m_swapChainHook = std::make_shared<VMTHook>(m_swapChain.Get());
    if (!m_swapChainHook) return false;
    
    // Present is typically at index 8
    // ResizeBuffers is typically at index 13
    m_oPresent = m_swapChainHook->GetOriginal<PresentFn>(8);
    m_oResizeBuffers = m_swapChainHook->GetOriginal<ResizeBuffersFn>(13);
    
    if (!m_oPresent || !m_oResizeBuffers) {
        LOG_ERROR(Hooks, "Failed to get original Present/ResizeBuffers");
        return false;
    }
    
    if (!m_swapChainHook->Hook(8, reinterpret_cast<void*>(hkPresent))) {
        LOG_ERROR(Hooks, "Failed to hook Present");
        return false;
    }
    
    if (!m_swapChainHook->Hook(13, reinterpret_cast<void*>(hkResizeBuffers))) {
        LOG_ERROR(Hooks, "Failed to hook ResizeBuffers");
        return false;
    }
    
    LOG_INFO(Hooks, "Swap chain hooked successfully");
    return true;
}

void D3D11Hook::UnhookSwapChain() {
    if (m_swapChainHook) {
        m_swapChainHook->UnhookAll();
        m_swapChainHook.reset();
    }
    m_oPresent = nullptr;
    m_oResizeBuffers = nullptr;
}

HRESULT __stdcall D3D11Hook::hkPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
    auto& instance = D3D11Hook::Instance();
    
    if (instance.m_presentEnabled) {
        for (auto& cb : instance.m_presentCallbacks) {
            try {
                cb(swapChain, syncInterval, flags);
            } catch (...) {}
        }
    }
    
    return instance.m_oPresent(swapChain, syncInterval, flags);
}

HRESULT __stdcall D3D11Hook::hkResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
    auto& instance = D3D11Hook::Instance();
    
    if (instance.m_resizeEnabled) {
        for (auto& cb : instance.m_resizeCallbacks) {
            try {
                cb(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
            } catch (...) {}
        }
    }
    
    // Release our render target before resize
    instance.ReleaseRenderTarget();
    
    HRESULT hr = instance.m_oResizeBuffers(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
    
    // Recreate render target after resize
    if (SUCCEEDED(hr)) {
        instance.CreateRenderTarget();
    }
    
    return hr;
}

void D3D11Hook::CreateRenderTarget() {
    if (!m_swapChain || !m_device) return;
    
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        LOG_ERROR(Hooks, "Failed to get back buffer: 0x{:X}", hr);
        return;
    }
    
    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) {
        LOG_ERROR(Hooks, "Failed to create render target view: 0x{:X}", hr);
        return;
    }
    
    LOG_DEBUG(Hooks, "Render target created");
}

void D3D11Hook::ReleaseRenderTarget() {
    m_renderTargetView.Reset();
    LOG_DEBUG(Hooks, "Render target released");
}

void D3D11Hook::NewFrame() {
    // ImGui new frame is handled by callbacks
}

void D3D11Hook::EndFrame() {
    // ImGui end frame is handled by callbacks
}

void D3D11Hook::Render() {
    // ImGui render is handled by callbacks
}

} // namespace core::hooks

// ImGui D3D11 implementation
namespace core::hooks::imgui_d3d11 {

bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!device || !context) return false;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    // Docking and Viewports require ImGui 1.80+ - define flags if not present
    #ifndef ImGuiConfigFlags_DockingEnable
    #define ImGuiConfigFlags_DockingEnable (1 << 4)
    #endif
    #ifndef ImGuiConfigFlags_ViewportsEnable
    #define ImGuiConfigFlags_ViewportsEnable (1 << 5)
    #endif
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = "ViceCity/imgui.ini";
    io.LogFilename = "ViceCity/imgui_log.txt";
    
    // Setup style
    ImGui::StyleColorsDark();
    
    // Initialize backends
    if (!ImGui_ImplDX11_Init(device, context)) {
        LOG_ERROR(Hooks, "Failed to initialize ImGui DX11 backend");
        return false;
    }
    
    HWND hwnd = FindWindowA("SDL_window", nullptr);
    if (!ImGui_ImplWin32_Init(hwnd)) {
        LOG_ERROR(Hooks, "Failed to initialize ImGui Win32 backend");
        return false;
    }
    
    LOG_INFO(Hooks, "ImGui DX11 initialized");
    return true;
}

void Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    LOG_INFO(Hooks, "ImGui shutdown");
}

void NewFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void EndFrame() {
    ImGui::EndFrame();
}

void RenderDrawData(ImDrawData* drawData) {
    if (!drawData) return;
    
    auto& hook = D3D11Hook::Instance();
    if (!hook.GetRenderTargetView()) return;
    
    ImGui_ImplDX11_RenderDrawData(drawData);
}

void CreateDeviceObjects() {
    ImGui_ImplDX11_CreateDeviceObjects();
}

void InvalidateDeviceObjects() {
    ImGui_ImplDX11_InvalidateDeviceObjects();
}

} // namespace core::hooks::imgui_d3d11

// ImGui Win32 implementation
namespace core::hooks::imgui_win32 {

bool Initialize(HWND hwnd) {
    if (!hwnd) return false;
    
    if (!ImGui_ImplWin32_Init(hwnd)) {
        LOG_ERROR(Hooks, "Failed to initialize ImGui Win32 backend");
        return false;
    }
    
    LOG_INFO(Hooks, "ImGui Win32 initialized");
    return true;
}

void Shutdown() {
    ImGui_ImplWin32_Shutdown();
}

void NewFrame() {
    ImGui_ImplWin32_NewFrame();
}

// Forward declaration for ImGui Win32 WndProc handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

} // namespace core::hooks::imgui_win32