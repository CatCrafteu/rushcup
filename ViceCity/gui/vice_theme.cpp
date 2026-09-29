#include "core/pch.hpp"
#include "gui/vice_theme.hpp"
#include "core/config/config_manager.hpp"

namespace gui {

void ViceTheme::Initialize() {
    ApplyTheme();
    LoadConfig();
    LOG_INFO(GUI, "ViceTheme initialized");
}

void ViceTheme::Shutdown() {
    SaveConfig();
    LOG_INFO(GUI, "ViceTheme shutdown");
}

void ViceTheme::ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ApplyStyleColors(style);
    ApplyStyleVars(style);
}

void ViceTheme::ApplyStyleColors(ImGuiStyle& style) {
    ImVec4* colors = style.Colors;
    
    // Main colors
    colors[ImGuiCol_Text] = ImColor(m_colors.Text_Main);
    colors[ImGuiCol_TextDisabled] = ImColor(m_colors.Text_Disabled);
    colors[ImGuiCol_WindowBg] = ImColor(m_colors.BG_Primary);
    colors[ImGuiCol_ChildBg] = ImColor(m_colors.BG_Card);
    colors[ImGuiCol_PopupBg] = ImColor(m_colors.Popup_Bg);
    
    // Borders
    colors[ImGuiCol_Border] = ImColor(m_colors.Border_Dim);
    colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
    
    // Frame
    colors[ImGuiCol_FrameBg] = ImColor(m_colors.BG_Card);
    colors[ImGuiCol_FrameBgHovered] = ImColor(m_colors.BG_Hover);
    colors[ImGuiCol_FrameBgActive] = ImColor(m_colors.BG_Active);
    
    // Title
    colors[ImGuiCol_TitleBg] = ImColor(m_colors.BG_Card);
    colors[ImGuiCol_TitleBgActive] = ImColor(m_colors.BG_Active);
    colors[ImGuiCol_TitleBgCollapsed] = ImColor(m_colors.BG_Primary);
    
    // Menu
    colors[ImGuiCol_MenuBarBg] = ImColor(m_colors.BG_Card);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImColor(m_colors.Scrollbar_Bg);
    colors[ImGuiCol_ScrollbarGrab] = ImColor(m_colors.Scrollbar_Grab);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(m_colors.Scrollbar_Grab_Hover);
    colors[ImGuiCol_ScrollbarGrabActive] = ImColor(m_colors.Scrollbar_Grab_Active);
    
    // Checkbox
    colors[ImGuiCol_CheckMark] = ImColor(m_colors.PrimaryPink);
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImColor(m_colors.PrimaryPink);
    colors[ImGuiCol_SliderGrabActive] = ImColor(m_colors.PinkSoft);
    
    // Button
    colors[ImGuiCol_Button] = ImColor(m_colors.Button_Normal);
    colors[ImGuiCol_ButtonHovered] = ImColor(m_colors.Button_Hover);
    colors[ImGuiCol_ButtonActive] = ImColor(m_colors.Button_Active);
    
    // Header
    colors[ImGuiCol_Header] = ImColor(m_colors.Header_Bg);
    colors[ImGuiCol_HeaderHovered] = ImColor(m_colors.Header_Hover);
    colors[ImGuiCol_HeaderActive] = ImColor(m_colors.Header_Active);
    
    // Separator
    colors[ImGuiCol_Separator] = ImColor(m_colors.Separator);
    colors[ImGuiCol_SeparatorHovered] = ImColor(m_colors.Border_Pink);
    colors[ImGuiCol_SeparatorActive] = ImColor(m_colors.PrimaryPink);
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip] = ImColor(m_colors.Border_Dim);
    colors[ImGuiCol_ResizeGripHovered] = ImColor(m_colors.Border_Pink);
    colors[ImGuiCol_ResizeGripActive] = ImColor(m_colors.PrimaryPink);
    
    // Tab
    colors[ImGuiCol_Tab] = ImColor(m_colors.Tab_Normal);
    colors[ImGuiCol_TabHovered] = ImColor(m_colors.Tab_Hover);
    colors[ImGuiCol_TabActive] = ImColor(m_colors.Tab_Active);
    colors[ImGuiCol_TabUnfocused] = ImColor(m_colors.Tab_Normal);
    colors[ImGuiCol_TabUnfocusedActive] = ImColor(m_colors.Tab_Hover);
    
    // Docking
    colors[ImGuiCol_DockingPreview] = ImColor(m_colors.PrimaryPink);
    colors[ImGuiCol_DockingEmptyBg] = ImColor(m_colors.BG_Primary);
    
    // Plot
    colors[ImGuiCol_PlotLines] = ImColor(m_colors.Plot_Lines);
    colors[ImGuiCol_PlotLinesHovered] = ImColor(m_colors.Plot_Lines_Hover);
    colors[ImGuiCol_PlotHistogram] = ImColor(m_colors.Plot_Histogram);
    colors[ImGuiCol_PlotHistogramHovered] = ImColor(m_colors.Plot_Histogram_Hover);
    
    // Table
    colors[ImGuiCol_TableHeaderBg] = ImColor(m_colors.Table_Header);
    colors[ImGuiCol_TableBorderStrong] = ImColor(m_colors.Table_Border);
    colors[ImGuiCol_TableBorderLight] = ImColor(m_colors.Border_Dim);
    colors[ImGuiCol_TableRowBg] = ImColor(m_colors.Table_Row);
    colors[ImGuiCol_TableRowBgAlt] = ImColor(m_colors.Table_Row_Alt);
    
    // Text link
    colors[ImGuiCol_TextSelectedBg] = ImColor(m_colors.Glow_Pink);
    
    // Drag drop
    colors[ImGuiCol_DragDropTarget] = ImColor(m_colors.PrimaryYellow);
    
    // Nav
    colors[ImGuiCol_NavHighlight] = ImColor(m_colors.PrimaryPink);
    colors[ImGuiCol_NavWindowingHighlight] = ImColor(m_colors.PrimaryYellow);
    colors[ImGuiCol_NavWindowingDimBg] = ImColor(m_colors.Modal_Dim);
    
    // Modal
    colors[ImGuiCol_ModalWindowDimBg] = ImColor(m_colors.Modal_Dim);
}

void ViceTheme::ApplyStyleVars(ImGuiStyle& style) {
    style.WindowRounding = m_style.WindowRounding;
    style.ChildRounding = m_style.ChildRounding;
    style.FrameRounding = m_style.FrameRounding;
    style.PopupRounding = m_style.PopupRounding;
    style.ScrollbarRounding = m_style.ScrollbarRounding;
    style.GrabRounding = m_style.GrabRounding;
    style.TabRounding = m_style.TabRounding;
    
    style.WindowBorderSize = m_style.WindowBorderSize;
    style.ChildBorderSize = m_style.ChildBorderSize;
    style.PopupBorderSize = m_style.PopupBorderSize;
    style.TabBorderSize = m_style.TabBorderSize;
    
    style.WindowPadding = m_style.WindowPadding;
    style.FramePadding = m_style.FramePadding;
    style.ItemSpacing = m_style.ItemSpacing;
    style.ItemInnerSpacing = m_style.ItemInnerSpacing;
    style.CellPadding = m_style.CellPadding;
    style.TouchExtraPadding = m_style.TouchExtraPadding;
    
    style.IndentSpacing = m_style.IndentSpacing;
    style.ScrollbarSize = m_style.ScrollbarSize;
    style.GrabMinSize = m_style.GrabMinSize;
    
    style.WindowTitleAlign = m_style.WindowTitleAlign;
    style.ButtonTextAlign = m_style.ButtonTextAlign;
    style.SelectableTextAlign = m_style.SelectableTextAlign;
    
    style.AntiAliasFringe = m_style.AntiAliasFringe;
    style.CurveTessellationTol = m_style.CurveTessellationTol;
    style.CircleTessellationMaxError = m_style.CircleTessellationMaxError;
}

void ViceTheme::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    // Load colors
    auto loadColor = [&](const char* key, ImU32 defaultColor) -> ImU32 {
        auto opt = config.Get("gui", key);
        if (opt) {
            auto arr = opt->get<std::array<int, 4>>();
            return IM_COL32(arr[0], arr[1], arr[2], arr[3]);
        }
        return defaultColor;
    };
    
    m_colors.PrimaryPink = loadColor("colors.primary_pink", m_colors.PrimaryPink);
    m_colors.PrimaryYellow = loadColor("colors.primary_yellow", m_colors.PrimaryYellow);
    m_colors.BG_Primary = loadColor("colors.bg_primary", m_colors.BG_Primary);
    m_colors.BG_Card = loadColor("colors.bg_card", m_colors.BG_Card);
    m_colors.Text_Main = loadColor("colors.text_main", m_colors.Text_Main);
    m_colors.Text_Muted = loadColor("colors.text_muted", m_colors.Text_Muted);
    
    ApplyTheme();
}

void ViceTheme::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto saveColor = [&](const char* key, ImU32 color) {
        std::array<int, 4> arr = {
            IM_COL32_R(color),
            IM_COL32_G(color),
            IM_COL32_B(color),
            IM_COL32_A(color)
        };
        config.Set("gui", key, arr);
    };
    
    saveColor("colors.primary_pink", m_colors.PrimaryPink);
    saveColor("colors.primary_yellow", m_colors.PrimaryYellow);
    saveColor("colors.bg_primary", m_colors.BG_Primary);
    saveColor("colors.bg_card", m_colors.BG_Card);
    saveColor("colors.text_main", m_colors.Text_Main);
    saveColor("colors.text_muted", m_colors.Text_Muted);
}

ImU32 ViceTheme::LerpColor(ImU32 a, ImU32 b, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    uint8_t r = static_cast<uint8_t>(IM_COL32_R(a) + (IM_COL32_R(b) - IM_COL32_R(a)) * t);
    uint8_t g = static_cast<uint8_t>(IM_COL32_G(a) + (IM_COL32_G(b) - IM_COL32_G(a)) * t);
    uint8_t bl = static_cast<uint8_t>(IM_COL32_B(a) + (IM_COL32_B(b) - IM_COL32_B(a)) * t);
    uint8_t al = static_cast<uint8_t>(IM_COL32_A(a) + (IM_COL32_A(b) - IM_COL32_A(a)) * t);
    return IM_COL32(r, g, bl, al);
}

void ViceTheme::DrawGradientRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, 
                                  ImU32 colLeft, ImU32 colRight, bool horizontal) {
    if (horizontal) {
        drawList->AddRectFilledMultiColor(pMin, pMax, colLeft, colRight, colRight, colLeft);
    } else {
        drawList->AddRectFilledMultiColor(pMin, pMax, colLeft, colLeft, colRight, colRight);
    }
}

void ViceTheme::DrawGradientRectV(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                                   ImU32 colTop, ImU32 colBottom) {
    drawList->AddRectFilledMultiColor(pMin, pMax, colTop, colTop, colBottom, colBottom);
}

void ViceTheme::DrawRoundedGradientRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                                         ImU32 col1, ImU32 col2, float rounding, float thickness,
                                         bool horizontal) {
    // Draw filled rounded rect with gradient using multiple thin rects
    const int steps = 16;
    float stepSize = horizontal ? (pMax.x - pMin.x) / steps : (pMax.y - pMin.y) / steps;
    
    for (int i = 0; i < steps; ++i) {
        float t1 = static_cast<float>(i) / steps;
        float t2 = static_cast<float>(i + 1) / steps;
        ImU32 c1 = LerpColor(col1, col2, t1);
        ImU32 c2 = LerpColor(col1, col2, t2);
        
        ImVec2 rMin, rMax;
        if (horizontal) {
            rMin = ImVec2(pMin.x + t1 * (pMax.x - pMin.x), pMin.y);
            rMax = ImVec2(pMin.x + t2 * (pMax.x - pMin.x), pMax.y);
        } else {
            rMin = ImVec2(pMin.x, pMin.y + t1 * (pMax.y - pMin.y));
            rMax = ImVec2(pMax.x, pMin.y + t2 * (pMax.y - pMin.y));
        }
        
        float r = rounding;
        if (i == 0) r = rounding;
        else if (i == steps - 1) r = rounding;
        else r = 0.0f;
        
        drawList->AddRectFilled(rMin, rMax, c1, r, 
            (i == 0 ? ImDrawFlags_RoundCornersLeft : 0) | 
            (i == steps - 1 ? ImDrawFlags_RoundCornersRight : 0));
    }
}

void ViceTheme::DrawGlowRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                              ImU32 glowColor, float glowSize, float rounding) {
    // Draw multiple expanding rects with decreasing alpha
    for (int i = 0; i < 8; ++i) {
        float t = static_cast<float>(i) / 8.0f;
        float size = glowSize * (1.0f - t * 0.5f);
        uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(glowColor) * (1.0f - t));
        ImU32 color = (glowColor & 0x00FFFFFF) | (alpha << 24);
        
        ImVec2 rMin = ImVec2(pMin.x - size, pMin.y - size);
        ImVec2 rMax = ImVec2(pMax.x + size, pMax.y + size);
        drawList->AddRect(rMin, rMax, color, rounding + size, ImDrawFlags_RoundCornersAll, 1.0f);
    }
}

void ViceTheme::DrawInnerGlow(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                               ImU32 glowColor, float glowSize, float rounding) {
    for (int i = 0; i < 8; ++i) {
        float t = static_cast<float>(i) / 8.0f;
        float size = glowSize * t;
        uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(glowColor) * (1.0f - t));
        ImU32 color = (glowColor & 0x00FFFFFF) | (alpha << 24);
        
        ImVec2 rMin = ImVec2(pMin.x + size, pMin.y + size);
        ImVec2 rMax = ImVec2(pMax.x - size, pMax.y - size);
        if (rMin.x >= rMax.x || rMin.y >= rMax.y) break;
        
        drawList->AddRect(rMin, rMax, color, rounding - size, ImDrawFlags_RoundCornersAll, 1.0f);
    }
}

void ViceTheme::DrawShimmer(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                             ImU32 baseColor, ImU32 shimmerColor, float time, float speed) {
    // Draw base
    drawList->AddRectFilled(pMin, pMax, baseColor);
    
    // Draw shimmer sweep
    float sweepPos = fmodf(time * speed, 2.0f) - 1.0f; // -1 to 1
    float width = pMax.x - pMin.x;
    float sweepX = pMin.x + (sweepPos + 1.0f) * 0.5f * width;
    
    ImVec2 shimmerMin = ImVec2(sweepX - width * 0.1f, pMin.y);
    ImVec2 shimmerMax = ImVec2(sweepX + width * 0.1f, pMax.y);
    
    if (shimmerMin.x < pMax.x && shimmerMax.x > pMin.x) {
        shimmerMin.x = std::max(shimmerMin.x, pMin.x);
        shimmerMax.x = std::min(shimmerMax.x, pMax.x);
        drawList->AddRectFilled(shimmerMin, shimmerMax, shimmerColor);
    }
}

ImU32 ViceTheme::ColorFromConfig(const std::string& key, ImU32 defaultColor) {
    auto& config = core::config::ConfigManager::Instance();
    auto opt = config.Get("gui", key);
    if (opt) {
        auto arr = opt->get<std::array<int, 4>>();
        return IM_COL32(arr[0], arr[1], arr[2], arr[3]);
    }
    return defaultColor;
}

void ViceTheme::SaveColorToConfig(const std::string& key, ImU32 color) {
    auto& config = core::config::ConfigManager::Instance();
    std::array<int, 4> arr = {
        IM_COL32_R(color),
        IM_COL32_G(color),
        IM_COL32_B(color),
        IM_COL32_A(color)
    };
    config.Set("gui", key, arr);
}

} // namespace gui