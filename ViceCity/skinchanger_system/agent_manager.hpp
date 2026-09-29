#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Agent Manager (memesense parity - agents, patches, equip regions)
class AgentManager {
public:
    struct Agent {
        int id = 0;
        std::string name;
        std::string modelPath;
        std::string iconPath;
        int rarity = 0;
        uint64_t price = 0;
        std::string faction; // "CT" or "T"
        std::vector<int> patches; // Available patch IDs
        bool isDefault = false;
    };
    
    struct Patch {
        int id = 0;
        std::string name;
        std::string iconPath;
        int rarity = 0;
        uint64_t price = 0;
        std::string faction; // "CT", "T", or "Both"
    };
    
    struct AgentState {
        int equippedCT = 0;       // Agent ID for CT
        int equippedT = 0;        // Agent ID for T
        int equippedPatchCT = 0;  // Patch ID for CT
        int equippedPatchT = 0;   // Patch ID for T
        int equipRegionCT = 0;    // 0=head, 1=torso, 2=legs
        int equipRegionT = 0;
    };
    
    static AgentManager& Instance() {
        static AgentManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
    // Equip/Unequip
    bool EquipAgent(int agentId, bool isCT);
    bool EquipPatch(int patchId, bool isCT);
    bool UnequipAgent(bool isCT);
    bool UnequipPatch(bool isCT);
    void SetEquipRegion(int region, bool isCT);
    
    // State
    AgentState& GetState() { return m_state; }
    const AgentState& GetState() const { return m_state; }
    
    // Definitions
    std::vector<Agent> GetAllAgents() const;
    std::vector<Agent> GetAgentsByFaction(const std::string& faction) const;
    std::optional<Agent> GetAgent(int id) const;
    std::vector<Patch> GetAllPatches() const;
    std::vector<Patch> GetPatchesByFaction(const std::string& faction) const;
    std::optional<Patch> GetPatch(int id) const;
    std::vector<Agent> GetOwnedAgents() const;
    std::vector<Patch> GetOwnedPatches() const;
    
    // Preview
    void PreviewAgent(int agentId, bool isCT);
    void StopPreview();

private:
    AgentManager() = default;
    AgentState m_state;
    std::vector<Agent> m_agents;
    std::vector<Patch> m_patches;
    int m_previewAgent = 0;
    bool m_previewIsCT = true;
    std::mutex m_mutex;
    
    void InitializeAgents();
    void InitializePatches();
    void LoadState();
    void SaveState();
    
    // Default agents
    static constexpr int DEFAULT_CT_AGENT = 1;
    static constexpr int DEFAULT_T_AGENT = 2;
};

} // namespace skinchanger