#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <mutex>
#include <vector>
#include <functional>
#include "../sdk/structs.hpp"
#include <imgui.h>
#include "vmt_hook.hpp"

namespace core::hooks {

using Microsoft::WRL::ComPtr;

// D3D11 Hook for Present/ResizeBuffers
class D3D11Hook {
public:
    using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
    using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
    
    static D3D11Hook& Instance() {
        static D3D11Hook instance;
        return instance;
    }
    
    bool Initialize();
    void Shutdown();
    
    bool IsInitialized() const { return m_initialized; }
    
    // Original functions
    PresentFn GetOriginalPresent() const { return m_oPresent; }
    ResizeBuffersFn GetOriginalResizeBuffers() const { return m_oResizeBuffers; }
    
    // Device access
    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
    IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }
    
    // Render target
    ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }
    void CreateRenderTarget();
    void ReleaseRenderTarget();
    
    // ImGui integration
    void NewFrame();
    void EndFrame();
    void Render();
    
    // Callback registration
    using PresentCallback = std::function<void(IDXGISwapChain*, UINT, UINT)>;
    using ResizeCallback = std::function<void(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
    
    void AddPresentCallback(PresentCallback cb) { m_presentCallbacks.push_back(cb); }
    void AddResizeCallback(ResizeCallback cb) { m_resizeCallbacks.push_back(cb); }
    void ClearCallbacks() { m_presentCallbacks.clear(); m_resizeCallbacks.clear(); }
    
    // Hook control
    void SetPresentEnabled(bool enabled) { m_presentEnabled = enabled; }
    void SetResizeEnabled(bool enabled) { m_resizeEnabled = enabled; }

private:
    D3D11Hook() = default;
    ~D3D11Hook() = default;
    
    bool CreateDeviceAndSwapChain();
    bool HookSwapChain();
    void UnhookSwapChain();
    
    // Hook functions
    static HRESULT __stdcall hkPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
    static HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
    
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    
    std::shared_ptr<VMTHook> m_swapChainHook;
    PresentFn m_oPresent = nullptr;
    ResizeBuffersFn m_oResizeBuffers = nullptr;
    
    std::vector<PresentCallback> m_presentCallbacks;
    std::vector<ResizeCallback> m_resizeCallbacks;
    
    bool m_initialized = false;
    bool m_presentEnabled = true;
    bool m_resizeEnabled = true;
    std::mutex m_mutex;
};

// D3D11 Render utilities
namespace d3d11 {
    
    // Create shader resource view from memory
    ID3D11ShaderResourceView* CreateTextureFromMemory(ID3D11Device* device, const void* data, size_t size, int& outWidth, int& outHeight);
    ID3D11ShaderResourceView* CreateTextureFromFile(ID3D11Device* device, const wchar_t* path, int& outWidth, int& outHeight);
    
    // Font creation
    IDWriteFactory* GetDWriteFactory();
    IDWriteTextFormat* CreateTextFormat(IDWriteFactory* factory, const wchar_t* fontFamily, float size, DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL);
    
    // Drawing helpers
    void DrawRect(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv, float x, float y, float w, float h, const float color[4]);
    void DrawLine(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv, float x1, float y1, float x2, float y2, float thickness, const float color[4]);
    void DrawCircle(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv, float x, float y, float radius, int segments, const float color[4], float thickness = 1.0f);
    void DrawFilledCircle(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv, float x, float y, float radius, int segments, const float color[4]);
    void DrawText(ID3D11DeviceContext* ctx, IDWriteTextFormat* format, ID3D11RenderTargetView* rtv, const wchar_t* text, float x, float y, const float color[4]);
    
    // World to screen using D3D
    bool WorldToScreen(const sdk::Vector3D& world, sdk::Vector2D& screen, const sdk::Matrix4x4& viewMatrix, int width, int height);
    
    // View matrix helpers
    sdk::Matrix4x4 GetViewMatrix();
    sdk::Matrix4x4 GetProjectionMatrix();
    sdk::Matrix4x4 GetViewProjectionMatrix();
    
} // namespace d3d11

// ImGui D3D11 initialization (called from D3D11Hook::NewFrame)
namespace imgui_d3d11 {
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();
    void NewFrame();
    void EndFrame();
    void RenderDrawData(ImDrawData* drawData);
    void CreateDeviceObjects();
    void InvalidateDeviceObjects();
}

// ImGui Win32 initialization
namespace imgui_win32 {
    bool Initialize(HWND hwnd);
    void Shutdown();
    void NewFrame();
    LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

} // namespace core::hooks