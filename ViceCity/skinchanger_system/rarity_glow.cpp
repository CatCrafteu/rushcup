#include "core/pch.hpp"
#include "skinchanger_system/rarity_glow.hpp"
#include "gui/vice_theme.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void RarityGlow::Initialize() {
    LoadConfig();
    LOG_INFO(Skinchanger, "RarityGlow initialized");
}

void RarityGlow::Shutdown() {
    SaveConfig();
    LOG_INFO(Skinchanger, "RarityGlow shutdown");
}

void RarityGlow::Render() {
    if (!m_config.enabled) return;
    
    // This is called from UI rendering code
    // The actual drawing happens in DrawRarityBorder/Glow/Name
}

void RarityGlow::DrawRarityBorder(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size, int rarity, float thickness, float rounding) {
    if (!m_config.enabled) return;
    if (rarity < 0 || rarity >= 8) return;
    
    const auto& color = RARITY_COLORS[rarity];
    ImU32 borderColor = color.borderColor;
    
    // Use custom color if set
    if (m_hasCustomColors[rarity]) {
        borderColor = m_customColors[rarity].borderColor;
    }
    
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, rounding, 0, thickness);
}

void RarityGlow::DrawRarityGlow(ImDrawList* drawList, const ImVec2& center, float radius, int rarity, float intensity) {
    if (!m_config.enabled) return;
    if (rarity < 0 || rarity >= 8) return;
    
    const auto& color = RARITY_COLORS[rarity];
    ImU32 glowColor = color.glowColor;
    
    if (m_hasCustomColors[rarity]) {
        glowColor = m_customColors[rarity].glowColor;
    }
    
    uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(glowColor) * intensity * m_config.glowIntensity);
    glowColor = (glowColor & 0x00FFFFFF) | (alpha << 24);
    
    // Draw multiple circles for glow effect
    for (int i = 0; i < 4; ++i) {
        float t = static_cast<float>(i) / 4.0f;
        float r = radius + m_config.borderThickness * (1.0f + t * 2.0f);
        uint8_t a = static_cast<uint8_t>(alpha * (1.0f - t * 0.5f));
        ImU32 c = (glowColor & 0x00FFFFFF) | (a << 24);
        
        drawList->AddCircle(center, r, c, 32, m_config.borderThickness);
    }
}

void RarityGlow::DrawRarityName(ImDrawList* drawList, const ImVec2& pos, int rarity, const char* text, float fontSize) {
    if (!m_config.enabled) return;
    if (rarity < 0 || rarity >= 8) return;
    if (!m_config.showRarityName && !text) return;
    
    const auto& color = RARITY_COLORS[rarity];
    ImU32 nameColor = color.nameColor;
    
    if (m_hasCustomColors[rarity]) {
        nameColor = m_customColors[rarity].nameColor;
    }
    
    const char* name = text ? text : RARITY_COLORS[rarity].name;
    ImFont* font = gui::ViceFonts::Instance().GetFont(gui::FontType::UI_Normal);
    
    drawList->AddText(font, fontSize, pos, nameColor, name);
}

void RarityGlow::DrawAnimatedRarityGlow(ImDrawList* drawList, const ImVec2& center, float radius, int rarity, float time, float speed) {
    if (!m_config.enabled || !m_config.animateGlow) return;
    if (rarity < 0 || rarity >= 8) return;
    
    const auto& color = RARITY_COLORS[rarity];
    ImU32 glowColor = color.glowColor;
    
    if (m_hasCustomColors[rarity]) {
        glowColor = m_customColors[rarity].glowColor;
    }
    
    // Pulsing animation
    float pulse = (sinf(time * speed * m_config.animationSpeed) + 1.0f) * 0.5f;
    float intensity = 0.5f + pulse * 0.5f;
    intensity *= m_config.glowIntensity;
    
    uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(glowColor) * intensity);
    glowColor = (glowColor & 0x00FFFFFF) | (alpha << 24);
    
    // Draw animated glow rings
    for (int i = 0; i < 3; ++i) {
        float t = static_cast<float>(i) / 3.0f;
        float r = radius + m_config.borderThickness * (1.0f + t * 3.0f + pulse * 10.0f);
        uint8_t a = static_cast<uint8_t>(alpha * (1.0f - t * 0.7f));
        ImU32 c = (glowColor & 0x00FFFFFF) | (a << 24);
        
        drawList->AddCircle(center, r, c, 32, m_config.borderThickness);
    }
}

void RarityGlow::SetCustomRarityColor(int rarity, uint32_t border, uint32_t glow, uint32_t name) {
    if (rarity < 0 || rarity >= 8) return;
    
    m_customColors[rarity].borderColor = border;
    m_customColors[rarity].glowColor = glow;
    m_customColors[rarity].nameColor = name;
    m_hasCustomColors[rarity] = true;
}

void RarityGlow::ResetRarityColor(int rarity) {
    if (rarity < 0 || rarity >= 8) return;
    m_hasCustomColors[rarity] = false;
}

void RarityGlow::ResetAllRarityColors() {
    for (int i = 0; i < 8; ++i) {
        m_hasCustomColors[i] = false;
    }
}

RarityGlow::Config& RarityGlow::GetConfig() {
    return m_config;
}

void RarityGlow::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto opt = config.Get<json>("gui", "rarity_glow");
    if (opt) {
        try {
            m_config.enabled = opt->value("enabled", true);
            m_config.borderThickness = opt->value("borderThickness", 2.0f);
            m_config.glowIntensity = opt->value("glowIntensity", 1.0f);
            m_config.animateGlow = opt->value("animateGlow", true);
            m_config.animationSpeed = opt->value("animationSpeed", 2.0f);
            m_config.showRarityName = opt->value("showRarityName", true);
            m_config.nameFontSize = opt->value("nameFontSize", 12.0f);
            
            // Load custom colors
            for (int i = 0; i < 8; ++i) {
                std::string key = "customRarity" + std::to_string(i);
                if (opt->contains(key)) {
                    auto c = (*opt)[key];
                    m_customColors[i].borderColor = IM_COL32(
                        c.value("borderR", 255), c.value("borderG", 255),
                        c.value("borderB", 255), c.value("borderA", 255)
                    );
                    m_customColors[i].glowColor = IM_COL32(
                        c.value("glowR", 255), c.value("glowG", 255),
                        c.value("glowB", 255), c.value("glowA", 255)
                    );
                    m_customColors[i].nameColor = IM_COL32(
                        c.value("nameR", 255), c.value("nameG", 255),
                        c.value("nameB", 255), c.value("nameA", 255)
                    );
                    m_hasCustomColors[i] = true;
                }
            }
        } catch (...) {}
    }
}

void RarityGlow::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["enabled"] = m_config.enabled;
    j["borderThickness"] = m_config.borderThickness;
    j["glowIntensity"] = m_config.glowIntensity;
    j["animateGlow"] = m_config.animateGlow;
    j["animationSpeed"] = m_config.animationSpeed;
    j["showRarityName"] = m_config.showRarityName;
    j["nameFontSize"] = m_config.nameFontSize;
    
    for (int i = 0; i < 8; ++i) {
        if (m_hasCustomColors[i]) {
            std::string key = "customRarity" + std::to_string(i);
            j[key] = {
                {"borderR", IM_COL32_R(m_customColors[i].borderColor)},
                {"borderG", IM_COL32_G(m_customColors[i].borderColor)},
                {"borderB", IM_COL32_B(m_customColors[i].borderColor)},
                {"borderA", IM_COL32_A(m_customColors[i].borderColor)},
                {"glowR", IM_COL32_R(m_customColors[i].glowColor)},
                {"glowG", IM_COL32_G(m_customColors[i].glowColor)},
                {"glowB", IM_COL32_B(m_customColors[i].glowColor)},
                {"glowA", IM_COL32_A(m_customColors[i].glowColor)},
                {"nameR", IM_COL32_R(m_customColors[i].nameColor)},
                {"nameG", IM_COL32_G(m_customColors[i].nameColor)},
                {"nameB", IM_COL32_B(m_customColors[i].nameColor)},
                {"nameA", IM_COL32_A(m_customColors[i].nameColor)}
            };
        }
    }
    
    config.Set("gui", "rarity_glow", j);
}

} // namespace skinchanger