#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Music Kit Manager (memesense parity)
class MusicKitManager {
public:
    struct MusicKit {
        int id = 0;
        std::string name;
        std::string description;
        std::string mainMenuPath;
        std::string roundStartPath;
        std::string roundEndPath;
        std::string bombPlantPath;
        std::string bombDefusePath;
        std::string bombExplodePath;
        std::string roundTenSecPath;
        std::string mvpPath;
        int rarity = 0;
        uint64_t price = 0;
        bool isDefault = false;
    };
    
    struct MusicKitState {
        int equippedKit = 0;        // 0 = default
        int equippedMVP = 0;        // 0 = default
        float mainMenuVolume = 1.0f;
        float roundVolume = 1.0f;
        bool enableMainMenu = true;
        bool enableRound = true;
        bool enableMVP = true;
    };
    
    static MusicKitManager& Instance() {
        static MusicKitManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Update(float dt);
    void Render();
    
    // Equip/Unequip
    bool EquipMusicKit(int kitId);
    bool EquipMVP(int kitId);
    bool UnequipMusicKit();
    bool UnequipMVP();
    
    // Preview
    void PreviewMusicKit(int kitId);
    void StopPreview();
    bool IsPreviewing() const { return m_previewKit != 0; }
    int GetPreviewKit() const { return m_previewKit; }
    
    // State
    MusicKitState& GetState() { return m_state; }
    const MusicKitState& GetState() const { return m_state; }
    
    // Definitions
    std::vector<MusicKit> GetAllMusicKits() const;
    std::optional<MusicKit> GetMusicKit(int id) const;
    std::vector<MusicKit> GetOwnedMusicKits() const;
    std::vector<MusicKit> GetMusicKitsByRarity(int rarity) const;
    
    // Playback
    void PlayMainMenu();
    void PlayRoundStart();
    void PlayRoundEnd(bool won);
    void PlayBombPlant();
    void PlayBombDefuse();
    void PlayBombExplode();
    void PlayTenSecondWarning();
    void PlayMVP();
    
    // Volume
    void SetMainMenuVolume(float vol) { m_state.mainMenuVolume = std::clamp(vol, 0.0f, 1.0f); }
    void SetRoundVolume(float vol) { m_state.roundVolume = std::clamp(vol, 0.0f, 1.0f); }

private:
    MusicKitManager() = default;
    MusicKitState m_state;
    std::vector<MusicKit> m_musicKits;
    int m_previewKit = 0;
    float m_previewTime = 0.0f;
    bool m_previewPlaying = false;
    std::mutex m_mutex;
    
    void InitializeMusicKits();
    void LoadState();
    void SaveState();
    void PlayMusic(const std::string& path, float volume);
    void StopAllMusic();
    
    // Default music kit (id 0)
    static constexpr int DEFAULT_KIT = 0;
};

} // namespace skinchanger