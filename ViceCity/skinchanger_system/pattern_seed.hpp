#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Pattern Seed Browser (memesense parity - browse seeds 0-1000 with preview)
class PatternSeed {
public:
    struct PatternInfo {
        int paintKit = 0;
        int seed = 0;
        float wear = 0.0f;
        std::string description;
        sdk::Vector2D patternOffset = {0, 0};
        float patternScale = 1.0f;
        float patternRotation = 0.0f;
        // For preview rendering
        void* previewTexture = nullptr;
        int previewWidth = 0;
        int previewHeight = 0;
    };
    
    struct PatternSeedState {
        int currentPaintKit = 0;
        int currentSeed = 0;
        float currentWear = 0.001f;
        int itemDefIndex = 0;
        bool showPreview = true;
        bool showDetails = true;
        int previewSize = 512;
    };
    
    static PatternSeed& Instance() {
        static PatternSeed instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void Update(float dt);
    
    // Browse
    void SetPaintKit(int paintKit);
    void SetSeed(int seed);
    void SetWear(float wear);
    void SetItemDefIndex(int itemDefIndex);
    void NextSeed();
    void PreviousSeed();
    void RandomSeed();
    
    // Get pattern info
    std::optional<PatternInfo> GetPatternInfo(int paintKit, int seed, float wear);
    std::vector<PatternInfo> GetPatternRange(int paintKit, int startSeed, int endSeed, float wear);
    
    // Search
    std::vector<int> FindSeedsWithPattern(int paintKit, const std::string& patternName, float wear = 0.001f);
    std::vector<int> FindSeedsByColor(int paintKit, const sdk::Color& targetColor, float tolerance = 0.1f, float wear = 0.001f);
    
    // State
    PatternSeedState& GetState() { return m_state; }
    const PatternSeedState& GetState() const { return m_state; }
    
    // Pattern generation
    void GeneratePreviewTexture(int paintKit, int seed, float wear, int width, int height);
    void UpdatePreviewTexture();

private:
    PatternSeed() = default;
    PatternSeedState m_state;
    std::unordered_map<std::string, PatternInfo> m_cache; // "paintKit_seed_wear" -> PatternInfo
    std::mutex m_mutex;
    
    void InitializePatternData();
    PatternInfo GeneratePatternInfo(int paintKit, int seed, float wear);
    sdk::Vector2D CalculatePatternOffset(int paintKit, int seed);
    float CalculatePatternScale(int paintKit, int seed);
    float CalculatePatternRotation(int paintKit, int seed);
    std::string GeneratePatternDescription(int paintKit, int seed);
    
    // Pattern data (simplified - in reality would parse VTF/stencil data)
    struct PaintKitPatternData {
        int id = 0;
        std::string name;
        std::string patternType; // "solid", "gradient", "hydrographic", "custom", etc.
        std::vector<std::string> patternNames; // Names for specific seed ranges
        sdk::Color baseColor = {255, 255, 255, 255};
        sdk::Color accentColor = {0, 0, 0, 255};
    };
    
    std::unordered_map<int, PaintKitPatternData> m_paintKitData;
    void LoadPaintKitData();
};

} // namespace skinchanger