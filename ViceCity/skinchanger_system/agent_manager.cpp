#include "core/pch.hpp"
#include "skinchanger_system/agent_manager.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void AgentManager::Initialize() {
    InitializeAgents();
    InitializePatches();
    LoadState();
    LOG_INFO(Skinchanger, "AgentManager initialized");
}

void AgentManager::Shutdown() {
    SaveState();
    LOG_INFO(Skinchanger, "AgentManager shutdown");
}

void AgentManager::Render() {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Agents")) {
        // CT Agent
        ImGui::Text("CT Side Agent:");
        auto ctAgent = GetAgent(m_state.equippedCT);
        if (ctAgent) {
            ImGui::SameLine();
            ImGui::Text("%s", ctAgent->name.c_str());
        } else {
            ImGui::SameLine();
            ImGui::Text("Default");
        }
        
        if (ImGui::BeginCombo("##ct_agent", ctAgent ? ctAgent->name.c_str() : "Default")) {
            if (ImGui::Selectable("Default", m_state.equippedCT == 0)) {
                EquipAgent(0, true);
            }
            ImGui::Separator();
            
            auto ctAgents = GetAgentsByFaction("CT");
            for (const auto& agent : ctAgents) {
                bool owned = IsAgentOwned(agent.id);
                std::string label = fmt::format("{} {}", agent.name, owned ? "" : " (Locked)");
                if (ImGui::Selectable(label.c_str(), agent.id == m_state.equippedCT)) {
                    if (owned || agent.id == 0) {
                        EquipAgent(agent.id, true);
                    }
                }
            }
            ImGui::EndCombo();
        }
        
        // CT Patch
        if (m_state.equippedCT > 0) {
            ImGui::Text("CT Patch:");
            auto ctPatch = GetPatch(m_state.equippedPatchCT);
            if (ctPatch) {
                ImGui::SameLine();
                ImGui::Text("%s", ctPatch->name.c_str());
            } else {
                ImGui::SameLine();
                ImGui::Text("None");
            }
            
            if (ImGui::BeginCombo("##ct_patch", ctPatch ? ctPatch->name.c_str() : "None")) {
                if (ImGui::Selectable("None", m_state.equippedPatchCT == 0)) {
                    EquipPatch(0, true);
                }
                ImGui::Separator();
                
                auto patches = GetPatchesByFaction("CT");
                for (const auto& patch : patches) {
                    bool owned = IsPatchOwned(patch.id);
                    std::string label = fmt::format("{} {}", patch.name, owned ? "" : " (Locked)");
                    if (ImGui::Selectable(label.c_str(), patch.id == m_state.equippedPatchCT)) {
                        if (owned || patch.id == 0) {
                            EquipPatch(patch.id, true);
                        }
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        // CT Equip Region
        ImGui::Text("CT Equip Region:");
        const char* regions[] = {"Head", "Torso", "Legs"};
        if (ImGui::Combo("##ct_region", &m_state.equipRegionCT, regions, 3)) {
            SaveState();
        }
        
        ImGui::Separator();
        
        // T Agent
        ImGui::Text("T Side Agent:");
        auto tAgent = GetAgent(m_state.equippedT);
        if (tAgent) {
            ImGui::SameLine();
            ImGui::Text("%s", tAgent->name.c_str());
        } else {
            ImGui::SameLine();
            ImGui::Text("Default");
        }
        
        if (ImGui::BeginCombo("##t_agent", tAgent ? tAgent->name.c_str() : "Default")) {
            if (ImGui::Selectable("Default", m_state.equippedT == 0)) {
                EquipAgent(0, false);
            }
            ImGui::Separator();
            
            auto tAgents = GetAgentsByFaction("T");
            for (const auto& agent : tAgents) {
                bool owned = IsAgentOwned(agent.id);
                std::string label = fmt::format("{} {}", agent.name, owned ? "" : " (Locked)");
                if (ImGui::Selectable(label.c_str(), agent.id == m_state.equippedT)) {
                    if (owned || agent.id == 0) {
                        EquipAgent(agent.id, false);
                    }
                }
            }
            ImGui::EndCombo();
        }
        
        // T Patch
        if (m_state.equippedT > 0) {
            ImGui::Text("T Patch:");
            auto tPatch = GetPatch(m_state.equippedPatchT);
            if (tPatch) {
                ImGui::SameLine();
                ImGui::Text("%s", tPatch->name.c_str());
            } else {
                ImGui::SameLine();
                ImGui::Text("None");
            }
            
            if (ImGui::BeginCombo("##t_patch", tPatch ? tPatch->name.c_str() : "None")) {
                if (ImGui::Selectable("None", m_state.equippedPatchT == 0)) {
                    EquipPatch(0, false);
                }
                ImGui::Separator();
                
                auto patches = GetPatchesByFaction("T");
                for (const auto& patch : patches) {
                    bool owned = IsPatchOwned(patch.id);
                    std::string label = fmt::format("{} {}", patch.name, owned ? "" : " (Locked)");
                    if (ImGui::Selectable(label.c_str(), patch.id == m_state.equippedPatchT)) {
                        if (owned || patch.id == 0) {
                            EquipPatch(patch.id, false);
                        }
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        // T Equip Region
        ImGui::Text("T Equip Region:");
        if (ImGui::Combo("##t_region", &m_state.equipRegionT, regions, 3)) {
            SaveState();
        }
        
        ImGui::Separator();
        
        // Preview
        if (m_previewAgent > 0) {
            ImGui::Separator();
            ImGui::Text("Preview: %s", GetAgent(m_previewAgent)->name.c_str());
            if (ImGui::Button("Stop Preview")) {
                StopPreview();
            }
        }
    }
    ImGui::End();
}

bool AgentManager::EquipAgent(int agentId, bool isCT) {
    if (agentId > 0 && !IsAgentOwned(agentId)) {
        LOG_WARN(Skinchanger, "Agent not owned: %d", agentId);
        return false;
    }
    
    if (isCT) {
        m_state.equippedCT = agentId;
    } else {
        m_state.equippedT = agentId;
    }
    
    SaveState();
    LOG_INFO(Skinchanger, "Equipped agent: %d (%s)", agentId, isCT ? "CT" : "T");
    return true;
}

bool AgentManager::EquipPatch(int patchId, bool isCT) {
    if (patchId > 0 && !IsPatchOwned(patchId)) {
        LOG_WARN(Skinchanger, "Patch not owned: %d", patchId);
        return false;
    }
    
    if (isCT) {
        m_state.equippedPatchCT = patchId;
    } else {
        m_state.equippedPatchT = patchId;
    }
    
    SaveState();
    LOG_INFO(Skinchanger, "Equipped patch: %d (%s)", patchId, isCT ? "CT" : "T");
    return true;
}

bool AgentManager::UnequipAgent(bool isCT) {
    if (isCT) {
        m_state.equippedCT = 0;
    } else {
        m_state.equippedT = 0;
    }
    SaveState();
    return true;
}

bool AgentManager::UnequipPatch(bool isCT) {
    if (isCT) {
        m_state.equippedPatchCT = 0;
    } else {
        m_state.equippedPatchT = 0;
    }
    SaveState();
    return true;
}

void AgentManager::SetEquipRegion(int region, bool isCT) {
    if (isCT) {
        m_state.equipRegionCT = region;
    } else {
        m_state.equipRegionT = region;
    }
    SaveState();
}

std::vector<AgentManager::Agent> AgentManager::GetAllAgents() const {
    return m_agents;
}

std::vector<AgentManager::Agent> AgentManager::GetAgentsByFaction(const std::string& faction) const {
    std::vector<Agent> result;
    for (const auto& agent : m_agents) {
        if (agent.faction == faction) result.push_back(agent);
    }
    return result;
}

std::optional<AgentManager::Agent> AgentManager::GetAgent(int id) const {
    for (const auto& agent : m_agents) {
        if (agent.id == id) return agent;
    }
    return std::nullopt;
}

std::vector<AgentManager::Patch> AgentManager::GetAllPatches() const {
    return m_patches;
}

std::vector<AgentManager::Patch> AgentManager::GetPatchesByFaction(const std::string& faction) const {
    std::vector<Patch> result;
    for (const auto& patch : m_patches) {
        if (patch.faction == faction || patch.faction == "Both") result.push_back(patch);
    }
    return result;
}

std::optional<AgentManager::Patch> AgentManager::GetPatch(int id) const {
    for (const auto& patch : m_patches) {
        if (patch.id == id) return patch;
    }
    return std::nullopt;
}

std::vector<AgentManager::Agent> AgentManager::GetOwnedAgents() const {
    std::vector<Agent> result;
    for (const auto& agent : m_agents) {
        if (IsAgentOwned(agent.id)) result.push_back(agent);
    }
    return result;
}

std::vector<AgentManager::Patch> AgentManager::GetOwnedPatches() const {
    std::vector<Patch> result;
    for (const auto& patch : m_patches) {
        if (IsPatchOwned(patch.id)) result.push_back(patch);
    }
    return result;
}

void AgentManager::PreviewAgent(int agentId, bool isCT) {
    m_previewAgent = agentId;
    m_previewIsCT = isCT;
}

void AgentManager::StopPreview() {
    m_previewAgent = 0;
}

bool AgentManager::IsAgentOwned(int agentId) const {
    if (agentId == 0) return true; // Default always owned
    
    auto& inv = InventoryCore::Instance();
    auto agents = inv.GetAgents();
    for (const auto& agent : agents) {
        if (agent.agentIndex == agentId) return true;
    }
    return false;
}

bool AgentManager::IsPatchOwned(int patchId) const {
    if (patchId == 0) return true;
    
    auto& inv = InventoryCore::Instance();
    auto patches = inv.GetPatches(); // Would need GetPatches method
    // For now, check inventory
    return false;
}

void AgentManager::InitializeAgents() {
    m_agents = {
        // CT Agents
        {1, "FBI", "models/agents/fbi.mdl", "icons/agents/fbi.png", 1, 10000, "CT", {1, 2, 3}, false},
        {2, "ST6", "models/agents/st6.mdl", "icons/agents/st6.png", 2, 15000, "CT", {4, 5}, false},
        {3, "SWAT", "models/agents/swat.mdl", "icons/agents/swat.png", 1, 8000, "CT", {6}, false},
        {4, "GSG-9", "models/agents/gsg9.mdl", "icons/agents/gsg9.png", 2, 12000, "CT", {7, 8}, false},
        {5, "KSK", "models/agents/ksk.mdl", "icons/agents/ksk.png", 3, 20000, "CT", {9}, false},
        {6, "SAS", "models/agents/sas.mdl", "icons/agents/sas.png", 3, 25000, "CT", {10}, false},
        
        // T Agents
        {101, "Phoenix Connexion", "models/agents/phoenix.mdl", "icons/agents/phoenix.png", 1, 10000, "T", {11, 12}, false},
        {102, "Elite Crew", "models/agents/elite.mdl", "icons/agents/elite.png", 2, 15000, "T", {13, 14}, false},
        {103, "Guerrilla Warfare", "models/agents/guerrilla.mdl", "icons/agents/guerrilla.png", 1, 8000, "T", {15}, false},
        {104, "The Professionals", "models/agents/professionals.mdl", "icons/agents/professionals.png", 2, 12000, "T", {16, 17}, false},
        {105, "Balkan", "models/agents/balkan.mdl", "icons/agents/balkan.png", 3, 20000, "T", {18}, false},
    };
}

void AgentManager::InitializePatches() {
    m_patches = {
        {1, "FBI Patch", "icons/patches/fbi.png", 1, 500, "CT"},
        {2, "ST6 Patch", "icons/patches/st6.png", 2, 1000, "CT"},
        {3, "SWAT Patch", "icons/patches/swat.png", 1, 500, "CT"},
        {4, "GSG-9 Patch", "icons/patches/gsg9.png", 2, 1000, "CT"},
        {5, "KSK Patch", "icons/patches/ksk.png", 3, 2000, "CT"},
        {6, "SAS Patch", "icons/patches/sas.png", 3, 2000, "CT"},
        {7, "Phoenix Patch", "icons/patches/phoenix.png", 1, 500, "T"},
        {8, "Elite Patch", "icons/patches/elite.png", 2, 1000, "T"},
        {9, "Guerrilla Patch", "icons/patches/guerrilla.png", 1, 500, "T"},
        {10, "Professionals Patch", "icons/patches/professionals.png", 2, 1000, "T"},
        {11, "Balkan Patch", "icons/patches/balkan.png", 3, 2000, "T"},
    };
}

void AgentManager::LoadState() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto opt = config.Get<json>("skinchanger_agents", "agents");
    if (opt) {
        try {
            m_state.equippedCT = opt->value("equippedCT", 0);
            m_state.equippedT = opt->value("equippedT", 0);
            m_state.equippedPatchCT = opt->value("equippedPatchCT", 0);
            m_state.equippedPatchT = opt->value("equippedPatchT", 0);
            m_state.equipRegionCT = opt->value("equipRegionCT", 0);
            m_state.equipRegionT = opt->value("equipRegionT", 0);
        } catch (...) {}
    }
}

void AgentManager::SaveState() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["equippedCT"] = m_state.equippedCT;
    j["equippedT"] = m_state.equippedT;
    j["equippedPatchCT"] = m_state.equippedPatchCT;
    j["equippedPatchT"] = m_state.equippedPatchT;
    j["equipRegionCT"] = m_state.equipRegionCT;
    j["equipRegionT"] = m_state.equipRegionT;
    
    config.Set("skinchanger_agents", "agents", j);
}

} // namespace skinchanger