#include "entry.hpp"
#include "core/pch.hpp"
#include "core/hooks/vmt_hook.hpp"
#include "core/hooks/d3d11_hook.hpp"
#include "core/memory/pattern_scanner.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"
#include "core/sdk/interfaces.hpp"
#include "features/rage/rage_aimbot.hpp"
#include "features/legit/triggerbot.hpp"
#include "features/legit/legit_aa.hpp"
#include "features/legit/movement.hpp"
#include "features/visuals/esp.hpp"
#include "features/visuals/esp.hpp"
#include "features/visuals/esp.hpp"
#include "features/misc/misc.hpp"
#include "features/misc/misc.hpp"
#include "features/lua/lua_manager.hpp"
#include "gui/vice_theme.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_fonts.hpp"
#include "gui/vice_render.hpp"
#include "gui/tabs/ragebot_tab.hpp"
#include "gui/tabs/legitbot_tab.hpp"
#include "gui/tabs/visuals_tab.hpp"
#include "gui/tabs/misc_tab.hpp"
#include "gui/tabs/config_tab.hpp"
#include "gui/tabs/lua_tab.hpp"
#include "gui/menu_manager.hpp"
#include "resolver/resolver.hpp"
#include "resolver/resolver.hpp"
#include "resolver/resolver.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "skinchanger_system/case_opening.hpp"
#include "skinchanger_system/sticker_tool.hpp"
#include "skinchanger_system/inspect_panel.hpp"
#include "skinchanger_system/music_kit_manager.hpp"
#include "skinchanger_system/agent_manager.hpp"
#include "skinchanger_system/glove_manager.hpp"
#include "skinchanger_system/medal_manager.hpp"
#include "skinchanger_system/statrak_manager.hpp"
#include "skinchanger_system/rarity_glow.hpp"
#include "skinchanger_system/pattern_seed.hpp"
#include "skinchanger_system/economy_sim.hpp"

namespace core {

// Global state
static bool g_unloadRequested = false;
static HANDLE g_mainThread = nullptr;
static DWORD g_mainThreadId = 0;
static HMODULE g_moduleHandle = nullptr;

// Forward declarations
DWORD WINAPI MainThread(LPVOID lpParam);
void InitializeCheat();
void ShutdownCheat();
void OnUnload();

// Entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            g_moduleHandle = hModule;
            
            // Create main thread
            g_mainThread = CreateThread(nullptr, 0, MainThread, hModule, 0, &g_mainThreadId);
            if (g_mainThread) {
                CloseHandle(g_mainThread);
            }
            break;
        }
        case DLL_PROCESS_DETACH: {
            if (!lpReserved) { // Not process termination
                g_unloadRequested = true;
                OnUnload();
            }
            break;
        }
    }
    return TRUE;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = static_cast<HMODULE>(lpParam);
    
    // Wait for game to initialize
    while (!GetModuleHandleA("client.dll") || !GetModuleHandleA("engine2.dll") || 
           !GetModuleHandleA("server.dll") || !GetModuleHandleA("vstdlib.dll")) {
        Sleep(100);
        if (g_unloadRequested) return 0;
    }
    
    // Additional wait for renderer
    while (!GetModuleHandleA("d3d11.dll") || !GetModuleHandleA("dxgi.dll")) {
        Sleep(100);
        if (g_unloadRequested) return 0;
    }
    
    Sleep(1000); // Extra safety
    
    try {
        InitializeCheat();
    } catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "ViceCity Initialization Error", MB_ICONERROR);
    }
    
    // Keep thread alive until unload
    while (!g_unloadRequested) {
        Sleep(100);
    }
    
    ShutdownCheat();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

void InitializeCheat() {
    // 1. Initialize logger first
    core::logger::Logger::Instance().Initialize();
    LOG_INFO(Core, "ViceCity CS2 Cheat initializing...");
    LOG_INFO(Core, "Build: {}", __TIMESTAMP__);
    
    // 2. Initialize interfaces
    if (!sdk::interfaces::Initialize()) {
        LOG_CRITICAL(Core, "Failed to initialize interfaces!");
        throw std::runtime_error("Interface initialization failed");
    }
    LOG_INFO(Core, "Interfaces initialized");
    
    // 3. Initialize memory/patterns
    if (!core::memory::InitializeSignatures()) {
        LOG_CRITICAL(Core, "Failed to initialize signatures!");
        throw std::runtime_error("Signature initialization failed");
    }
    LOG_INFO(Core, "Signatures initialized");
    
    // 4. Initialize netvars
    if (!core::memory::NetvarManager::Instance().Initialize()) {
        LOG_CRITICAL(Core, "Failed to initialize netvars!");
        throw std::runtime_error("Netvar initialization failed");
    }
    LOG_INFO(Core, "Netvars initialized");
    
    // 5. Initialize config
    if (!core::config::ConfigManager::Instance().Initialize()) {
        LOG_CRITICAL(Core, "Failed to initialize config!");
        throw std::runtime_error("Config initialization failed");
    }
    core::config::ConfigManager::Instance().Load("default");
    LOG_INFO(Core, "Config initialized");
    
    // 6. Initialize hooks
    if (!core::hooks::HookManager::Instance().Initialize()) {
        LOG_CRITICAL(Core, "Failed to initialize hooks!");
        throw std::runtime_error("Hook initialization failed");
    }
    LOG_INFO(Core, "Hooks initialized");
    
    // 7. Initialize D3D11 hook
    if (!core::hooks::D3D11Hook::Instance().Initialize()) {
        LOG_CRITICAL(Core, "Failed to initialize D3D11 hook!");
        throw std::runtime_error("D3D11 hook initialization failed");
    }
    LOG_INFO(Core, "D3D11 hook initialized");
    
    // 8. Initialize ImGui
    if (!core::hooks::imgui_d3d11::Initialize(
        core::hooks::D3D11Hook::Instance().GetDevice(),
        core::hooks::D3D11Hook::Instance().GetContext()
    )) {
        LOG_CRITICAL(Core, "Failed to initialize ImGui D3D11!");
        throw std::runtime_error("ImGui D3D11 initialization failed");
    }
    
    if (!core::hooks::imgui_win32::Initialize(FindWindowA("SDL_window", nullptr))) {
        LOG_CRITICAL(Core, "Failed to initialize ImGui Win32!");
        throw std::runtime_error("ImGui Win32 initialization failed");
    }
    LOG_INFO(Core, "ImGui initialized");
    
    // 9. Initialize GUI theme and widgets
    gui::ViceTheme::Instance().Initialize();
    gui::ViceWidgets::Instance().Initialize();
    gui::ViceFonts::Instance().Initialize();
    gui::ViceRender::Instance().Initialize();
    LOG_INFO(Core, "GUI system initialized");
    
    // 10. Initialize features
    features::rage::RageAimbot::Instance().Initialize();
    features::legit::Triggerbot::Instance().Initialize();
    features::legit::Backtrack::Instance().Initialize();
    features::legit::LegitAA::Instance().Initialize();
    features::legit::Movement::Instance().Initialize();
    features::visuals::ESP::Instance().Initialize();
    features::visuals::Chams::Instance().Initialize();
    features::visuals::World::Instance().Initialize();
    features::visuals::Radar::Instance().Initialize();
    features::visuals::Effects::Instance().Initialize();
    features::misc::MovementEx::Instance().Initialize();
    features::misc::Logs::Instance().Initialize();
    features::misc::Skinchanger::Instance().Initialize();
    features::misc::InventoryUI::Instance().Initialize();
    LOG_INFO(Core, "Features initialized");
    
    // 11. Initialize resolver
    resolver::Resolver::Instance().Initialize();
    resolver::AnimFix::Instance().Initialize();
    resolver::LagComp::Instance().Initialize();
    LOG_INFO(Core, "Resolver initialized");
    
    // 12. Initialize skinchanger system
    skinchanger::InventoryCore::Instance().Initialize();
    skinchanger::CaseOpening::Instance().Initialize();
    skinchanger::StickerTool::Instance().Initialize();
    skinchanger::InspectPanel::Instance().Initialize();
    skinchanger::MusicKitManager::Instance().Initialize();
    skinchanger::AgentManager::Instance().Initialize();
    skinchanger::GloveManager::Instance().Initialize();
    skinchanger::MedalManager::Instance().Initialize();
    skinchanger::StatTrakManager::Instance().Initialize();
    skinchanger::RarityGlow::Instance().Initialize();
    skinchanger::PatternSeed::Instance().Initialize();
    skinchanger::EconomySim::Instance().Initialize();
    LOG_INFO(Core, "Skinchanger system initialized");
    
    // 13. Initialize Lua
    features::lua::LuaManager::Instance().Initialize();
    LOG_INFO(Core, "Lua system initialized");
    
    // 14. Initialize GUI tabs
    gui::RageBotTab::Instance().Initialize();
    gui::LegitBotTab::Instance().Initialize();
    gui::VisualsTab::Instance().Initialize();
    gui::MiscTab::Instance().Initialize();
    gui::ConfigTab::Instance().Initialize();
    gui::LuaTab::Instance().Initialize();
    gui::MenuManager::Instance().Initialize();
    LOG_INFO(Core, "GUI tabs initialized");
    
    // 15. Register D3D11 callbacks
    core::hooks::D3D11Hook::Instance().AddPresentCallback([](IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
        // ImGui new frame
        core::hooks::imgui_d3d11::NewFrame();
        core::hooks::imgui_win32::NewFrame();
        ImGui::NewFrame();
        
        // Render menu
        gui::MenuManager::Instance().Render();
        
        // Render features (ESP, etc.)
        features::visuals::ESP::Instance().Render();
        features::visuals::Radar::Instance().Render();
        features::visuals::Effects::Instance().Render();
        features::misc::InventoryUI::Instance().Render();
        
        // Lua render callbacks
        features::lua::LuaManager::Instance().OnRender();
        
        // ImGui render
        ImGui::EndFrame();
        ImGui::Render();
        core::hooks::imgui_d3d11::RenderDrawData(ImGui::GetDrawData());
    });
    
    core::hooks::D3D11Hook::Instance().AddResizeCallback([](IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        core::hooks::imgui_d3d11::InvalidateDeviceObjects();
        core::hooks::D3D11Hook::Instance().ReleaseRenderTarget();
        // Original resize will be called
        // After resize:
        core::hooks::D3D11Hook::Instance().CreateRenderTarget();
        core::hooks::imgui_d3d11::CreateDeviceObjects();
    });
    
    LOG_INFO(Core, "ViceCity CS2 Cheat fully initialized!");
    LOG_INFO(Core, "Press INSERT to open menu");
}

void ShutdownCheat() {
    LOG_INFO(Core, "Shutting down ViceCity...");
    
    // Shutdown in reverse order
    gui::MenuManager::Instance().Shutdown();
    gui::LuaTab::Instance().Shutdown();
    gui::ConfigTab::Instance().Shutdown();
    gui::MiscTab::Instance().Shutdown();
    gui::VisualsTab::Instance().Shutdown();
    gui::LegitBotTab::Instance().Shutdown();
    gui::RageBotTab::Instance().Shutdown();
    
    features::lua::LuaManager::Instance().Shutdown();
    
    skinchanger::EconomySim::Instance().Shutdown();
    skinchanger::PatternSeed::Instance().Shutdown();
    skinchanger::RarityGlow::Instance().Shutdown();
    skinchanger::StatTrakManager::Instance().Shutdown();
    skinchanger::MedalManager::Instance().Shutdown();
    skinchanger::GloveManager::Instance().Shutdown();
    skinchanger::AgentManager::Instance().Shutdown();
    skinchanger::MusicKitManager::Instance().Shutdown();
    skinchanger::InspectPanel::Instance().Shutdown();
    skinchanger::StickerTool::Instance().Shutdown();
    skinchanger::CaseOpening::Instance().Shutdown();
    skinchanger::InventoryCore::Instance().Shutdown();
    
    resolver::LagComp::Instance().Shutdown();
    resolver::AnimFix::Instance().Shutdown();
    resolver::Resolver::Instance().Shutdown();
    
    gui::ViceRender::Instance().Shutdown();
    gui::ViceFonts::Instance().Shutdown();
    gui::ViceWidgets::Instance().Shutdown();
    gui::ViceTheme::Instance().Shutdown();
    
    core::hooks::imgui_win32::Shutdown();
    core::hooks::imgui_d3d11::Shutdown();
    
    features::misc::InventoryUI::Instance().Shutdown();
    features::misc::Skinchanger::Instance().Shutdown();
    features::misc::Logs::Instance().Shutdown();
    features::misc::MovementEx::Instance().Shutdown();
    features::visuals::Effects::Instance().Shutdown();
    features::visuals::Radar::Instance().Shutdown();
    features::visuals::World::Instance().Shutdown();
    features::visuals::Chams::Instance().Shutdown();
    features::visuals::ESP::Instance().Shutdown();
    features::legit::Movement::Instance().Shutdown();
    features::legit::LegitAA::Instance().Shutdown();
    features::legit::Backtrack::Instance().Shutdown();
    features::legit::Triggerbot::Instance().Shutdown();
    features::rage::RageAimbot::Instance().Shutdown();
    
    core::hooks::HookManager::Instance().Shutdown();
    core::hooks::D3D11Hook::Instance().Shutdown();
    
    core::config::ConfigManager::Instance().Save("default");
    core::config::ConfigManager::Instance().Shutdown();
    
    core::memory::NetvarManager::Instance().Shutdown();
    sdk::interfaces::Shutdown();
    
    core::logger::Logger::Instance().Shutdown();
    
    LOG_INFO(Core, "ViceCity shutdown complete");
}

void OnUnload() {
    g_unloadRequested = true;
    
    // Wait for main thread to finish
    if (g_mainThread) {
        WaitForSingleObject(g_mainThread, 5000);
    }
}

} // namespace core