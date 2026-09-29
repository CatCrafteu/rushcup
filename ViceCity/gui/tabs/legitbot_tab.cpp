#include "core/pch.hpp"
#include "gui/tabs/legitbot_tab.hpp"
#include "features/legit/triggerbot.hpp"
#include "features/legit/backtrack.hpp"
#include "features/legit/legit_aa.hpp"
#include "features/legit/movement.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"

namespace gui {

void LegitBotTab::Initialize() {
    LOG_INFO(GUI, "LegitBotTab initialized");
}

void LegitBotTab::Shutdown() {
    LOG_INFO(GUI, "LegitBotTab shutdown");
}

void LegitBotTab::Render() {
    ImGui::BeginChild("##legitbot_subtabs", ImVec2(0, 30), false);
    
    const char* subTabs[] = {"Triggerbot", "Backtrack", "Legit AA", "Movement"};
    for (int i = 0; i < 4; ++i) {
        bool selected = static_cast<int>(m_currentSubTab) == i;
        if (ImGui::Selectable(subTabs[i], selected, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x / 4, 0))) {
            m_currentSubTab = static_cast<SubTab>(i);
        }
        if (i < 3) ImGui::SameLine();
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    
    switch (m_currentSubTab) {
        case SubTab::Triggerbot: RenderTriggerbotTab(); break;
        case SubTab::Backtrack: RenderBacktrackTab(); break;
        case SubTab::LegitAA: RenderLegitAATab(); break;
        case SubTab::Movement: RenderMovementTab(); break;
    }
}

void LegitBotTab::RenderTriggerbotTab() {
    ImGui::BeginChild("##triggerbot_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_triggerConfig.enabled);
    // Keybind widget for m_triggerConfig.keybind
    
    const char* modes[] = {"Toggle", "Hold", "Double Tap"};
    ImGui::Combo("Keybind Mode", reinterpret_cast<int*>(&m_triggerConfig.keybindMode), modes, 3);
    ImGui::Separator();
    
    ImGui::DragInt("Delay Min (ms)", &m_triggerConfig.delayMin, 1, 0, 500);
    ImGui::DragInt("Delay Max (ms)", &m_triggerConfig.delayMax, 1, 0, 500);
    ImGui::DragInt("Burst Shots", &m_triggerConfig.burstShots, 1, 1, 10);
    ImGui::DragInt("Burst Delay", &m_triggerConfig.burstDelay, 1, 0, 500);
    ImGui::Separator();
    
    RenderTriggerbotHitgroups();
    ImGui::Separator();
    
    RenderTriggerbotChecks();
    ImGui::Separator();
    
    ImGui::Checkbox("Magnum/Revolver", &m_triggerConfig.magnumRevolver);
    ImGui::DragFloat("Min Damage", &m_triggerConfig.minDamage, 0.1f, 0.0f, 100.0f);
    ImGui::DragInt("Magazine Check", &m_triggerConfig.magazineCheck, 1, 0, 30);
    
    ImGui::EndChild();
}

void LegitBotTab::RenderBacktrackTab() {
    ImGui::BeginChild("##backtrack_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_backtrackConfig.enabled);
    ImGui::DragInt("Time Limit (ms)", &m_backtrackConfig.timeLimit, 1, 1, 200);
    ImGui::Checkbox("Visualize", &m_backtrackConfig.visualize);
    ImGui::Separator();
    
    const char* bones[] = {"Head", "Neck", "Body"};
    ImGui::Combo("Bone", reinterpret_cast<int*>(&m_backtrackConfig.bone), bones, 3);
    ImGui::Separator();
    
    const char* legitModes[] = {"On Key", "On Shot", "Always"};
    ImGui::Combo("Legit Mode", reinterpret_cast<int*>(&m_backtrackConfig.legitMode), legitModes, 3);
    // Keybind widget
    
    ImGui::EndChild();
}

void LegitBotTab::RenderLegitAATab() {
    ImGui::BeginChild("##legit_aa_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_legitAAConfig.enabled);
    // Keybind widget
    
    const char* keyModes[] = {"Toggle", "Hold"};
    ImGui::Combo("Keybind Mode", reinterpret_cast<int*>(&m_legitAAConfig.keybindMode), keyModes, 2);
    ImGui::Separator();
    
    RenderLegitAAPitch();
    ImGui::Separator();
    
    RenderLegitAAYaw();
    ImGui::Separator();
    
    RenderLegitAALBY();
    
    ImGui::EndChild();
}

void LegitBotTab::RenderMovementTab() {
    ImGui::BeginChild("##movement_tab", ImVec2(0, 0), false);
    
    RenderBhopSettings();
    ImGui::Separator();
    
    RenderAutoStrafeSettings();
    ImGui::Separator();
    
    RenderEdgeJumpSettings();
    ImGui::Separator();
    
    RenderFastDuckSettings();
    ImGui::Separator();
    
    RenderSlowWalkSettings();
    ImGui::Separator();
    
    RenderAutoPeekSettings();
    
    ImGui::EndChild();
}

// Helpers
void LegitBotTab::RenderTriggerbotHitgroups() {
    ImGui::Text("Hitgroups");
    static const char* hitgroups[] = {"Head", "Chest", "Stomach", "Left Arm", "Right Arm", "Left Leg", "Right Leg"};
    for (int i = 0; i < 7; ++i) {
        bool enabled = std::find(m_triggerConfig.hitgroups.begin(), m_triggerConfig.hitgroups.end(), i + 1) != m_triggerConfig.hitgroups.end();
        if (ImGui::Checkbox(hitgroups[i], &enabled)) {
            if (enabled) m_triggerConfig.hitgroups.push_back(i + 1);
            else m_triggerConfig.hitgroups.erase(std::remove(m_triggerConfig.hitgroups.begin(), m_triggerConfig.hitgroups.end(), i + 1), m_triggerConfig.hitgroups.end());
        }
        if (i % 3 != 2) ImGui::SameLine();
    }
}

void LegitBotTab::RenderTriggerbotChecks() {
    ImGui::Text("Checks");
    ImGui::Checkbox("Visible", &m_triggerConfig.checkVisible);
    ImGui::Checkbox("Scope", &m_triggerConfig.checkScope);
    ImGui::Checkbox("Flash", &m_triggerConfig.checkFlash);
    ImGui::Checkbox("Smoke", &m_triggerConfig.checkSmoke);
    ImGui::Checkbox("Teammates", &m_triggerConfig.checkTeammates);
}

void LegitBotTab::RenderBacktrackVisualization() {
    ImGui::Text("Visualization");
    // Color pickers for records, best record, history
}

void LegitBotTab::RenderLegitAAPitch() {
    ImGui::Text("Pitch");
    const char* pitches[] = {"Off", "Down", "Up", "Jitter"};
    ImGui::Combo("Pitch", reinterpret_cast<int*>(&m_legitAAConfig.pitch), pitches, 4);
}

void LegitBotTab::RenderLegitAAYaw() {
    ImGui::Text("Yaw");
    const char* yaws[] = {"Off", "Static", "Jitter", "Freestanding"};
    ImGui::Combo("Yaw", reinterpret_cast<int*>(&m_legitAAConfig.yaw), yaws, 4);
    
    if (m_legitAAConfig.yaw == features::legit::LegitAAConfig::Yaw::Static) {
        ImGui::DragFloat("Static Yaw", &m_legitAAConfig.yawStatic, 0.1f, -180.0f, 180.0f);
    } else if (m_legitAAConfig.yaw == features::legit::LegitAAConfig::Yaw::Jitter) {
        ImGui::DragFloat("Jitter Range", &m_legitAAConfig.yawJitterRange, 0.1f, 0.0f, 180.0f);
    }
    
    ImGui::Checkbox("At Targets", &m_legitAAConfig.atTargets);
    if (m_legitAAConfig.atTargets) {
        ImGui::DragFloat("Distance", &m_legitAAConfig.atTargetsDistance, 1.0f, 0.0f, 5000.0f);
    }
}

void LegitBotTab::RenderLegitAALBY() {
    ImGui::Text("LBY Mode");
    const char* lbyModes[] = {"Off", "Opposite", "Jitter"};
    ImGui::Combo("LBY Mode", reinterpret_cast<int*>(&m_legitAAConfig.lbyMode), lbyModes, 3);
}

void LegitBotTab::RenderBhopSettings() {
    ImGui::Text("Bunny Hop");
    ImGui::Checkbox("Enabled", &m_movementConfig.bhopEnabled);
    ImGui::DragFloat("Hitchance", &m_movementConfig.bhopHitchance, 0.1f, 0.0f, 100.0f);
    ImGui::DragInt("Max Hop", &m_movementConfig.bhopMaxHop, 1, 0, 10);
    ImGui::Checkbox("Air Strafe", &m_movementConfig.airStrafe);
}

void LegitBotTab::RenderAutoStrafeSettings() {
    ImGui::Text("Auto Strafe");
    ImGui::Checkbox("Enabled", &m_movementConfig.autoStrafeEnabled);
    
    const char* modes[] = {"Silent", "Normal"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_movementConfig.autoStrafeMode), modes, 2);
    ImGui::DragFloat("Retrack Speed", &m_movementConfig.autoStrafeRetrack, 0.1f, 0.1f, 5.0f);
}

void LegitBotTab::RenderEdgeJumpSettings() {
    ImGui::Text("Edge Jump");
    ImGui::Checkbox("Enabled", &m_movementConfig.edgeJumpEnabled);
    // Keybind widget
}

void LegitBotTab::RenderFastDuckSettings() {
    ImGui::Text("Fast Duck");
    ImGui::Checkbox("Enabled", &m_movementConfig.fastDuckEnabled);
}

void LegitBotTab::RenderSlowWalkSettings() {
    ImGui::Text("Slow Walk");
    ImGui::Checkbox("Enabled", &m_movementConfig.slowWalkEnabled);
    // Keybind widget
    ImGui::DragFloat("Speed %", &m_movementConfig.slowWalkSpeed, 1.0f, 1.0f, 100.0f);
}

void LegitBotTab::RenderAutoPeekSettings() {
    ImGui::Text("Auto Peek");
    ImGui::Checkbox("Enabled", &m_movementConfig.autoPeekEnabled);
    // Keybind widget
    ImGui::Checkbox("Render", &m_movementConfig.autoPeekRender);
}

} // namespace gui