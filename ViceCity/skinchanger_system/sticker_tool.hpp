#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Sticker tool (memesense parity - apply/scrape/peel/position/rotate/scale)
class StickerTool {
public:
    struct StickerPreset {
        int stickerId = 0;
        float wear = 0.0f;
        float scale = 1.0f;
        float rotation = 0.0f;
        sdk::Vector2D offset = {0, 0};
    };
    
    struct StickerSlotData {
        int slot = 0;
        int stickerId = 0;
        float wear = 0.0f;
        float scale = 1.0f;
        float rotation = 0.0f;
        sdk::Vector2D offset = {0, 0};
        bool isPeeled = false;
        float peelProgress = 0.0f; // 0.0 to 1.0
        bool isScraping = false;
        float scrapeProgress = 0.0f; // 0.0 to 1.0
    };
    
    static StickerTool& Instance() {
        static StickerTool instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void Update(float dt);
    
    // Apply sticker to weapon
    bool ApplySticker(uint64_t itemId, int slot, int stickerId, const StickerPreset& preset = {});
    
    // Scrape sticker (progressive wear -> peel -> remove)
    bool ScrapeSticker(uint64_t itemId, int slot, float amount = 0.1f);
    bool PeelSticker(uint64_t itemId, int slot);
    bool RemoveSticker(uint64_t itemId, int slot);
    
    // Position/Rotate/Scale
    bool SetStickerPosition(uint64_t itemId, int slot, const sdk::Vector2D& offset);
    bool SetStickerRotation(uint64_t itemId, int slot, float rotation);
    bool SetStickerScale(uint64_t itemId, int slot, float scale);
    bool SetStickerWear(uint64_t itemId, int slot, float wear);
    
    // Get sticker data
    std::optional<StickerSlotData> GetStickerData(uint64_t itemId, int slot);
    std::vector<StickerSlotData> GetAllStickers(uint64_t itemId);
    
    // Sticker definitions
    struct StickerInfo {
        int id = 0;
        std::string name;
        std::string iconPath;
        int rarity = 0; // 0-7
        uint64_t price = 0;
        bool isHolo = false;
        bool isFoil = false;
        bool isGold = false;
        bool isTournament = false;
        int tournamentId = 0;
        int tournamentTeam = 0;
        int tournamentStage = 0;
        int tournamentPlayer = 0;
    };
    
    std::vector<StickerInfo> GetAllStickers() const;
    std::optional<StickerInfo> GetStickerInfo(int stickerId) const;
    std::vector<StickerInfo> GetStickersByRarity(int rarity) const;
    std::vector<StickerInfo> SearchStickers(const std::string& query) const;
    
    // Presets
    void SavePreset(const std::string& name, const StickerPreset& preset);
    std::optional<StickerPreset> LoadPreset(const std::string& name);
    std::vector<std::string> GetPresetNames() const;
    void DeletePreset(const std::string& name);
    
    // UI State
    struct UIState {
        uint64_t selectedItemId = 0;
        int selectedSlot = 0;
        int selectedStickerId = 0;
        StickerPreset currentPreset;
        bool showPresets = false;
        std::string searchQuery;
        int selectedRarity = -1;
        bool showOnlyOwned = false;
    };
    
    UIState& GetUIState() { return m_uiState; }

private:
    StickerTool() = default;
    UIState m_uiState;
    std::unordered_map<std::string, StickerPreset> m_presets;
    std::vector<StickerInfo> m_stickers;
    std::mutex m_mutex;
    
    void InitializeStickers();
    void LoadPresets();
    void SavePresets();
    
    // Animation helpers
    void UpdateScrapeAnimation(uint64_t itemId, int slot, float dt);
    void UpdatePeelAnimation(uint64_t itemId, int slot, float dt);
    float CalculateScrapeWear(float baseWear, float scrapeProgress);
    
    // Sticker positions per weapon (predefined)
    struct StickerPosition {
        sdk::Vector2D defaultPos;
        sdk::Vector2D minPos;
        sdk::Vector2D maxPos;
        float defaultScale = 1.0f;
        float minScale = 0.25f;
        float maxScale = 4.0f;
    };
    
    std::unordered_map<int, std::array<StickerPosition, 5>> m_stickerPositions; // itemDefIndex -> slots
    void InitializeStickerPositions();
    
    // Weapon-specific sticker slots
    static constexpr int MAX_STICKER_SLOTS = 5; // 4 for weapons, 1 for gloves
    int GetMaxSlotsForWeapon(int itemDefIndex) const;
};

} // namespace skinchanger