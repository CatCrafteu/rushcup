#include "core/pch.hpp"
#include "skinchanger_system/pattern_seed.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void PatternSeed::Initialize() {
    LoadPaintKitData();
    LOG_INFO(Skinchanger, "PatternSeed initialized");
}

void PatternSeed::Shutdown() {
    LOG_INFO(Skinchanger, "PatternSeed shutdown");
}

void PatternSeed::Render() {
    if (m_state.currentPaintKit == 0) return;
    
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Pattern Seed Browser")) {
        // Current paint kit
        ImGui::Text("Paint Kit: %s (#%d)", GetPaintKitName(m_state.currentPaintKit).c_str(), m_state.currentPaintKit);
        ImGui::Separator();
        
        // Seed controls
        ImGui::DragInt("Seed", &m_state.currentSeed, 1, 0, 1000);
        ImGui::SameLine();
        if (ImGui::Button("Random")) RandomSeed();
        ImGui::SameLine();
        if (ImGui::Button("Previous")) PreviousSeed();
        ImGui::SameLine();
        if (ImGui::Button("Next")) NextSeed();
        
        ImGui::DragFloat("Wear", &m_state.currentWear, 0.0001f, 0.0f, 1.0f, "%.4f");
        ImGui::DragInt("Weapon", &m_state.itemDefIndex, 1, 0, 10000);
        
        ImGui::Separator();
        
        // Preview
        if (m_state.showPreview) {
            RenderPreview();
        }
        
        // Details
        if (m_state.showDetails) {
            RenderDetails();
        }
        
        // Search
        ImGui::Separator();
        if (ImGui::Button("Find Seeds by Pattern")) {
            // Open search dialog
        }
        ImGui::SameLine();
        if (ImGui::Button("Find Seeds by Color")) {
            // Open color search dialog
        }
    }
    ImGui::End();
}

void PatternSeed::Update(float dt) {
    UpdatePreviewTexture();
}

void PatternSeed::SetPaintKit(int paintKit) {
    m_state.currentPaintKit = paintKit;
    m_state.currentSeed = 0;
    UpdatePreviewTexture();
}

void PatternSeed::SetSeed(int seed) {
    m_state.currentSeed = std::clamp(seed, 0, 1000);
    UpdatePreviewTexture();
}

void PatternSeed::SetWear(float wear) {
    m_state.currentWear = std::clamp(wear, 0.0f, 1.0f);
    UpdatePreviewTexture();
}

void PatternSeed::SetItemDefIndex(int itemDefIndex) {
    m_state.itemDefIndex = itemDefIndex;
}

void PatternSeed::NextSeed() {
    m_state.currentSeed = (m_state.currentSeed + 1) % 1001;
    UpdatePreviewTexture();
}

void PatternSeed::PreviousSeed() {
    m_state.currentSeed = (m_state.currentSeed - 1 + 1001) % 1001;
    UpdatePreviewTexture();
}

void PatternSeed::RandomSeed() {
    m_state.currentSeed = rand() % 1001;
    UpdatePreviewTexture();
}

std::optional<PatternSeed::PatternInfo> PatternSeed::GetPatternInfo(int paintKit, int seed, float wear) {
    std::string cacheKey = fmt::format("{}_{}_{:.4f}", paintKit, seed, wear);
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(cacheKey);
        if (it != m_cache.end()) return it->second;
    }
    
    PatternInfo info = GeneratePatternInfo(paintKit, seed, wear);
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache[cacheKey] = info;
    }
    
    return info;
}

std::vector<PatternSeed::PatternInfo> PatternSeed::GetPatternRange(int paintKit, int startSeed, int endSeed, float wear) {
    std::vector<PatternInfo> result;
    startSeed = std::clamp(startSeed, 0, 1000);
    endSeed = std::clamp(endSeed, 0, 1000);
    
    if (startSeed > endSeed) std::swap(startSeed, endSeed);
    
    for (int seed = startSeed; seed <= endSeed; ++seed) {
        auto info = GetPatternInfo(paintKit, seed, wear);
        if (info) result.push_back(*info);
    }
    
    return result;
}

std::vector<int> PatternSeed::FindSeedsWithPattern(int paintKit, const std::string& patternName, float wear) {
    std::vector<int> result;
    
    auto dataIt = m_paintKitData.find(paintKit);
    if (dataIt == m_paintKitData.end()) return result;
    
    const auto& data = dataIt->second;
    std::string lowerPattern = patternName;
    std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);
    
    for (size_t i = 0; i < data.patternNames.size(); ++i) {
        std::string lowerName = data.patternNames[i];
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerPattern) != std::string::npos) {
            // This pattern name corresponds to a seed range
            // For simplicity, return the seed index * 100
            result.push_back(static_cast<int>(i * 100));
        }
    }
    
    return result;
}

std::vector<int> PatternSeed::FindSeedsByColor(int paintKit, const sdk::Color& targetColor, float tolerance, float wear) {
    std::vector<int> result;
    
    // This would analyze the actual pattern texture
    // For now, return empty
    return result;
}

PatternSeed::PatternSeedState& PatternSeed::GetState() {
    return m_state;
}

const PatternSeed::PatternSeedState& PatternSeed::GetState() const {
    return m_state;
}

void PatternSeed::GeneratePreviewTexture(int paintKit, int seed, float wear, int width, int height) {
    // Would generate actual texture from pattern data
    // Placeholder - creates a simple colored rectangle
}

void PatternSeed::UpdatePreviewTexture() {
    if (m_state.currentPaintKit > 0) {
        GeneratePreviewTexture(m_state.currentPaintKit, m_state.currentSeed, m_state.currentWear, m_state.previewSize, m_state.previewSize);
    }
}

void PatternSeed::RenderPreview() {
    auto info = GetPatternInfo(m_state.currentPaintKit, m_state.currentSeed, m_state.currentWear);
    if (!info) return;
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float size = static_cast<float>(m_state.previewSize);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Background checkerboard
    const int checkerSize = 16;
    for (int x = 0; x < size; x += checkerSize) {
        for (int y = 0; y < size; y += checkerSize) {
            bool dark = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            ImU32 c = dark ? IM_COL32(180, 180, 180, 255) : IM_COL32(220, 220, 220, 255);
            drawList->AddRectFilled(
                ImVec2(pos.x + x, pos.y + y),
                ImVec2(pos.x + x + checkerSize, pos.y + y + checkerSize),
                c
            );
        }
    }
    
    // Pattern preview (placeholder - solid color based on paint kit)
    ImU32 patternColor = IM_COL32(100 + (m_state.currentPaintKit % 155), 100 + (m_state.currentSeed % 155), 100 + ((m_state.currentPaintKit + m_state.currentSeed) % 155), 255);
    drawList->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), patternColor, 4.0f);
    drawList->AddRect(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(255, 215, 0, 255), 4.0f, 0, 2.0f);
    
    ImGui::Dummy(ImVec2(size, size));
}

void PatternSeed::RenderDetails() {
    auto info = GetPatternInfo(m_state.currentPaintKit, m_state.currentSeed, m_state.currentWear);
    if (!info) return;
    
    ImGui::Text("Description: %s", info->description.c_str());
    ImGui::Text("Pattern Offset: (%.2f, %.2f)", info->patternOffset.x, info->patternOffset.y);
    ImGui::Text("Pattern Scale: %.2f", info->patternScale);
    ImGui::Text("Pattern Rotation: %.1f", info->patternRotation);
}

void PatternSeed::InitializePatternData() {
    // Already done in LoadPaintKitData
}

PatternSeed::PatternInfo PatternSeed::GeneratePatternInfo(int paintKit, int seed, float wear) {
    PatternInfo info;
    info.paintKit = paintKit;
    info.seed = seed;
    info.wear = wear;
    info.description = GetPatternDescription(paintKit, seed);
    info.patternOffset = CalculatePatternOffset(paintKit, seed);
    info.patternScale = CalculatePatternScale(paintKit, seed);
    info.patternRotation = CalculatePatternRotation(paintKit, seed);
    
    return info;
}

sdk::Vector2D PatternSeed::CalculatePatternOffset(int paintKit, int seed) {
    // Pattern offset based on seed
    float x = static_cast<float>(seed % 100) / 100.0f;
    float y = static_cast<float>(seed / 100) / 10.0f;
    return {x, y};
}

float PatternSeed::CalculatePatternScale(int paintKit, int seed) {
    // Pattern scale varies slightly with seed
    return 1.0f + (static_cast<float>(seed % 10) / 100.0f);
}

float PatternSeed::CalculatePatternRotation(int paintKit, int seed) {
    // Pattern rotation based on seed
    return static_cast<float>(seed % 360);
}

std::string PatternSeed::GeneratePatternDescription(int paintKit, int seed) {
    auto dataIt = m_paintKitData.find(paintKit);
    if (dataIt == m_paintKitData.end()) {
        return fmt::format("Pattern for paint kit {}", paintKit);
    }
    
    const auto& data = dataIt->second;
    if (!data.patternNames.empty()) {
        int patternIdx = seed % data.patternNames.size();
        return fmt::format("{} - {}", data.name, data.patternNames[patternIdx]);
    }
    
    return fmt::format("{} - Seed {}", data.name, seed);
}

std::string PatternSeed::GetPaintKitName(int paintKit) const {
    auto it = m_paintKitData.find(paintKit);
    if (it != m_paintKitData.end()) return it->second.name;
    return fmt::format("Paint Kit #{}", paintKit);
}

void PatternSeed::LoadPaintKitData() {
    // Initialize with known paint kit data
    // This is a small subset - real implementation would have all 1000+ paint kits
    
    m_paintKitData[1] = {1, "Desert Eagle | Blaze", "custom", {"Default", "Flame", "Inferno", "Phoenix"}, {255, 100, 0, 255}, {255, 200, 0, 255}};
    m_paintKitData[2] = {2, "AK-47 | Redline", "hydrographic", {"Default", "Clean", "Scratched", "Worn"}, {200, 0, 0, 255}, {100, 100, 100, 255}};
    m_paintKitData[3] = {3, "AWP | Asiimov", "custom", {"Default", "Clean", "Orange", "Black"}, {255, 255, 255, 255}, {0, 100, 200, 255}};
    m_paintKitData[4] = {4, "M4A4 | Howl", "custom", {"Default", "Contraband"}, {255, 50, 50, 255}, {255, 200, 0, 255}};
    m_paintKitData[5] = {5, "Karambit | Doppler", "custom", {"Phase 1", "Phase 2", "Phase 3", "Phase 4", "Ruby", "Sapphire", "Black Pearl"}, {100, 0, 200, 255}, {255, 0, 100, 255}};
    m_paintKitData[6] = {6, "Butterfly Knife | Fade", "custom", {"Default", "Full Fade", "Tip Fade"}, {255, 0, 100, 255}, {100, 255, 0, 255}};
    m_paintKitData[7] = {7, "Glock-18 | Fade", "custom", {"Default", "Full Fade", "Partial"}, {100, 200, 255, 255}, {255, 100, 0, 255}};
    m_paintKitData[8] = {8, "USP-S | Kill Confirmed", "custom", {"Default", "Clean", "Bloody"}, {200, 200, 200, 255}, {255, 50, 50, 255}};
    m_paintKitData[9] = {9, "AWP | Dragon Lore", "custom", {"Default", "Clean", "Gold"}, {200, 150, 0, 255}, {100, 50, 0, 255}};
    m_paintKitData[10] = {10, "M4A1-S | Knight", "custom", {"Default", "Clean", "Gold Trim"}, {50, 50, 100, 255}, {200, 180, 0, 255}};
    
    // Add more paint kits as needed
    for (int i = 11; i < 1000; ++i) {
        m_paintKitData[i] = {i, fmt::format("Paint Kit #{}", i), "solid", {"Default"}, 
                           sdk::Color{static_cast<uint8_t>(i % 255), static_cast<uint8_t>((i * 2) % 255), static_cast<uint8_t>((i * 3) % 255), 255},
                           sdk::Color{static_cast<uint8_t>((i * 4) % 255), static_cast<uint8_t>((i * 5) % 255), static_cast<uint8_t>((i * 6) % 255), 255}};
    }
}

} // namespace skinchanger