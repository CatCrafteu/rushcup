#include "core/pch.hpp"
#include "gui/menu_manager.hpp"
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
#include "core/sdk/interfaces.hpp"
#include "features/visuals/esp.hpp"
#include "features/visuals/esp.hpp"
#include "features/visuals/effects.hpp"
#include "features/misc/inventory_ui.hpp"
#include "features/lua/lua_manager.hpp"

namespace gui {

void MenuManager::Initialize() {
    // Load theme
    ViceTheme::Instance().ApplyTheme();
    
    // Initialize tabs
    RageBotTab::Instance().Initialize();
    LegitBotTab::Instance().Initialize();
    VisualsTab::Instance().Initialize();
    MiscTab::Instance().Initialize();
    ConfigTab::Instance().Initialize();
    LuaTab::Instance().Initialize();
    
    // Register toggle callback
    OnMenuToggle([this](bool open) {
        if (open) {
            // Focus menu
        }
    });
    
    LOG_INFO(GUI, "MenuManager initialized");
}

void MenuManager::Shutdown() {
    RageBotTab::Instance().Shutdown();
    LegitBotTab::Instance().Shutdown();
    VisualsTab::Instance().Shutdown();
    MiscTab::Instance().Shutdown();
    ConfigTab::Instance().Shutdown();
    LuaTab::Instance().Shutdown();
    
    LOG_INFO(GUI, "MenuManager shutdown");
}

void MenuManager::Render() {
    HandleInput();
    UpdateAnimations();
    
    if (!m_open && m_menuAlpha <= 0.0f) return;
    
    RenderMainWindow();
    RenderOverlays();
}

void MenuManager::Toggle() {
    SetOpen(!m_open);
}

void MenuManager::SetOpen(bool open) {
    if (m_open == open) return;
    
    m_open = open;
    m_wasOpen = open;
    
    for (auto& cb : m_onToggleCallbacks) {
        try {
            cb(open);
        } catch (...) {}
    }
    
    LOG_INFO(GUI, "Menu {}", open ? "opened" : "closed");
}

void MenuManager::SetMenuKey(int key) {
    m_menuKey = key;
}

void MenuManager::SetDPIScale(float scale) {
    m_dpiScale = std::clamp(scale, 0.5f, 3.0f);
    ViceFonts::Instance().UpdateDPIScale(m_dpiScale);
}

void MenuManager::HandleInput() {
    // Handle menu keybind
    static bool keyPressed = false;
    bool keyDown = (GetAsyncKeyState(m_menuKey) & 0x8000) != 0;
    
    if (keyDown && !keyPressed) {
        Toggle();
        keyPressed = true;
    } else if (!keyDown) {
        keyPressed = false;
    }
    
    // Handle menu dragging when open
    if (m_open && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        // Check if clicking on title bar area
        if (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
            mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + 35) {
            // Start drag (handled by ImGui)
        }
    }
}

void MenuManager::UpdateAnimations() {
    float targetAlpha = m_open ? 1.0f : 0.0f;
    float targetSlide = m_open ? 0.0f : 30.0f;
    
    float speed = m_animationSpeed * ImGui::GetIO().DeltaTime;
    m_menuAlpha = ImLerp(m_menuAlpha, targetAlpha, speed);
    m_menuSlide = ImLerp(m_menuSlide, targetSlide, speed);
}

void MenuManager::RenderMainWindow() {
    if (m_menuAlpha <= 0.0f) return;
    
    ApplyWindowStyle();
    
    ImGui::SetNextWindowPos(m_windowPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(m_windowSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(m_menuAlpha);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_MenuBar;
    
    if (!ImGui::Begin("ViceCity", &m_open, flags)) {
        ImGui::End();
        return;
    }
    
    // Save window state
    m_windowPos = ImGui::GetWindowPos();
    m_windowSize = ImGui::GetWindowSize();
    
    RenderTabBar();
    RenderTabContent();
    
    ImGui::End();
}

void MenuManager::ApplyWindowStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ViceTheme::Colors& colors = ViceTheme::Instance().GetColors();
    
    // Apply alpha to all colors
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        ImVec4& col = style.Colors[i];
        col.w *= m_menuAlpha;
    }
    
    // Window rounding
    style.WindowRounding = ViceTheme::Instance().GetStyle().WindowRounding;
    style.WindowBorderSize = ViceTheme::Instance().GetStyle().WindowBorderSize;
    style.WindowPadding = ViceTheme::Instance().GetStyle().WindowPadding;
    
    // Custom window background with gradient
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                           colors.BG_Primary, style.WindowRounding);
    
    // Top bar gradient
    ViceTheme::Instance().DrawGradientRect(drawList, pos, 
                                          ImVec2(pos.x + size.x, pos.y + 35),
                                          colors.PrimaryPink, colors.PrimaryYellow, true);
    
    // Border
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                     colors.Border_Pink, style.WindowRounding, ImDrawFlags_RoundCornersAll, 1.0f);
    
    // Shadow
    ViceRender::Instance().DrawLayeredShadow(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                                            style.WindowRounding, 4);
}

void MenuManager::RenderTabBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 10));
    
    ViceTheme::Colors& colors = ViceTheme::Instance().GetColors();
    
    // Custom tab bar background
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float tabBarHeight = 40;
    
    drawList->AddRectFilled(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + tabBarHeight),
                           colors.BG_Card, 0.0f, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
    
    // Tab buttons
    const char* tabNames[] = {"RageBot", "LegitBot", "Visuals", "Misc", "Config", "Lua"};
    MainTab tabs[] = {MainTab::RageBot, MainTab::LegitBot, MainTab::Visuals, MainTab::Misc, MainTab::Config, MainTab::Lua};
    
    float tabWidth = ImGui::GetContentRegionAvail().x / 6.0f;
    
    for (int i = 0; i < 6; ++i) {
        bool selected = m_currentTab == tabs[i];
        
        ImVec2 tabPos = ImVec2(pos.x + i * tabWidth, pos.y);
        ImVec2 tabMax = ImVec2(tabPos.x + tabWidth, tabPos.y + tabBarHeight);
        
        // Tab background
        if (selected) {
            drawList->AddRectFilled(tabPos, tabMax, colors.Tab_Active, 0.0f, 
                                   (i == 0 ? ImDrawFlags_RoundCornersTopLeft : 0) | 
                                   (i == 5 ? ImDrawFlags_RoundCornersTopRight : 0));
            
            // Active indicator
            drawList->AddRectFilled(
                ImVec2(tabPos.x, tabMax.y - 3),
                ImVec2(tabMax.x, tabMax.y),
                colors.PrimaryPink
            );
        } else if (ImGui::IsMouseHoveringRect(tabPos, tabMax)) {
            drawList->AddRectFilled(tabPos, tabMax, colors.Tab_Hover);
        } else {
            drawList->AddRectFilled(tabPos, tabMax, colors.Tab_Normal);
        }
        
        // Tab text
        ImVec2 textSize = ImGui::CalcTextSize(tabNames[i]);
        ImVec2 textPos = ImVec2(tabPos.x + (tabWidth - textSize.x) * 0.5f, tabPos.y + (tabBarHeight - textSize.y) * 0.5f);
        drawList->AddText(textPos, selected ? colors.Tab_Text_Active : colors.Tab_Text, tabNames[i]);
        
        // Click detection
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(tabPos, tabMax)) {
            m_currentTab = tabs[i];
        }
    }
    
    ImGui::Dummy(ImVec2(0, tabBarHeight));
    ImGui::PopStyleVar(2);
    
    // Separator
    ImGui::Separator();
}

void MenuManager::RenderTabContent() {
    ImGui::BeginChild("##tabcontent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
    
    switch (m_currentTab) {
        case MainTab::RageBot:
            RageBotTab::Instance().Render();
            break;
        case MainTab::LegitBot:
            LegitBotTab::Instance().Render();
            break;
        case MainTab::Visuals:
            VisualsTab::Instance().Render();
            break;
        case MainTab::Misc:
            MiscTab::Instance().Render();
            break;
        case MainTab::Config:
            ConfigTab::Instance().Render();
            break;
        case MainTab::Lua:
            LuaTab::Instance().Render();
            break;
    }
    
    ImGui::EndChild();
}

void MenuManager::RenderOverlays() {
    if (m_showWatermark) RenderWatermark();
    if (m_showSpectatorList) RenderSpectatorList();
    if (m_showKeybinds) RenderKeybinds();
    if (m_showPerformance) RenderPerformanceOverlay();
}

void MenuManager::RenderWatermark() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    
    std::string text = GetWatermarkText();
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    
    float padding = 12.0f;
    ImVec2 pos = ImVec2(viewportSize.x - textSize.x - padding * 2, padding);
    ImVec2 size = ImVec2(textSize.x + padding * 2, textSize.y + padding);
    
    ViceTheme::Colors& colors = ViceTheme::Instance().GetColors();
    
    // Background with gradient
    ViceTheme::Instance().DrawGradientRect(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y),
                                          colors.BG_Card, colors.BG_Active, true);
    
    // Border
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.PrimaryPink, 6.0f);
    
    // Text
    drawList->AddText(ImVec2(pos.x + padding, pos.y + padding * 0.5f), colors.Text_Main, text.c_str());
    
    // Accent line
    drawList->AddLine(
        ImVec2(pos.x, pos.y + size.y),
        ImVec2(pos.x + size.x, pos.y + size.y),
        colors.PrimaryPink, 2.0f
    );
}

std::string MenuManager::GetWatermarkText() const {
    return fmt::format("ViceCity v{} | {} fps", VICECITY_VERSION, static_cast<int>(ImGui::GetIO().Framerate));
}

void MenuManager::RenderSpectatorList() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    
    ImVec2 pos = ImVec2(20, viewportSize.y - 200);
    ImVec2 size = ImVec2(250, 180);
    
    ViceTheme::Colors& colors = ViceTheme::Instance().GetColors();
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.BG_Card, 6.0f);
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.Border_Dim, 6.0f);
    
    // Title
    drawList->AddText(ImVec2(pos.x + 12, pos.y + 8), colors.Text_Main, "Spectators");
    drawList->AddLine(ImVec2(pos.x + 8, pos.y + 30), ImVec2(pos.x + size.x - 8, pos.y + 30), colors.Border_Dim);
    
    // TODO: Add actual spectator logic
    drawList->AddText(ImVec2(pos.x + 12, pos.y + 40), colors.Text_Muted, "No spectators");
}

void MenuManager::RenderKeybinds() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    
    ImVec2 pos = ImVec2(viewportSize.x - 270, viewportSize.y - 300);
    ImVec2 size = ImVec2(250, 280);
    
    ViceTheme::Colors& colors = ViceTheme::Instance().GetColors();
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.BG_Card, 6.0f);
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.Border_Dim, 6.0f);
    
    // Title
    drawList->AddText(ImVec2(pos.x + 12, pos.y + 8), colors.Text_Main, "Keybinds");
    drawList->AddLine(ImVec2(pos.x + 8, pos.y + 30), ImVec2(pos.x + size.x - 8, pos.y + 30), colors.Border_Dim);
    
    // Keybinds list
    struct KeybindInfo { const char* name; int key; features::legit::KeybindMode mode; };
    
    KeybindInfo binds[] = {
        {"Menu", m_menuKey, features::legit::KeybindMode::Toggle},
        {"Auto Peek", 0, features::legit::KeybindMode::Hold},
        {"Thirdperson", 0, features::legit::KeybindMode::Toggle},
        {"Edge Jump", 0, features::legit::KeybindMode::Hold},
        {"Slow Walk", 0, features::legit::KeybindMode::Hold},
    };
    
    float y = pos.y + 40;
    for (auto& bind : binds) {
        std::string keyName;
        switch (bind.key) {
            case 0: keyName = "None"; break;
            case VK_INSERT: keyName = "INSERT"; break;
            default: keyName = "Key"; break;
        }
        
        std::string modeStr;
        switch (bind.mode) {
            case features::legit::KeybindMode::Toggle: modeStr = "[T]"; break;
            case features::legit::KeybindMode::Hold: modeStr = "[H]"; break;
            case features::legit::KeybindMode::Always: modeStr = "[A]"; break;
            case features::legit::KeybindMode::DoubleTap: modeStr = "[2x]"; break;
        }
        
        drawList->AddText(ImVec2(pos.x + 12, y), colors.Text_Main, bind.name);
        drawList->AddText(ImVec2(pos.x + size.x - 80, y), colors.Text_Muted, (keyName + " " + modeStr).c_str());
        y += 24;
    }
}

void MenuManager::RenderPerformanceOverlay() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    
    ImVec2 pos = ImVec2(20, 60);
    ImVec2 size = ImVec2(200, 100);
    
    ViceTheme::Colors& colors = ViceTheme::Instance().GetColors();
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.BG_Card, 6.0f);
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), colors.Border_Dim, 6.0f);
    
    // Stats
    drawList->AddText(ImVec2(pos.x + 10, pos.y + 10), colors.Text_Main, 
                     fmt::format("FPS: {:.0f}", ImGui::GetIO().Framerate).c_str());
    drawList->AddText(ImVec2(pos.x + 10, pos.y + 30), colors.Text_Muted, 
                     fmt::format("Frame: {:.2f} ms", 1000.0f / ImGui::GetIO().Framerate).c_str());
    drawList->AddText(ImVec2(pos.x + 10, pos.y + 50), colors.Text_Muted, 
                     fmt::format("Draw calls: {}", ImGui::GetIO().MetricsRenderVertices).c_str());
    drawList->AddText(ImVec2(pos.x + 10, pos.y + 70), colors.Text_Muted, 
                     fmt::format("Vertices: {}", ImGui::GetIO().MetricsRenderVertices).c_str());
}

} // namespace gui