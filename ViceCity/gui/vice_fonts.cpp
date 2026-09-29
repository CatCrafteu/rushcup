#include "core/pch.hpp"
#include "gui/vice_fonts.hpp"
#include "core/config/config_manager.hpp"

namespace gui {

void ViceFonts::Initialize() {
    LoadDefaultFonts();
    LOG_INFO(GUI, "ViceFonts initialized");
}

void ViceFonts::Shutdown() {
    LOG_INFO(GUI, "ViceFonts shutdown");
}

void ViceFonts::RebuildFontAtlas() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Build();
}

ImFont* ViceFonts::GetFont(FontType type) const {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < static_cast<int>(FontType::COUNT)) {
        return m_fonts[idx];
    }
    return nullptr;
}

ImFont* ViceFonts::GetFont(const std::string& name) const {
    auto it = m_customFonts.find(name);
    if (it != m_customFonts.end()) return it->second;
    return nullptr;
}

ImFont* ViceFonts::GetUIFont(float size) const {
    // Find closest font size
    float bestDiff = FLT_MAX;
    ImFont* bestFont = m_fonts[static_cast<int>(FontType::UI_Normal)];
    
    for (int i = static_cast<int>(FontType::UI_Tiny); i <= static_cast<int>(FontType::UI_Title); ++i) {
        ImFont* font = m_fonts[i];
        if (font) {
            float diff = std::abs(font->FontSize - size);
            if (diff < bestDiff) {
                bestDiff = diff;
                bestFont = font;
            }
        }
    }
    
    return bestFont;
}

void ViceFonts::PushFont(FontType type) {
    ImFont* font = GetFont(type);
    if (font) ImGui::PushFont(font);
}

void ViceFonts::PopFont() {
    ImGui::PopFont();
}

void ViceFonts::PushUIFont(float size) {
    ImFont* font = GetUIFont(size);
    if (font) ImGui::PushFont(font);
}

void ViceFonts::PopUIFont() {
    ImGui::PopFont();
}

void ViceFonts::SetGlobalScale(float scale) {
    m_globalScale = std::clamp(scale, 0.5f, 3.0f);
    ImGui::GetIO().FontGlobalScale = m_globalScale * m_dpiScale;
}

void ViceFonts::UpdateDPIScale(float dpiScale) {
    m_dpiScale = std::clamp(dpiScale, 0.5f, 3.0f);
    ImGui::GetIO().FontGlobalScale = m_globalScale * m_dpiScale;
}

bool ViceFonts::LoadFont(const std::string& name, const std::string& path, float size, const ImFontConfig& config, const ImWchar* ranges) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Check if file exists
    if (!std::filesystem::exists(path)) {
        LOG_WARN(GUI, "Font file not found: {}", path);
        return false;
    }
    
    ImFontConfig fontConfig = config;
    fontConfig.SizePixels = size;
    fontConfig.RasterizerFlags = ImGuiFreeType_RasterizerFlags::ImGuiFreeType_RasterizerFlags_ForceAutoHint;
    
    ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, &fontConfig, ranges);
    
    if (font) {
        m_customFonts[name] = font;
        LOG_INFO(GUI, "Loaded custom font: {} ({})", name, path);
        return true;
    }
    
    LOG_ERROR(GUI, "Failed to load font: {}", path);
    return false;
}

ImVec2 ViceFonts::CalcTextSize(FontType type, const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width) const {
    ImFont* font = GetFont(type);
    if (!font) font = ImGui::GetFont();
    return font->CalcTextSizeA(font->FontSize, FLT_MAX, wrap_width, text, text_end, nullptr);
}

ImVec2 ViceFonts::CalcTextSize(const ImFont* font, const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width) const {
    if (!font) font = ImGui::GetFont();
    return font->CalcTextSizeA(font->FontSize, FLT_MAX, wrap_width, text, text_end, nullptr);
}

bool ViceFonts::MergeIconFont(FontType baseFont, FontType iconFont, const ImWchar* iconRanges) {
    ImFont* base = GetFont(baseFont);
    ImFont* icon = GetFont(iconFont);
    
    if (!base || !icon) return false;
    
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryTTF(
        const_cast<void*>(static_cast<const void*>(icon->GetFontData())),
        icon->GetFontDataSize(),
        base->FontSize,
        &config,
        iconRanges ? iconRanges : ICON_RANGES
    );
    
    return true;
}

void ViceFonts::LoadDefaultFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    ImFontConfig config;
    config.SizePixels = 14.0f;
    config.RasterizerFlags = ImGuiFreeType_RasterizerFlags::ImGuiFreeType_RasterizerFlags_ForceAutoHint;
    config.OversampleH = 2;
    config.OversampleV = 1;
    config.PixelSnapH = true;
    
    // Try to load Inter Variable font
    bool hasInter = false;
    if (std::filesystem::exists(INTER_VARIABLE_PATH)) {
        ImFontConfig varConfig = config;
        varConfig.SizePixels = 14.0f;
        m_fonts[static_cast<int>(FontType::Inter_Variable)] = io.Fonts->AddFontFromFileTTF(
            INTER_VARIABLE_PATH, 14.0f, &varConfig, io.Fonts->GetGlyphRangesDefault());
        hasInter = true;
    }
    
    // Try to load JetBrains Mono
    bool hasJetBrains = false;
    if (std::filesystem::exists(JETBRAINS_MONO_PATH)) {
        ImFontConfig monoConfig = config;
        monoConfig.SizePixels = 13.0f;
        m_fonts[static_cast<int>(FontType::JetBrainsMono_Regular)] = io.Fonts->AddFontFromFileTTF(
            JETBRAINS_MONO_PATH, 13.0f, &monoConfig, io.Fonts->GetGlyphRangesDefault());
        hasJetBrains = true;
    }
    
    // Fallback to default font
    if (!hasInter) {
        m_fonts[static_cast<int>(FontType::Inter_Regular)] = io.Fonts->AddFontDefault(&config);
    }
    
    if (!hasJetBrains) {
        m_fonts[static_cast<int>(FontType::JetBrainsMono_Regular)] = io.Fonts->AddFontDefault(&config);
    }
    
    // Create UI size variants from base fonts
    ImFont* baseFont = m_fonts[static_cast<int>(FontType::Inter_Variable)] ? 
                       m_fonts[static_cast<int>(FontType::Inter_Variable)] : 
                       m_fonts[static_cast<int>(FontType::Inter_Regular)];
    
    ImFont* monoFont = m_fonts[static_cast<int>(FontType::JetBrainsMono_Regular)];
    
    // UI sizes
    std::array<float, 8> uiSizes = {10, 12, 14, 16, 18, 20, 24, 32};
    std::array<FontType, 8> uiTypes = {
        FontType::UI_Tiny, FontType::UI_Small, FontType::UI_Normal,
        FontType::UI_Medium, FontType::UI_Large, FontType::UI_XLarge,
        FontType::UI_Header, FontType::UI_Title
    };
    
    for (int i = 0; i < 8; ++i) {
        ImFontConfig sizeConfig = config;
        sizeConfig.SizePixels = uiSizes[i];
        m_fonts[static_cast<int>(uiTypes[i])] = io.Fonts->AddFontFromMemoryTTF(
            const_cast<void*>(baseFont->GetFontData()),
            baseFont->GetFontDataSize(),
            uiSizes[i],
            &sizeConfig,
            io.Fonts->GetGlyphRangesDefault()
        );
    }
    
    // Load icon fonts
    LoadIconFonts();
    
    // Build atlas
    io.Fonts->Build();
    
    // Set default font
    io.FontDefault = m_fonts[static_cast<int>(FontType::UI_Normal)];
    
    LOG_INFO(GUI, "Default fonts loaded, atlas size: {}x{}", io.Fonts->TexWidth, io.Fonts->TexHeight);
}

void ViceFonts::LoadInterVariable() {
    // Already handled in LoadDefaultFonts
}

void ViceFonts::LoadJetBrainsMono() {
    // Already handled in LoadDefaultFonts
}

void ViceFonts::LoadIconFonts() {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.PixelSnapH = true;
    iconConfig.SizePixels = 16.0f;
    
    // Material Icons
    if (std::filesystem::exists(MATERIAL_ICONS_PATH)) {
        m_fonts[static_cast<int>(FontType::MaterialIcons)] = io.Fonts->AddFontFromFileTTF(
            MATERIAL_ICONS_PATH, 16.0f, &iconConfig, ICON_RANGES);
    } else {
        // Fallback: use default font with icon ranges
        m_fonts[static_cast<int>(FontType::MaterialIcons)] = io.Fonts->AddFontDefault(&iconConfig, ICON_RANGES);
    }
    
    // Font Awesome
    if (std::filesystem::exists(FONT_AWESOME_PATH)) {
        ImFontConfig faConfig = iconConfig;
        faConfig.SizePixels = 14.0f;
        m_fonts[static_cast<int>(FontType::FontAwesome)] = io.Fonts->AddFontFromFileTTF(
            FONT_AWESOME_PATH, 14.0f, &faConfig, ICON_RANGES);
    } else {
        m_fonts[static_cast<int>(FontType::FontAwesome)] = m_fonts[static_cast<int>(FontType::MaterialIcons)];
    }
    
    // Custom icons (empty for now)
    m_fonts[static_cast<int>(FontType::CustomIcons)] = m_fonts[static_cast<int>(FontType::MaterialIcons)];
    
    // Merge icon fonts into UI fonts
    for (int i = static_cast<int>(FontType::UI_Tiny); i <= static_cast<int>(FontType::UI_Title); ++i) {
        if (m_fonts[i]) {
            ImFontConfig mergeConfig;
            mergeConfig.MergeMode = true;
            mergeConfig.PixelSnapH = true;
            mergeConfig.SizePixels = m_fonts[i]->FontSize;
            
            io.Fonts->AddFontFromMemoryTTF(
                const_cast<void*>(static_cast<const void*>(m_fonts[static_cast<int>(FontType::MaterialIcons)]->GetFontData())),
                m_fonts[static_cast<int>(FontType::MaterialIcons)]->GetFontDataSize(),
                m_fonts[i]->FontSize,
                &mergeConfig,
                ICON_RANGES
            );
        }
    }
}

void ViceFonts::CreateFontConfigs() {
    // Font configurations are created on-demand
}

} // namespace gui