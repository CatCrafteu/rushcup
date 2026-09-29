#pragma once

#include <imgui.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include "vice_theme.hpp"

namespace gui {

// Font Manager (Inter Variable, JetBrains Mono, Icons)
class ViceFonts {
public:
    enum class FontType {
        // Main fonts
        Inter_Regular,
        Inter_Medium,
        Inter_SemiBold,
        Inter_Bold,
        Inter_Variable,
        
        // Monospace
        JetBrainsMono_Regular,
        JetBrainsMono_Medium,
        JetBrainsMono_SemiBold,
        JetBrainsMono_Bold,
        
        // UI sizes
        UI_Tiny,      // 10px
        UI_Small,     // 12px
        UI_Normal,    // 14px
        UI_Medium,    // 16px
        UI_Large,     // 18px
        UI_XLarge,    // 20px
        UI_Header,    // 24px
        UI_Title,     // 32px
        
        // Icon fonts
        MaterialIcons,
        FontAwesome,
        CustomIcons,
        
        COUNT
    };
    
    struct FontInfo {
        std::string name;
        std::string path;
        float size = 14.0f;
        bool isVariable = false;
        float weight = 400.0f; // For variable fonts
        ImFontConfig config;
        const ImWchar* glyphRanges = nullptr;
    };
    
    static ViceFonts& Instance() {
        static ViceFonts instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void RebuildFontAtlas();
    
    // Get font by type
    ImFont* GetFont(FontType type) const;
    ImFont* GetFont(const std::string& name) const;
    
    // Get font for UI size
    ImFont* GetUIFont(float size) const;
    
    // Push/Pop font helpers
    void PushFont(FontType type);
    void PopFont();
    void PushUIFont(float size);
    void PopUIFont();
    
    // Font scaling
    void SetGlobalScale(float scale);
    float GetGlobalScale() const { return m_globalScale; }
    
    // DPI awareness
    void UpdateDPIScale(float dpiScale);
    float GetDPIScale() const { return m_dpiScale; }
    
    // Load custom font
    bool LoadFont(const std::string& name, const std::string& path, float size, const ImFontConfig& config = ImFontConfig(), const ImWchar* ranges = nullptr);
    
    // Icon font helpers
    ImFont* GetIconFont() const { return m_fonts[static_cast<int>(FontType::MaterialIcons)]; }
    ImFont* GetFontAwesome() const { return m_fonts[static_cast<int>(FontType::FontAwesome)]; }
    
    // Text measurement with specific font
    ImVec2 CalcTextSize(FontType type, const char* text, const char* text_end = nullptr, bool hide_text_after_double_hash = false, float wrap_width = -1.0f) const;
    ImVec2 CalcTextSize(const ImFont* font, const char* text, const char* text_end = nullptr, bool hide_text_after_double_hash = false, float wrap_width = -1.0f) const;
    
    // Font merging (for icons + text)
    bool MergeIconFont(FontType baseFont, FontType iconFont, const ImWchar* iconRanges = nullptr);

private:
    ViceFonts() = default;
    ~ViceFonts() = default;
    
    std::array<ImFont*, static_cast<int>(FontType::COUNT)> m_fonts = {nullptr};
    std::unordered_map<std::string, ImFont*> m_customFonts;
    float m_globalScale = 1.0f;
    float m_dpiScale = 1.0f;
    std::mutex m_mutex;
    
    void LoadDefaultFonts();
    void LoadInterVariable();
    void LoadJetBrainsMono();
    void LoadIconFonts();
    void CreateFontConfigs();
    
    // Default font paths (relative to executable or embedded)
    static constexpr const char* INTER_VARIABLE_PATH = "fonts/Inter-VariableFont_wght.ttf";
    static constexpr const char* JETBRAINS_MONO_PATH = "fonts/JetBrainsMono-VariableFont_wght.ttf";
    static constexpr const char* MATERIAL_ICONS_PATH = "fonts/MaterialIcons-Regular.ttf";
    static constexpr const char* FONT_AWESOME_PATH = "fonts/FontAwesome6-Regular.otf";
    
    // Glyph ranges
    static constexpr ImWchar ICON_RANGES[] = {
        0xE000, 0xF8FF, // Private Use Area
        0x2000, 0x206F, // General Punctuation
        0x2190, 0x21FF, // Arrows
        0x25A0, 0x25FF, // Geometric Shapes
        0x2600, 0x26FF, // Misc Symbols
        0x2700, 0x27BF, // Dingbats
        0xE000, 0xF8FF, // Private Use (Material Icons)
        0xF000, 0xF2FF, // Font Awesome
        0
    };
    
    static constexpr ImWchar CYRILLIC_RANGES[] = {
        0x0400, 0x04FF, // Cyrillic
        0x0500, 0x052F, // Cyrillic Supplement
        0
    };
    
    static constexpr ImWchar EXTENDED_LATIN_RANGES[] = {
        0x0100, 0x017F, // Latin Extended-A
        0x0180, 0x024F, // Latin Extended-B
        0x1E00, 0x1EFF, // Latin Extended Additional
        0
    };
};

} // namespace gui