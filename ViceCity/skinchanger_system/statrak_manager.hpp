#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// StatTrak Manager (client-side counter spoof)
class StatTrakManager {
public:
    struct StatTrakData {
        uint64_t itemId = 0;
        int kills = 0;
        int confirmedKills = 0; // Actual game kills
        bool isSpoofed = false;
        int spoofedKills = 0;
        std::chrono::steady_clock::time_point lastUpdate;
    };
    
    static StatTrakManager& Instance() {
        static StatTrakManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnFrameStageNotify(int stage);
    void OnFireEvent(void* event);
    
    // Spoofing
    bool EnableSpoof(uint64_t itemId, int kills);
    bool DisableSpoof(uint64_t itemId);
    bool SetSpoofedKills(uint64_t itemId, int kills);
    bool IncrementSpoofedKills(uint64_t itemId);
    
    // Get data
    std::optional<StatTrakData> GetStatTrakData(uint64_t itemId);
    int GetDisplayKills(uint64_t itemId) const;
    
    // Sync with inventory
    void SyncWithInventory();
    void UpdateFromGame(uint64_t itemId, int gameKills);

private:
    StatTrakManager() = default;
    std::unordered_map<uint64_t, StatTrakData> m_statTrakData;
    std::mutex m_mutex;
    
    void OnPlayerDeath(void* event);
    void OnRoundEnd(void* event);
    void ApplyStatTrakToWeapon(sdk::CBaseWeapon* weapon, const StatTrakData& data);
};

} // namespace skinchanger