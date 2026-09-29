#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Glove Manager (memesense parity - gloves, wear, pattern seed)
class GloveManager {
public:
    struct Glove {
        int id = 0;
        std::string name;
        std::string modelPath;
        std::string iconPath;
        int rarity = 0;
        uint64_t price = 0;
        std::vector<int> paintKits; // Available skins for this glove
        bool isDefault = false;
    };
    
    struct GloveState {
        int equippedGlove = 0;
        int paintKit = 0;
        float wear = 0.001f;
        int seed = 0;
    };
    
    static GloveManager& Instance() {
        static GloveManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
    // Equip/Unequip
    bool EquipGlove(int gloveId);
    bool UnequipGlove();
    bool SetGloveSkin(int paintKit, float wear = 0.001f, int seed = 0);
    
    // State
    GloveState& GetState() { return m_state; }
    const GloveState& GetState() const { return m_state; }
    
    // Definitions
    std::vector<Glove> GetAllGloves() const;
    std::optional<Glove> GetGlove(int id) const;
    std::vector<Glove> GetOwnedGloves() const;
    std::vector<int> GetPaintKitsForGlove(int gloveId) const;
    
    // Preview
    void PreviewGlove(int gloveId, int paintKit = 0, float wear = 0.001f, int seed = 0);
    void StopPreview();
    
    // Model rendering
    void RenderGloveModel(int gloveId, int paintKit, float wear, int seed);

private:
    GloveManager() = default;
    GloveState m_state;
    std::vector<Glove> m_gloves;
    int m_previewGlove = 0;
    int m_previewPaintKit = 0;
    float m_previewWear = 0.001f;
    int m_previewSeed = 0;
    std::mutex m_mutex;
    
    void InitializeGloves();
    void LoadState();
    void SaveState();
    
    static constexpr int DEFAULT_GLOVE = 0;
};

} // namespace skinchanger