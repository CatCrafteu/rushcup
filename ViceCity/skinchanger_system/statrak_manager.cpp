#include "core/pch.hpp"
#include "skinchanger_system/statrak_manager.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/logger/logger.hpp"

namespace skinchanger {

void StatTrakManager::Initialize() {
    SyncWithInventory();
    LOG_INFO(Skinchanger, "StatTrakManager initialized");
}

void StatTrakManager::Shutdown() {
    LOG_INFO(Skinchanger, "StatTrakManager shutdown");
}

void StatTrakManager::OnFrameStageNotify(int stage) {
    if (stage != 0) return; // FRAME_NET_UPDATE_POSTDATAUPDATE_START
    
    // Update StatTrak from game
    // This would read actual kill counts from weapons
}

void StatTrakManager::OnFireEvent(void* event) {
    // Handle player_death event for StatTrak
    // This would parse the event and update counters
}

bool StatTrakManager::EnableSpoof(uint64_t itemId, int kills) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (itemOpt->statTrak == -1) return false; // Not a StatTrak weapon
    
    StatTrakData data;
    data.itemId = itemId;
    data.kills = kills;
    data.confirmedKills = 0;
    data.isSpoofed = true;
    data.spoofedKills = kills;
    data.lastUpdate = std::chrono::steady_clock::now();
    
    m_statTrakData[itemId] = data;
    
    // Apply to weapon in game
    // ApplyStatTrakToWeapon(...)
    
    LOG_INFO(Skinchanger, "Enabled StatTrak spoof for item %llu: %d kills", itemId, kills);
    return true;
}

bool StatTrakManager::DisableSpoof(uint64_t itemId) {
    auto it = m_statTrakData.find(itemId);
    if (it == m_statTrakData.end()) return false;
    
    it->second.isSpoofed = false;
    it->second.spoofedKills = it->second.confirmedKills;
    
    LOG_INFO(Skinchanger, "Disabled StatTrak spoof for item %llu", itemId);
    return true;
}

bool StatTrakManager::SetSpoofedKills(uint64_t itemId, int kills) {
    auto it = m_statTrakData.find(itemId);
    if (it == m_statTrakData.end()) return false;
    
    it->second.spoofedKills = kills;
    it->second.kills = kills;
    it->second.lastUpdate = std::chrono::steady_clock::now();
    
    // Apply to weapon
    // ApplyStatTrakToWeapon(...)
    
    return true;
}

bool StatTrakManager::IncrementSpoofedKills(uint64_t itemId) {
    auto it = m_statTrakData.find(itemId);
    if (it == m_statTrakData.end()) return false;
    
    it->second.spoofedKills++;
    it->second.kills = it->second.spoofedKills;
    it->second.lastUpdate = std::chrono::steady_clock::now();
    
    // Apply to weapon
    // ApplyStatTrakToWeapon(...)
    
    return true;
}

std::optional<StatTrakManager::StatTrakData> StatTrakManager::GetStatTrakData(uint64_t itemId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_statTrakData.find(itemId);
    if (it == m_statTrakData.end()) return std::nullopt;
    return it->second;
}

int StatTrakManager::GetDisplayKills(uint64_t itemId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_statTrakData.find(itemId);
    if (it == m_statTrakData.end()) {
        // Get from inventory
        auto& inv = InventoryCore::Instance();
        auto itemOpt = inv.GetItem(itemId);
        if (itemOpt) return itemOpt->statTrak;
        return -1;
    }
    
    return it->second.isSpoofed ? it->second.spoofedKills : it->second.confirmedKills;
}

void StatTrakManager::SyncWithInventory() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto& inv = InventoryCore::Instance();
    auto weapons = inv.GetWeapons();
    
    for (const auto& weapon : weapons) {
        if (weapon.statTrak >= 0) {
            auto it = m_statTrakData.find(weapon.itemId);
            if (it == m_statTrakData.end()) {
                StatTrakData data;
                data.itemId = weapon.itemId;
                data.kills = weapon.statTrak;
                data.confirmedKills = weapon.statTrak;
                data.isSpoofed = false;
                data.spoofedKills = weapon.statTrak;
                data.lastUpdate = std::chrono::steady_clock::now();
                m_statTrakData[weapon.itemId] = data;
            }
        }
    }
}

void StatTrakManager::UpdateFromGame(uint64_t itemId, int gameKills) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_statTrakData.find(itemId);
    if (it == m_statTrakData.end()) {
        StatTrakData data;
        data.itemId = itemId;
        data.kills = gameKills;
        data.confirmedKills = gameKills;
        data.isSpoofed = false;
        data.spoofedKills = gameKills;
        data.lastUpdate = std::chrono::steady_clock::now();
        m_statTrakData[itemId] = data;
    } else if (!it->second.isSpoofed) {
        it->second.confirmedKills = gameKills;
        it->second.kills = gameKills;
    }
}

void StatTrakManager::OnPlayerDeath(void* event) {
    // Parse player_death event
    // If local player killed with StatTrak weapon, increment
}

void StatTrakManager::OnRoundEnd(void* event) {
    // Sync StatTrak at round end
}

void StatTrakManager::ApplyStatTrakToWeapon(sdk::CBaseWeapon* weapon, const StatTrakData& data) {
    if (!weapon) return;
    
    // Apply StatTrak count to weapon entity
    // This would use the SDK to set the StatTrak value
}

} // namespace skinchanger