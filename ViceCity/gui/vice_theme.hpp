#pragma once

#include <imgui.h>
#include <vector>
#include "../../core/config/config_manager.hpp"

namespace gui {

// ViceCity Theme (Pink-Yellow Warmth)
class ViceTheme {
public:
    struct Colors {
        // Core
        ImU32 PrimaryPink = IM_COL32(255, 95, 155, 255);      // #FF5F9B
        ImU32 PrimaryYellow = IM_COL32(255, 215, 0, 255);     // #FFD700
        ImU32 PinkSoft = IM_COL32(255, 140, 180, 255);        // #FF8CB4
        ImU32 YellowSoft = IM_COL32(255, 230, 80, 255);       // #FFE650
        
        // Backgrounds
        ImU32 BG_Primary = IM_COL32(28, 24, 28, 255);         // #1C181C
        ImU32 BG_Card = IM_COL32(36, 30, 36, 255);            // #241E24
        ImU32 BG_Hover = IM_COL32(48, 40, 48, 255);           // #302830
        ImU32 BG_Active = IM_COL32(60, 50, 60, 255);          // #3C323C
        ImU32 BG_Modal = IM_COL32(20, 18, 20, 240);           // #141214 @ 94%
        
        // Borders
        ImU32 Border_Dim = IM_COL32(55, 45, 55, 255);         // #372D37
        ImU32 Border_Pink = IM_COL32(120, 70, 95, 255);       // #78465F
        ImU32 Border_Yellow = IM_COL32(120, 105, 40, 255);    // #786928
        ImU32 Border_Active = IM_COL32(255, 95, 155, 255);    // Pink
        
        // Text
        ImU32 Text_Main = IM_COL32(255, 245, 240, 255);       // #FFF5F0
        ImU32 Text_Muted = IM_COL32(180, 165, 175, 255);      // #B4A5AF
        ImU32 Text_Disabled = IM_COL32(100, 90, 95, 255);     // #645A5F
        ImU32 Text_Link = IM_COL32(255, 140, 180, 255);       // Pink soft
        ImU32 Text_Warning = IM_COL32(255, 215, 0, 255);      // Yellow
        ImU32 Text_Error = IM_COL32(255, 80, 80, 255);        // Red
        ImU32 Text_Success = IM_COL32(80, 255, 120, 255);     // Green
        
        // Glows/Shadows
        ImU32 Glow_Pink = IM_COL32(255, 95, 155, 120);
        ImU32 Glow_Yellow = IM_COL32(255, 215, 0, 100);
        ImU32 Glow_Pink_Strong = IM_COL32(255, 95, 155, 180);
        ImU32 Glow_Yellow_Strong = IM_COL32(255, 215, 0, 150);
        ImU32 Shadow = IM_COL32(0, 0, 0, 180);
        ImU32 Shadow_Strong = IM_COL32(0, 0, 0, 220);
        
        // Widget states
        ImU32 Button_Normal = IM_COL32(48, 40, 48, 255);
        ImU32 Button_Hover = IM_COL32(60, 50, 60, 255);
        ImU32 Button_Active = IM_COL32(80, 65, 80, 255);
        ImU32 Button_Text = IM_COL32(255, 245, 240, 255);
        
        ImU32 Checkbox_Bg = IM_COL32(36, 30, 36, 255);
        ImU32 Checkbox_Check = IM_COL32(255, 95, 155, 255);
        ImU32 Checkbox_Hover = IM_COL32(48, 40, 48, 255);
        
        ImU32 Slider_Bg = IM_COL32(36, 30, 36, 255);
        ImU32 Slider_Grab = IM_COL32(255, 95, 155, 255);
        ImU32 Slider_Grab_Active = IM_COL32(255, 140, 180, 255);
        ImU32 Slider_Text = IM_COL32(255, 245, 240, 255);
        
        ImU32 Tab_Normal = IM_COL32(36, 30, 36, 255);
        ImU32 Tab_Hover = IM_COL32(48, 40, 48, 255);
        ImU32 Tab_Active = IM_COL32(255, 95, 155, 255);
        ImU32 Tab_Text = IM_COL32(180, 165, 175, 255);
        ImU32 Tab_Text_Active = IM_COL32(255, 245, 240, 255);
        
        ImU32 Combo_Bg = IM_COL32(36, 30, 36, 255);
        ImU32 Combo_Hover = IM_COL32(48, 40, 48, 255);
        ImU32 Combo_Selected = IM_COL32(60, 50, 60, 255);
        ImU32 Combo_Text = IM_COL32(255, 245, 240, 255);
        
        ImU32 Input_Bg = IM_COL32(28, 24, 28, 255);
        ImU32 Input_Border = IM_COL32(55, 45, 55, 255);
        ImU32 Input_Border_Hover = IM_COL32(120, 70, 95, 255);
        ImU32 Input_Border_Active = IM_COL32(255, 95, 155, 255);
        ImU32 Input_Text = IM_COL32(255, 245, 240, 255);
        ImU32 Input_Placeholder = IM_COL32(100, 90, 95, 255);
        
        ImU32 Header_Bg = IM_COL32(36, 30, 36, 255);
        ImU32 Header_Hover = IM_COL32(48, 40, 48, 255);
        ImU32 Header_Active = IM_COL32(60, 50, 60, 255);
        
        ImU32 Separator = IM_COL32(55, 45, 55, 255);
        ImU32 Scrollbar_Bg = IM_COL32(28, 24, 28, 255);
        ImU32 Scrollbar_Grab = IM_COL32(60, 50, 60, 255);
        ImU32 Scrollbar_Grab_Hover = IM_COL32(80, 65, 80, 255);
        ImU32 Scrollbar_Grab_Active = IM_COL32(120, 70, 95, 255);
        
        ImU32 Popup_Bg = IM_COL32(28, 24, 28, 255);
        ImU32 Popup_Border = IM_COL32(120, 70, 95, 255);
        ImU32 Modal_Dim = IM_COL32(0, 0, 0, 200);
        
        ImU32 Tooltip_Bg = IM_COL32(36, 30, 36, 255);
        ImU32 Tooltip_Border = IM_COL32(120, 70, 95, 255);
        
        ImU32 Progress_Bg = IM_COL32(36, 30, 36, 255);
        ImU32 Progress_Fill = IM_COL32(255, 95, 155, 255);
        
        ImU32 Plot_Lines = IM_COL32(255, 95, 155, 255);
        ImU32 Plot_Lines_Hover = IM_COL32(255, 140, 180, 255);
        ImU32 Plot_Histogram = IM_COL32(255, 215, 0, 255);
        ImU32 Plot_Histogram_Hover = IM_COL32(255, 230, 80, 255);
        
        ImU32 Table_Header = IM_COL32(36, 30, 36, 255);
        ImU32 Table_Border = IM_COL32(55, 45, 55, 255);
        ImU32 Table_Row = IM_COL32(28, 24, 28, 255);
        ImU32 Table_Row_Alt = IM_COL32(32, 28, 32, 255);
        ImU32 Table_Row_Hover = IM_COL32(48, 40, 48, 255);
    };
    
    struct Style {
        float WindowRounding = 8.0f;
        float ChildRounding = 6.0f;
        float FrameRounding = 4.0f;
        float PopupRounding = 6.0f;
        float ScrollbarRounding = 6.0f;
        float GrabRounding = 4.0f;
        float TabRounding = 4.0f;
        
        float WindowBorderSize = 1.0f;
        float ChildBorderSize = 1.0f;
        float PopupBorderSize = 1.0f;
        float TabBorderSize = 1.0f;
        
        ImVec2 WindowPadding = {12, 12};
        ImVec2 FramePadding = {8, 6};
        ImVec2 ItemSpacing = {8, 6};
        ImVec2 ItemInnerSpacing = {6, 4};
        ImVec2 CellPadding = {6, 4};
        ImVec2 TouchExtraPadding = {0, 0};
        
        float IndentSpacing = 20.0f;
        float ScrollbarSize = 10.0f;
        float GrabMinSize = 12.0f;
        
        ImVec2 WindowTitleAlign = {0.5f, 0.5f};
        ImVec2 ButtonTextAlign = {0.5f, 0.5f};
        ImVec2 SelectableTextAlign = {0.0f, 0.5f};
        
        float AntiAliasFringe = 1.0f;
        float CurveTessellationTol = 1.25f;
        float CircleTessellationMaxError = 0.3f;
    };
    
    struct Animation {
        float HoverSpeed = 0.15f;
        float ActiveSpeed = 0.08f;
        float TabSpeed = 0.2f;
        float PopupSpeed = 0.15f;
        float TooltipSpeed = 0.1f;
        float CheckboxSpeed = 0.12f;
        float SliderSpeed = 0.1f;
        float ComboSpeed = 0.15f;
    };
    
    static ViceTheme& Instance() {
        static ViceTheme instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void ApplyTheme();
    void LoadConfig();
    void SaveConfig();
    
    Colors& GetColors() { return m_colors; }
    const Colors& GetColors() const { return m_colors; }
    Style& GetStyle() { return m_style; }
    const Style& GetStyle() const { return m_style; }
    Animation& GetAnimation() { return m_animation; }
    const Animation& GetAnimation() const { return m_animation; }
    
    // Gradient helpers
    ImU32 LerpColor(ImU32 a, ImU32 b, float t) const;
    void DrawGradientRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, 
                          ImU32 colLeft, ImU32 colRight, bool horizontal = true);
    void DrawGradientRectV(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                           ImU32 colTop, ImU32 colBottom);
    void DrawRoundedGradientRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                                  ImU32 col1, ImU32 col2, float rounding, float thickness = 1.0f,
                                  bool horizontal = true);
    
    // Glow effects
    void DrawGlowRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                      ImU32 glowColor, float glowSize = 10.0f, float rounding = 8.0f);
    void DrawInnerGlow(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                       ImU32 glowColor, float glowSize = 8.0f, float rounding = 6.0f);
    
    // Shimmer effect
    void DrawShimmer(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax,
                     ImU32 baseColor, ImU32 shimmerColor, float time, float speed = 1.0f);

private:
    ViceTheme() = default;
    Colors m_colors;
    Style m_style;
    Animation m_animation;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "gui.theme";
    
    void ApplyStyleColors(ImGuiStyle& style);
    void ApplyStyleVars(ImGuiStyle& style);
    ImU32 ColorFromConfig(const std::string& key, ImU32 defaultColor);
    void SaveColorToConfig(const std::string& key, ImU32 color);
};

} // namespace gui