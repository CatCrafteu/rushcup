#pragma once

#include <imgui.h>
#include <string>
#include <functional>
#include <mutex>
#include "vice_theme.hpp"
#include "vice_widgets.hpp"
#include "vice_fonts.hpp"
#include "vice_render.hpp"
#include "tabs/ragebot_tab.hpp"
#include "tabs/legitbot_tab.hpp"
#include "tabs/visuals_tab.hpp"
#include "tabs/misc_tab.hpp"
#include "tabs/config_tab.hpp"
#include "tabs/lua_tab.hpp"

namespace gui {

class MenuManager {
public:
    static MenuManager& Instance() {
        static MenuManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void Toggle();
    void SetOpen(bool open);
    bool IsOpen() const { return m_open; }
    
    // Menu keybind
    void SetMenuKey(int key) { m_menuKey = key; }
    int GetMenuKey() const { return m_menuKey; }
    
    // DPI scaling
    void SetDPIScale(float scale);
    float GetDPIScale() const { return m_dpiScale; }
    
    // Animation state
    float GetMenuAlpha() const { return m_menuAlpha; }
    float GetMenuSlide() const { return m_menuSlide; }
    
    // Callbacks
    using MenuCallback = std::function<void(bool)>;
    void OnMenuToggle(MenuCallback cb) { m_onToggleCallbacks.push_back(cb); }
    
    // Watermark
    void RenderWatermark();
    void SetWatermarkVisible(bool visible) { m_showWatermark = visible; }
    bool IsWatermarkVisible() const { return m_showWatermark; }
    
    // Spectator list
    void RenderSpectatorList();
    void SetSpectatorListVisible(bool visible) { m_showSpectatorList = visible; }
    bool IsSpectatorListVisible() const { return m_showSpectatorList; }
    
    // Keybinds display
    void RenderKeybinds();
    void SetKeybindsVisible(bool visible) { m_showKeybinds = visible; }
    bool IsKeybindsVisible() const { return m_showKeybinds; }
    
    // FPS/Performance
    void RenderPerformanceOverlay();
    void SetPerformanceOverlayVisible(bool visible) { m_showPerformance = visible; }
    bool IsPerformanceOverlayVisible() const { return m_showPerformance; }

private:
    MenuManager() = default;
    
    bool m_open = false;
    bool m_wasOpen = false;
    int m_menuKey = VK_INSERT;
    float m_dpiScale = 1.0f;
    float m_menuAlpha = 0.0f;
    float m_menuSlide = 0.0f;
    float m_animationSpeed = 8.0f;
    
    // Overlays
    bool m_showWatermark = true;
    bool m_showSpectatorList = false;
    bool m_showKeybinds = false;
    bool m_showPerformance = false;
    
    // Window state
    ImVec2 m_windowPos = {100, 100};
    ImVec2 m_windowSize = {800, 600};
    bool m_windowCollapsed = false;
    
    // Tabs
    enum class MainTab { RageBot, LegitBot, Visuals, Misc, Config, Lua };
    MainTab m_currentTab = MainTab::RageBot;
    
    // Callbacks
    std::vector<MenuCallback> m_onToggleCallbacks;
    std::mutex m_mutex;
    
    // Theme refs
    gui::ViceTheme& m_theme = gui::ViceTheme::Instance();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
    gui::ViceFonts& m_fonts = gui::ViceFonts::Instance();
    gui::ViceRender& m_render = gui::ViceRender::Instance();
    
    void HandleInput();
    void UpdateAnimations();
    void RenderMainWindow();
    void RenderTabBar();
    void RenderTabContent();
    void RenderOverlays();
    void ApplyWindowStyle();
    void HandleMenuKeybind();
    
    // Watermark
    void DrawWatermark(ImDrawList* drawList);
    std::string GetWatermarkText() const;
    
    // Spectator list
    void DrawSpectatorList(ImDrawList* drawList);
    
    // Keybinds
    void DrawKeybinds(ImDrawList* drawList);
    
    // Performance
    void DrawPerformanceOverlay(ImDrawList* drawList);
    
    // Tab rendering
    void RenderRageBotTab();
    void RenderLegitBotTab();
    void RenderVisualsTab();
    void RenderMiscTab();
    void RenderConfigTab();
    void RenderLuaTab();
};

} // namespace gui