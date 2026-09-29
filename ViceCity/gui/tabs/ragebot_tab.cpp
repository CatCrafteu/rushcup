#include "core/pch.hpp"
#include "gui/tabs/ragebot_tab.hpp"
#include "features/rage/rage_aimbot.hpp"
#include "features/rage/exploitation.hpp"
#include "features/rage/anti_aim.hpp"
#include "features/rage/resolver.hpp"
#include "features/rage/autowall.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"

namespace gui {

void RageBotTab::Initialize() {
    LOG_INFO(GUI, "RageBotTab initialized");
}

void RageBotTab::Shutdown() {
    LOG_INFO(GUI, "RageBotTab shutdown");
}

void RageBotTab::Render() {
    ImGui::BeginChild("##ragebot_subtabs", ImVec2(0, 30), false);
    
    const char* subTabs[] = {"Aim", "Exploits", "Anti-Aim", "Resolver", "Logs"};
    for (int i = 0; i < 5; ++i) {
        bool selected = static_cast<int>(m_currentSubTab) == i;
        if (ImGui::Selectable(subTabs[i], selected, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x / 5, 0))) {
            m_currentSubTab = static_cast<SubTab>(i);
        }
        if (i < 4) ImGui::SameLine();
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    
    switch (m_currentSubTab) {
        case SubTab::Aim: RenderAimTab(); break;
        case SubTab::Exploits: RenderExploitsTab(); break;
        case SubTab::AntiAim: RenderAntiAimTab(); break;
        case SubTab::Resolver: RenderResolverTab(); break;
        case SubTab::Logs: RenderLogsTab(); break;
    }
}

void RageBotTab::RenderAimTab() {
    ImGui::BeginChild("##aim_tab", ImVec2(0, 0), false);
    
    // Enable
    ImGui::Checkbox("Enabled", &m_aimConfig.enabled);
    ImGui::Checkbox("Silent Aim", &m_aimConfig.silentAim);
    ImGui::Checkbox("Auto Scope", &m_aimConfig.autoScope);
    ImGui::Checkbox("Auto Stop", &m_aimConfig.autoStop);
    ImGui::Separator();
    
    // Target Selection
    const char* targetSel[] = {"FOV", "Distance", "Damage", "Health", "Threat"};
    ImGui::Combo("Target Selection", reinterpret_cast<int*>(&m_aimConfig.targetSelection), targetSel, 5);
    ImGui::Separator();
    
    // Hitbox Priority
    RenderHitboxPriority();
    ImGui::Separator();
    
    // Multipoint
    RenderMultipointSettings();
    ImGui::Separator();
    
    // Hitchance
    RenderHitchanceSettings();
    ImGui::Separator();
    
    // Minimum Damage
    RenderMinimumDamage();
    ImGui::Separator();
    
    // Autowall
    RenderAutowallSettings();
    ImGui::Separator();
    
    // Prefer Body Aim
    RenderPreferBodyAim();
    
    ImGui::EndChild();
}

void RageBotTab::RenderExploitsTab() {
    ImGui::BeginChild("##exploits_tab", ImVec2(0, 0), false);
    
    // Double Tap
    RenderDoubleTap();
    ImGui::Separator();
    
    // Hide Shots
    RenderHideShots();
    ImGui::Separator();
    
    // Fake Lag
    RenderFakeLag();
    ImGui::Separator();
    
    // Quick Stop
    RenderQuickStop();
    ImGui::Separator();
    
    // Quick Peek
    RenderQuickPeek();
    
    ImGui::EndChild();
}

void RageBotTab::RenderAntiAimTab() {
    ImGui::BeginChild("##aa_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_aaConfig.enabled);
    ImGui::Separator();
    
    // Pitch
    RenderPitchSettings();
    ImGui::Separator();
    
    // Yaw Base
    const char* yawBase[] = {"Local View", "At Targets", "Freestanding"};
    ImGui::Combo("Yaw Base", reinterpret_cast<int*>(&m_aaConfig.yawBase), yawBase, 3);
    ImGui::Separator();
    
    // Yaw
    RenderYawSettings();
    ImGui::Separator();
    
    // Desync
    RenderDesyncSettings();
    ImGui::Separator();
    
    // LBY Breaker
    RenderLBYBreaker();
    ImGui::Separator();
    
    // Freestanding
    RenderFreestanding();
    ImGui::Separator();
    
    // Manual AA
    RenderManualAA();
    
    ImGui::EndChild();
}

void RageBotTab::RenderResolverTab() {
    ImGui::BeginChild("##resolver_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_resolverConfig.enabled);
    
    const char* modes[] = {"Brute", "History", "ML"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_resolverConfig.mode), modes, 3);
    
    ImGui::Checkbox("Log Misses", &m_resolverConfig.logMisses);
    ImGui::DragInt("Update Rate", &m_resolverConfig.updateRate, 1, 1, 64);
    ImGui::DragInt("Max History", &m_resolverConfig.maxHistory, 1, 1, 256);
    ImGui::Separator();
    
    RenderAnimFixSettings();
    
    ImGui::EndChild();
}

void RageBotTab::RenderLogsTab() {
    ImGui::BeginChild("##logs_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Hit Logs", &m_aimConfig.enabled); // Would use separate config
    ImGui::Checkbox("Damage Logs", &m_aimConfig.enabled);
    ImGui::Checkbox("Purchase Logs", &m_aimConfig.enabled);
    ImGui::Checkbox("Console Logs", &m_aimConfig.enabled);
    ImGui::Separator();
    
    ImGui::Checkbox("Filter Local", &m_aimConfig.enabled);
    ImGui::Checkbox("Filter Teammates", &m_aimConfig.enabled);
    ImGui::Checkbox("Filter Enemies", &m_aimConfig.enabled);
    
    ImGui::EndChild();
}

// Helper implementations (simplified)
void RageBotTab::RenderHitboxPriority() {
    ImGui::Text("Hitbox Priority");
    static const char* hitboxes[] = {"Head", "Neck", "Chest", "Stomach", "Pelvis", "Arms", "Legs"};
    for (int i = 0; i < 7; ++i) {
        bool enabled = std::find(m_aimConfig.hitboxPriority.begin(), m_aimConfig.hitboxPriority.end(), i) != m_aimConfig.hitboxPriority.end();
        if (ImGui::Checkbox(hitboxes[i], &enabled)) {
            if (enabled) m_aimConfig.hitboxPriority.push_back(i);
            else m_aimConfig.hitboxPriority.erase(std::remove(m_aimConfig.hitboxPriority.begin(), m_aimConfig.hitboxPriority.end(), i), m_aimConfig.hitboxPriority.end());
        }
        if (i % 3 != 2) ImGui::SameLine();
    }
}

void RageBotTab::RenderMultipointSettings() {
    ImGui::Text("Multipoint");
    ImGui::Checkbox("Enabled", &m_aimConfig.multipoint.enabled);
    ImGui::DragFloat("Head Scale", &m_aimConfig.multipoint.headScale, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Body Scale", &m_aimConfig.multipoint.bodyScale, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Point Scale", &m_aimConfig.multipoint.pointScale, 0.01f, 0.0f, 1.0f);
    
    const char* headPoints[] = {"3", "5", "9", "13", "17"};
    ImGui::Combo("Head Points", &m_aimConfig.multipoint.headPoints, headPoints, 5);
}

void RageBotTab::RenderHitchanceSettings() {
    ImGui::Text("Hitchance");
    ImGui::Checkbox("Enabled", &m_aimConfig.hitchance.enabled);
    ImGui::DragFloat("Minimum", &m_aimConfig.hitchance.minimum, 0.1f, 0.0f, 100.0f);
    
    const char* modes[] = {"Standard", "Advanced"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_aimConfig.hitchance.mode), modes, 2);
    ImGui::Checkbox("Seed Sync", &m_aimConfig.hitchance.seedSync);
}

void RageBotTab::RenderMinimumDamage() {
    ImGui::Text("Minimum Damage");
    ImGui::DragFloat("Visible", &m_aimConfig.minimumDamage.visible, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Auto Wall", &m_aimConfig.minimumDamage.autoWall, 0.1f, 0.0f, 100.0f);
    
    // Override keybind would use Keybind widget
    ImGui::DragFloat("Override Value", &m_aimConfig.minimumDamage.overrideValue, 0.1f, 0.0f, 100.0f);
}

void RageBotTab::RenderAutowallSettings() {
    ImGui::Text("Autowall");
    ImGui::Checkbox("Enabled", &m_aimConfig.autowall.enabled);
    ImGui::DragFloat("Max Penetration", &m_aimConfig.autowall.maxPenetration, 0.1f, 0.0f, 5.0f);
    ImGui::DragFloat("Penetration Power", &m_aimConfig.autowall.penetrationPower, 0.1f, 0.0f, 2.0f);
}

void RageBotTab::RenderPreferBodyAim() {
    ImGui::Text("Prefer Body Aim");
    ImGui::Checkbox("Lethal", &m_aimConfig.preferBodyAim.lethal);
    ImGui::DragInt("HP Threshold", &m_aimConfig.preferBodyAim.hpThreshold, 1, 0, 100);
    ImGui::Checkbox("In Air", &m_aimConfig.preferBodyAim.inAir);
    // On Key would use Keybind widget
}

void RageBotTab::RenderDoubleTap() {
    ImGui::Text("Double Tap");
    ImGui::Checkbox("Enabled", &m_exploitConfig.doubleTap.enabled);
    
    const char* modes[] = {"Defensive", "Offensive"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_exploitConfig.doubleTap.mode), modes, 2);
    ImGui::Checkbox("Quick Stop", &m_exploitConfig.doubleTap.quickStop);
    ImGui::DragFloat("Recharge Time", &m_exploitConfig.doubleTap.rechargeTime, 0.01f, 0.0f, 1.0f);
}

void RageBotTab::RenderHideShots() {
    ImGui::Text("Hide Shots");
    ImGui::Checkbox("Enabled", &m_exploitConfig.hideShots.enabled);
    
    const char* modes[] = {"Standard", "Subticks"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_exploitConfig.hideShots.mode), modes, 2);
    ImGui::Checkbox("Quick Stop", &m_exploitConfig.hideShots.quickStop);
}

void RageBotTab::RenderFakeLag() {
    ImGui::Text("Fake Lag");
    ImGui::Checkbox("Enabled", &m_exploitConfig.fakeLag.enabled);
    
    const char* modes[] = {"Static", "Fluctuate", "Adaptive"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_exploitConfig.fakeLag.mode), modes, 3);
    ImGui::DragInt("Limit", &m_exploitConfig.fakeLag.limit, 1, 1, 16);
    ImGui::DragFloat("Variance", &m_exploitConfig.fakeLag.variance, 0.01f, 0.0f, 1.0f);
}

void RageBotTab::RenderQuickStop() {
    ImGui::Text("Quick Stop");
    
    const char* modes[] = {"Early", "Normal", "Late"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_exploitConfig.quickStop.mode), modes, 3);
}

void RageBotTab::RenderQuickPeek() {
    ImGui::Text("Quick Peek");
    ImGui::Checkbox("Enabled", &m_exploitConfig.quickPeek.enabled);
    // Keybind widget
    ImGui::Checkbox("Render Path", &m_exploitConfig.quickPeek.renderPath);
}

void RageBotTab::RenderPitchSettings() {
    ImGui::Text("Pitch");
    const char* pitches[] = {"Off", "Down", "Up", "Zero", "Jitter", "Custom"};
    ImGui::Combo("Pitch", reinterpret_cast<int*>(&m_aaConfig.pitch), pitches, 6);
    
    if (m_aaConfig.pitch == AntiAimConfig::Pitch::Custom) {
        ImGui::DragFloat("Custom Pitch", &m_aaConfig.pitchCustom, 0.1f, -89.0f, 89.0f);
    }
}

void RageBotTab::RenderYawSettings() {
    ImGui::Text("Yaw");
    const char* yaws[] = {"Off", "Backward", "Jitter", "Spin", "Static", "Custom"};
    ImGui::Combo("Yaw", reinterpret_cast<int*>(&m_aaConfig.yaw), yaws, 6);
    
    if (m_aaConfig.yaw == AntiAimConfig::Yaw::Static) {
        ImGui::DragFloat("Static Yaw", &m_aaConfig.yawStatic, 0.1f, -180.0f, 180.0f);
    } else if (m_aaConfig.yaw == AntiAimConfig::Yaw::Jitter) {
        ImGui::DragFloat("Jitter Range", &m_aaConfig.yawJitterRange, 0.1f, 0.0f, 180.0f);
    } else if (m_aaConfig.yaw == AntiAimConfig::Yaw::Spin) {
        ImGui::DragFloat("Spin Speed", &m_aaConfig.yawSpinSpeed, 0.1f, 0.0f, 50.0f);
    }
}

void RageBotTab::RenderDesyncSettings() {
    ImGui::Text("Desync");
    const char* desyncs[] = {"Off", "Static", "Jitter", "Avoid Overlap", "Custom"};
    ImGui::Combo("Desync", reinterpret_cast<int*>(&m_aaConfig.desync), desyncs, 5);
    
    if (m_aaConfig.desync == AntiAimConfig::Desync::Static) {
        ImGui::DragFloat("Static Desync", &m_aaConfig.desyncStatic, 0.1f, 0.0f, 60.0f);
    } else if (m_aaConfig.desync == AntiAimConfig::Desync::Jitter) {
        ImGui::DragFloat("Jitter Range", &m_aaConfig.desyncJitterRange, 0.1f, 0.0f, 60.0f);
    }
}

void RageBotTab::RenderLBYBreaker() {
    ImGui::Text("LBY Breaker");
    ImGui::Checkbox("Enabled", &m_aaConfig.lbyBreaker);
    
    const char* modes[] = {"Normal", "Extended"};
    ImGui::Combo("Mode", reinterpret_cast<int*>(&m_aaConfig.lbyBreakerMode), modes, 2);
}

void RageBotTab::RenderFreestanding() {
    ImGui::Text("Freestanding");
    ImGui::Checkbox("Enabled", &m_aaConfig.freestanding);
    // Keybind widget
    ImGui::DragFloat("Edge Distance", &m_aaConfig.freestandingEdgeDistance, 1.0f, 0.0f, 200.0f);
}

void RageBotTab::RenderManualAA() {
    ImGui::Text("Manual AA");
    // Keybind widgets for left/right/back
    ImGui::Checkbox("Indicator", &m_aaConfig.manualAA.indicator);
}

void RageBotTab::RenderResolverSettings() {
    ImGui::Text("Resolver");
    // Uses m_resolverConfig
}

void RageBotTab::RenderAnimFixSettings() {
    ImGui::Text("Animation Fix");
    // Would use AnimFix config
}

} // namespace gui