#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Rarity Glow (memesense parity - border glow per rarity tier)
class RarityGlow {
public:
    struct RarityColor {
        uint32_t borderColor = 0xFFFFFFFF;
        uint32_t glowColor = 0xFFFFFFFF;
        uint32_t nameColor = 0xFFFFFFFF;
        const char* name = "";
    };
    
    // Rarity tiers: 0=Consumer(Gray), 1=Industrial(Light Blue), 2=Mil-Spec(Blue), 3=Restricted(Purple)
    // 4=Classified(Pink), 5=Covert(Red), 6=Contraband(Gold), 7=Extra
    static constexpr RarityColor RARITY_COLORS[8] = {
        {0xB0B3B8FF, 0xB0B3B880, 0xB0B3B8FF, "Consumer Grade"},      // Gray
        {0x5E98D9FF, 0x5E98D980, 0x5E98D9FF, "Industrial Grade"},     // Light Blue
        {0x4B69FFFF, 0x4B69FF80, 0x4B69FFFF, "Mil-Spec Grade"},       // Blue
        {0x8847FFFF, 0x8847FF80, 0x8847FFFF, "Restricted"},           // Purple
        {0xD32CE6FF, 0xD32CE680, 0xD32CE6FF, "Classified"},           // Pink
        {0xEB4B4BFF, 0xEB4B4B80, 0xEB4B4BFF, "Covert"},               // Red
        {0xE4AE39FF, 0xE4AE3980, 0xE4AE39FF, "Contraband"},           // Gold
        {0xFFFFFFFF, 0xFFFFFF80, 0xFFFFFFFF, "Extraordinary"}         // White
    };
    
    static RarityGlow& Instance() {
        static RarityGlow instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
    // Get colors for rarity
    const RarityColor& GetRarityColor(int rarity) const {
        if (rarity >= 0 && rarity < 8) return RARITY_COLORS[rarity];
        return RARITY_COLORS[0];
    }
    
    // Apply glow to item in UI
    void DrawRarityBorder(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size, int rarity, float thickness = 2.0f, float rounding = 3.0f);
    void DrawRarityGlow(ImDrawList* drawList, const ImVec2& center, float radius, int rarity, float intensity = 1.0f);
    void DrawRarityName(ImDrawList* drawList, const ImVec2& pos, int rarity, const char* text = nullptr, float fontSize = 12.0f);
    
    // Animated glow
    void DrawAnimatedRarityGlow(ImDrawList* drawList, const ImVec2& center, float radius, int rarity, float time, float speed = 2.0f);
    
    // Custom colors (configurable)
    void SetCustomRarityColor(int rarity, uint32_t border, uint32_t glow, uint32_t name);
    void ResetRarityColor(int rarity);
    void ResetAllRarityColors();
    
    // Config
    struct Config {
        bool enabled = true;
        float borderThickness = 2.0f;
        float glowIntensity = 1.0f;
        bool animateGlow = true;
        float animationSpeed = 2.0f;
        bool showRarityName = true;
        float nameFontSize = 12.0f;
    };
    
    Config& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    RarityGlow() = default;
    Config m_config;
    RarityColor m_customColors[8];
    bool m_hasCustomColors[8] = {false};
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "skinchanger_rarity_glow";
};

} // namespace skinchanger