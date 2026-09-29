#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Medal Manager (memesense parity - medals, display)
class MedalManager {
public:
    struct Medal {
        int id = 0;
        std::string name;
        std::string description;
        std::string iconPath;
        int rarity = 0;
        uint64_t price = 0;
        std::string category; // "service", "tournament", "achievement", "special"
        int year = 0;
        bool isDisplayable = true;
    };
    
    struct MedalState {
        int equippedMedal = 0; // 0 = none
        std::vector<int> displayMedals; // Up to 5 displayed in profile
    };
    
    static MedalManager& Instance() {
        static MedalManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
    // Equip/Unequip
    bool EquipMedal(int medalId);
    bool UnequipMedal();
    bool AddDisplayMedal(int medalId);
    bool RemoveDisplayMedal(int medalId);
    bool SetDisplayMedals(const std::vector<int>& medals);
    
    // State
    MedalState& GetState() { return m_state; }
    const MedalState& GetState() const { return m_state; }
    
    // Definitions
    std::vector<Medal> GetAllMedals() const;
    std::vector<Medal> GetMedalsByCategory(const std::string& category) const;
    std::optional<Medal> GetMedal(int id) const;
    std::vector<Medal> GetOwnedMedals() const;
    std::vector<Medal> GetDisplayableMedals() const;
    
    // Preview
    void PreviewMedal(int medalId);
    void StopPreview();

private:
    MedalManager() = default;
    MedalState m_state;
    std::vector<Medal> m_medals;
    int m_previewMedal = 0;
    std::mutex m_mutex;
    
    void InitializeMedals();
    void LoadState();
    void SaveState();
};

} // namespace skinchanger