#include "core/pch.hpp"
#include "skinchanger_system/medal_manager.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void MedalManager::Initialize() {
    InitializeMedals();
    LoadState();
    LOG_INFO(Skinchanger, "MedalManager initialized");
}

void MedalManager::Shutdown() {
    SaveState();
    LOG_INFO(Skinchanger, "MedalManager shutdown");
}

void MedalManager::Render() {
    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Medals")) {
        // Equipped medal
        ImGui::Text("Equipped Medal:");
        auto medal = GetMedal(m_state.equippedMedal);
        if (medal) {
            ImGui::SameLine();
            ImGui::Text("%s", medal->name.c_str());
        } else {
            ImGui::SameLine();
            ImGui::Text("None");
        }
        
        if (ImGui::BeginCombo("##medal", medal ? medal->name.c_str() : "None")) {
            if (ImGui::Selectable("None", m_state.equippedMedal == 0)) {
                EquipMedal(0);
            }
            ImGui::Separator();
            
            auto medals = GetOwnedMedals();
            for (const auto& m : medals) {
                bool owned = IsMedalOwned(m.id);
                std::string label = fmt::format("{} {}", m.name, owned ? "" : " (Locked)");
                if (ImGui::Selectable(label.c_str(), m.id == m_state.equippedMedal)) {
                    if (owned || m.id == 0) {
                        EquipMedal(m.id);
                    }
                }
            }
            ImGui::EndCombo();
        }
        
        // Display medals
        ImGui::Separator();
        ImGui::Text("Display Medals (Profile):");
        
        for (int i = 0; i < 5; ++i) {
            ImGui::PushID(i);
            
            int medalId = (i < (int)m_state.displayMedals.size()) ? m_state.displayMedals[i] : 0;
            auto m = GetMedal(medalId);
            
            std::string label = m ? m->name : "Empty Slot";
            if (ImGui::BeginCombo(fmt::format("Slot {}", i + 1).c_str(), label.c_str())) {
                if (ImGui::Selectable("Empty", medalId == 0)) {
                    if (medalId != 0) {
                        m_state.displayMedals[i] = 0;
                        SaveState();
                    }
                }
                ImGui::Separator();
                
                auto displayable = GetDisplayableMedals();
                for (const auto& d : displayable) {
                    bool owned = IsMedalOwned(d.id);
                    std::string lbl = fmt::format("{} {}", d.name, owned ? "" : " (Locked)");
                    if (ImGui::Selectable(lbl.c_str(), d.id == medalId)) {
                        if (owned || d.id == 0) {
                            m_state.displayMedals[i] = d.id;
                            SaveState();
                        }
                    }
                }
                ImGui::EndCombo();
            }
            
            ImGui::PopID();
        }
        
        // Preview
        if (m_previewMedal > 0) {
            ImGui::Separator();
            ImGui::Text("Preview: %s", GetMedal(m_previewMedal)->name.c_str());
            ImGui::Text("Description: %s", GetMedal(m_previewMedal)->description.c_str());
            
            if (ImGui::Button("Stop Preview")) {
                StopPreview();
            }
        }
    }
    ImGui::End();
}

bool MedalManager::EquipMedal(int medalId) {
    if (medalId > 0 && !IsMedalOwned(medalId)) {
        LOG_WARN(Skinchanger, "Medal not owned: %d", medalId);
        return false;
    }
    
    m_state.equippedMedal = medalId;
    SaveState();
    LOG_INFO(Skinchanger, "Equipped medal: %d", medalId);
    return true;
}

bool MedalManager::UnequipMedal() {
    m_state.equippedMedal = 0;
    SaveState();
    return true;
}

bool MedalManager::AddDisplayMedal(int medalId) {
    if (medalId > 0 && !IsMedalOwned(medalId)) return false;
    if (m_state.displayMedals.size() >= 5) return false;
    if (std::find(m_state.displayMedals.begin(), m_state.displayMedals.end(), medalId) != m_state.displayMedals.end()) return false;
    
    m_state.displayMedals.push_back(medalId);
    SaveState();
    return true;
}

bool MedalManager::RemoveDisplayMedal(int medalId) {
    auto it = std::find(m_state.displayMedals.begin(), m_state.displayMedals.end(), medalId);
    if (it == m_state.displayMedals.end()) return false;
    
    m_state.displayMedals.erase(it);
    SaveState();
    return true;
}

bool MedalManager::SetDisplayMedals(const std::vector<int>& medals) {
    if (medals.size() > 5) return false;
    
    for (int id : medals) {
        if (id > 0 && !IsMedalOwned(id)) return false;
    }
    
    m_state.displayMedals = medals;
    SaveState();
    return true;
}

std::vector<MedalManager::Medal> MedalManager::GetAllMedals() const {
    return m_medals;
}

std::vector<MedalManager::Medal> MedalManager::GetMedalsByCategory(const std::string& category) const {
    std::vector<Medal> result;
    for (const auto& medal : m_medals) {
        if (medal.category == category) result.push_back(medal);
    }
    return result;
}

std::optional<MedalManager::Medal> MedalManager::GetMedal(int id) const {
    for (const auto& medal : m_medals) {
        if (medal.id == id) return medal;
    }
    return std::nullopt;
}

std::vector<MedalManager::Medal> MedalManager::GetOwnedMedals() const {
    std::vector<Medal> result;
    for (const auto& medal : m_medals) {
        if (IsMedalOwned(medal.id)) result.push_back(medal);
    }
    return result;
}

std::vector<MedalManager::Medal> MedalManager::GetDisplayableMedals() const {
    std::vector<Medal> result;
    for (const auto& medal : m_medals) {
        if (medal.isDisplayable && IsMedalOwned(medal.id)) result.push_back(medal);
    }
    return result;
}

void MedalManager::PreviewMedal(int medalId) {
    m_previewMedal = medalId;
}

void MedalManager::StopPreview() {
    m_previewMedal = 0;
}

bool MedalManager::IsMedalOwned(int medalId) const {
    if (medalId == 0) return true;
    
    auto& inv = InventoryCore::Instance();
    auto medals = inv.GetMedals();
    for (const auto& medal : medals) {
        if (medal.medalIndex == medalId) return true;
    }
    return false;
}

void MedalManager::InitializeMedals() {
    m_medals = {
        // Service Medals
        {1, "5 Year Veteran Coin", "5 years of service", "icons/medals/5year.png", 3, 5000, "service", 2019, true},
        {2, "10 Year Veteran Coin", "10 years of service", "icons/medals/10year.png", 4, 15000, "service", 2014, true},
        {3, "15 Year Veteran Coin", "15 years of service", "icons/medals/15year.png", 5, 50000, "service", 2009, true},
        
        // Tournament Medals
        {10, "ESL One Katowice 2015", "Tournament participant", "icons/medals/katowice_2015.png", 4, 10000, "tournament", 2015, true},
        {11, "ESL One Cologne 2015", "Tournament participant", "icons/medals/cologne_2015.png", 4, 10000, "tournament", 2015, true},
        {12, "DreamHack Cluj-Napoca 2015", "Tournament participant", "icons/medals/cluj_2015.png", 4, 8000, "tournament", 2015, true},
        {13, "MLG Columbus 2016", "Tournament participant", "icons/medals/mlg_2016.png", 4, 8000, "tournament", 2016, true},
        {14, "ELEAGUE Atlanta 2017", "Tournament participant", "icons/medals/eleague_2017.png", 4, 10000, "tournament", 2017, true},
        {15, "PGL Krakow 2017", "Tournament participant", "icons/medals/pgl_2017.png", 4, 10000, "tournament", 2017, true},
        {16, "ELEAGUE Boston 2018", "Tournament participant", "icons/medals/boston_2018.png", 5, 15000, "tournament", 2018, true},
        {17, "FACEIT London 2018", "Tournament participant", "icons/medals/faceit_2018.png", 5, 15000, "tournament", 2018, true},
        {18, "IEM Katowice 2019", "Tournament participant", "icons/medals/katowice_2019.png", 5, 15000, "tournament", 2019, true},
        {19, "StarLadder Berlin 2019", "Tournament participant", "icons/medals/starseries_2019.png", 5, 15000, "tournament", 2019, true},
        {20, "PGL Stockholm 2021", "Tournament participant", "icons/medals/stockholm_2021.png", 5, 20000, "tournament", 2021, true},
        {21, "PGL Antwerp 2022", "Tournament participant", "icons/medals/antwerp_2022.png", 5, 20000, "tournament", 2022, true},
        {22, "IEM Rio 2022", "Tournament participant", "icons/medals/rio_2022.png", 5, 20000, "tournament", 2022, true},
        {23, "BLAST Paris 2023", "Tournament participant", "icons/medals/paris_2023.png", 5, 20000, "tournament", 2023, true},
        {24, "Copenhagen 2024", "Tournament participant", "icons/medals/copenhagen_2024.png", 5, 25000, "tournament", 2024, true},
        
        // Achievement Medals
        {50, "Guardian Elite", "Complete Guardian missions on Elite difficulty", "icons/medals/guardian_elite.png", 3, 5000, "achievement", 0, true},
        {51, "Operation Hydra Participant", "Participated in Operation Hydra", "icons/medals/hydra.png", 4, 10000, "achievement", 2017, true},
        {52, "Operation Wildfire Participant", "Participated in Operation Wildfire", "icons/medals/wildfire.png", 4, 10000, "achievement", 2016, true},
        {53, "Operation Bloodhound Participant", "Participated in Operation Bloodhound", "icons/medals/bloodhound.png", 4, 10000, "achievement", 2015, true},
        {54, "Operation Phoenix Participant", "Participated in Operation Phoenix", "icons/medals/phoenix.png", 4, 10000, "achievement", 2014, true},
        {55, "Operation Bravo Participant", "Participated in Operation Bravo", "icons/medals/bravo.png", 4, 10000, "achievement", 2013, true},
        {56, "Operation Payback Participant", "Participated in Operation Payback", "icons/medals/payback.png", 4, 10000, "achievement", 2013, true},
        
        // Special Medals
        {100, "Perfect World 5 Year", "Perfect World 5 year anniversary", "icons/medals/pw_5year.png", 5, 50000, "special", 2020, true},
        {101, "Perfect World 10 Year", "Perfect World 10 year anniversary", "icons/medals/pw_10year.png", 6, 100000, "special", 2015, true},
    };
}

void MedalManager::LoadState() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto opt = config.Get<json>("skinchanger_medals", "medals");
    if (opt) {
        try {
            m_state.equippedMedal = opt->value("equippedMedal", 0);
            m_state.displayMedals = opt->value("displayMedals", std::vector<int>{});
        } catch (...) {}
    }
}

void MedalManager::SaveState() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["equippedMedal"] = m_state.equippedMedal;
    j["displayMedals"] = m_state.displayMedals;
    
    config.Set("skinchanger_medals", "medals", j);
}

} // namespace skinchanger