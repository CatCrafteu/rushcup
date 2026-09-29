#include "core/pch.hpp"
#include "gui/tabs/visuals_tab.hpp"
#include "features/visuals/esp.hpp"
#include "features/visuals/chams.hpp"
#include "features/visuals/world.hpp"
#include "features/visuals/radar.hpp"
#include "features/visuals/effects.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"

namespace gui {

void VisualsTab::Initialize() {
    LOG_INFO(GUI, "VisualsTab initialized");
}

void VisualsTab::Shutdown() {
    LOG_INFO(GUI, "VisualsTab shutdown");
}

void VisualsTab::Render() {
    ImGui::BeginChild("##visuals_subtabs", ImVec2(0, 30), false);
    
    const char* subTabs[] = {"ESP", "Chams", "World", "Radar", "Effects"};
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
        case SubTab::ESP: RenderESPTab(); break;
        case SubTab::Chams: RenderChamsTab(); break;
        case SubTab::World: RenderWorldTab(); break;
        case SubTab::Radar: RenderRadarTab(); break;
        case SubTab::Effects: RenderEffectsTab(); break;
    }
}

void VisualsTab::RenderESPTab() {
    ImGui::BeginChild("##esp_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_espConfig.enabled);
    ImGui::Separator();
    
    RenderESPBoxSettings();
    ImGui::Separator();
    
    RenderESPSkeletonSettings();
    ImGui::Separator();
    
    RenderESPHealthSettings();
    ImGui::Separator();
    
    RenderESPArmorSettings();
    ImGui::Separator();
    
    RenderESPAmmoSettings();
    ImGui::Separator();
    
    RenderESPNameSettings();
    ImGui::Separator();
    
    RenderESPWeaponSettings();
    ImGui::Separator();
    
    RenderESPFlagsSettings();
    ImGui::Separator();
    
    RenderESPDormantSettings();
    ImGui::Separator();
    
    RenderESPSnaplinesSettings();
    ImGui::Separator();
    
    RenderESPOffscreenArrowsSettings();
    
    ImGui::EndChild();
}

void VisualsTab::RenderChamsTab() {
    ImGui::BeginChild("##chams_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_chamsConfig.enabled);
    ImGui::Separator();
    
    RenderChamsPlayerSettings();
    ImGui::Separator();
    
    RenderChamsArmsSettings();
    ImGui::Separator();
    
    RenderChamsWeaponsSettings();
    ImGui::Separator();
    
    RenderChamsAttachmentsSettings();
    
    ImGui::EndChild();
}

void VisualsTab::RenderWorldTab() {
    ImGui::BeginChild("##world_tab", ImVec2(0, 0), false);
    
    RenderNightmodeSettings();
    ImGui::Separator();
    
    RenderPropTransparencySettings();
    ImGui::Separator();
    
    RenderGrenadePreviewSettings();
    ImGui::Separator();
    
    RenderHitmarkerSettings();
    ImGui::Separator();
    
    RenderDamageIndicatorSettings();
    ImGui::Separator();
    
    RenderSpreadCircleSettings();
    
    ImGui::EndChild();
}

void VisualsTab::RenderRadarTab() {
    ImGui::BeginChild("##radar_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_radarConfig.enabled);
    ImGui::DragFloat("Range", &m_radarConfig.range, 10.0f, 500.0f, 5000.0f);
    ImGui::DragFloat("Size", &m_radarConfig.size, 1.0f, 100.0f, 500.0f);
    ImGui::DragFloat2("Position", &m_radarConfig.position.x);
    ImGui::Checkbox("Show Teammates", &m_radarConfig.showTeammates);
    ImGui::Checkbox("Show Enemies", &m_radarConfig.showEnemies);
    ImGui::Checkbox("Show Bomb", &m_radarConfig.showBomb);
    ImGui::Checkbox("Custom Icons", &m_radarConfig.customIcons);
    
    ImGui::EndChild();
}

void VisualsTab::RenderEffectsTab() {
    ImGui::BeginChild("##effects_tab", ImVec2(0, 0), false);
    
    RenderBulletTracersSettings();
    ImGui::Separator();
    
    RenderImpactBeamsSettings();
    ImGui::Separator();
    
    RenderBulletBeamsSettings();
    ImGui::Separator();
    
    RenderHitNumbersSettings();
    
    ImGui::EndChild();
}

// ESP Helpers
void VisualsTab::RenderESPBoxSettings() {
    ImGui::Text("Box");
    ImGui::Checkbox("Enabled", &m_espConfig.box.enabled);
    
    const char* types[] = {"2D", "3D", "Corner"};
    ImGui::Combo("Type", reinterpret_cast<int*>(&m_espConfig.box.type), types, 3);
    
    // Color pickers
    // ViceWidgets::ColorPicker("Color##enemy", m_espConfig.box.color);
    // ViceWidgets::ColorPicker("Color##team", m_espConfig.box.colorTeammate);
}

void VisualsTab::RenderESPSkeletonSettings() {
    ImGui::Text("Skeleton");
    ImGui::Checkbox("Enabled", &m_espConfig.skeleton.enabled);
    // Color picker
}

void VisualsTab::RenderESPHealthSettings() {
    ImGui::Text("Health Bar");
    ImGui::Checkbox("Enabled", &m_espConfig.healthBar.enabled);
    // Color pickers for high/low
    ImGui::Separator();
    
    ImGui::Text("Health Text");
    ImGui::Checkbox("Enabled", &m_espConfig.healthText.enabled);
    // Color picker
}

void VisualsTab::RenderESPArmorSettings() {
    ImGui::Text("Armor");
    ImGui::Checkbox("Enabled", &m_espConfig.armor.enabled);
    // Color picker
}

void VisualsTab::RenderESPAmmoSettings() {
    ImGui::Text("Ammo");
    ImGui::Checkbox("Enabled", &m_espConfig.ammo.enabled);
    // Color picker
}

void VisualsTab::RenderESPNameSettings() {
    ImGui::Text("Name");
    ImGui::Checkbox("Enabled", &m_espConfig.name.enabled);
    // Color picker
}

void VisualsTab::RenderESPWeaponSettings() {
    ImGui::Text("Weapon");
    ImGui::Checkbox("Enabled", &m_espConfig.weapon.enabled);
    
    const char* types[] = {"Text", "Icon", "Ammo"};
    ImGui::Combo("Type", reinterpret_cast<int*>(&m_espConfig.weapon.type), types, 3);
    // Color picker
}

void VisualsTab::RenderESPFlagsSettings() {
    ImGui::Text("Flags");
    ImGui::Checkbox("Enabled", &m_espConfig.flags.enabled);
    ImGui::Checkbox("Money", &m_espConfig.flags.showMoney);
    ImGui::Checkbox("Armor", &m_espConfig.flags.showArmor);
    ImGui::Checkbox("Kit", &m_espConfig.flags.showKit);
    ImGui::Checkbox("Scoped", &m_espConfig.flags.showScoped);
    ImGui::Checkbox("Flashed", &m_espConfig.flags.showFlashed);
    ImGui::Checkbox("Defusing", &m_espConfig.flags.showDefusing);
    // Color picker
}

void VisualsTab::RenderESPDormantSettings() {
    ImGui::Text("Dormant");
    ImGui::Checkbox("Enabled", &m_espConfig.dormant.enabled);
    // Color picker
}

void VisualsTab::RenderESPSnaplinesSettings() {
    ImGui::Text("Snaplines");
    ImGui::Checkbox("Enabled", &m_espConfig.snaplines.enabled);
    
    const char* types[] = {"Bottom", "Center", "Top"};
    ImGui::Combo("Type", reinterpret_cast<int*>(&m_espConfig.snaplines.type), types, 3);
    // Color picker
}

void VisualsTab::RenderESPOffscreenArrowsSettings() {
    ImGui::Text("Offscreen Arrows");
    ImGui::Checkbox("Enabled", &m_espConfig.offscreenArrows.enabled);
    ImGui::DragFloat("Size", &m_espConfig.offscreenArrows.size, 0.1f, 4.0f, 30.0f);
    ImGui::DragFloat("Distance", &m_espConfig.offscreenArrows.distance, 1.0f, 50.0f, 500.0f);
    // Color picker
}

// Chams Helpers
void VisualsTab::RenderChamsPlayerSettings() {
    ImGui::Text("Player");
    
    // Visible
    ImGui::SeparatorText("Visible");
    ImGui::Checkbox("Enabled", &m_chamsConfig.player.visible.enabled);
    const char* mats[] = {"Flat", "Glass", "Plastic", "Metallic", "Glow", "Wireframe", "Pulse"};
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.player.visible.material), mats, 7);
    // Color pickers
    
    // Invisible
    ImGui::SeparatorText("Invisible");
    ImGui::Checkbox("Enabled", &m_chamsConfig.player.invisible.enabled);
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.player.invisible.material), mats, 7);
    // Color pickers
    
    // History
    ImGui::SeparatorText("History");
    ImGui::Checkbox("Enabled", &m_chamsConfig.player.history.enabled);
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.player.history.material), mats, 7);
    // Color picker
    ImGui::DragFloat("Duration", &m_chamsConfig.player.history.duration, 0.1f, 0.1f, 5.0f);
    
    // Shot
    ImGui::SeparatorText("Shot");
    ImGui::Checkbox("Enabled", &m_chamsConfig.player.shot.enabled);
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.player.shot.material), mats, 7);
    // Color picker
    ImGui::DragFloat("Duration", &m_chamsConfig.player.shot.duration, 0.1f, 0.1f, 2.0f);
}

void VisualsTab::RenderChamsArmsSettings() {
    ImGui::Text("Arms (Viewmodel)");
    ImGui::Checkbox("Enabled", &m_chamsConfig.arms.enabled);
    const char* mats[] = {"Flat", "Glass", "Plastic", "Metallic", "Glow", "Wireframe", "Pulse"};
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.arms.material), mats, 7);
    // Color picker
}

void VisualsTab::RenderChamsWeaponsSettings() {
    ImGui::Text("Weapons");
    ImGui::Checkbox("Enabled", &m_chamsConfig.weapons.enabled);
    const char* mats[] = {"Flat", "Glass", "Plastic", "Metallic", "Glow", "Wireframe", "Pulse"};
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.weapons.material), mats, 7);
    // Color picker
}

void VisualsTab::RenderChamsAttachmentsSettings() {
    ImGui::Text("Attachments");
    ImGui::Checkbox("Enabled", &m_chamsConfig.attachments.enabled);
    const char* mats[] = {"Flat", "Glass", "Plastic", "Metallic", "Glow", "Wireframe", "Pulse"};
    ImGui::Combo("Material", reinterpret_cast<int*>(&m_chamsConfig.attachments.material), mats, 7);
    // Color picker
}

// World Helpers
void VisualsTab::RenderNightmodeSettings() {
    ImGui::Text("Nightmode");
    ImGui::Checkbox("Enabled", &m_worldConfig.nightmode.enabled);
    // Color picker
}

void VisualsTab::RenderPropTransparencySettings() {
    ImGui::Text("Prop Transparency");
    ImGui::Checkbox("Enabled", &m_worldConfig.propTransparency.enabled);
    ImGui::DragFloat("Amount", &m_worldConfig.propTransparency.amount, 0.01f, 0.0f, 1.0f);
}

void VisualsTab::RenderGrenadePreviewSettings() {
    ImGui::Text("Grenade Preview");
    ImGui::Checkbox("Enabled", &m_worldConfig.grenadePreview.enabled);
    ImGui::Checkbox("Trajectory", &m_worldConfig.grenadePreview.trajectory);
    ImGui::Checkbox("Bounce", &m_worldConfig.grenadePreview.bounce);
    // Color picker
}

void VisualsTab::RenderHitmarkerSettings() {
    ImGui::Text("Hitmarker");
    ImGui::Checkbox("Enabled", &m_worldConfig.hitmarker.enabled);
    
    const char* types[] = {"2D", "3D", "Sound"};
    ImGui::Combo("Type", reinterpret_cast<int*>(&m_worldConfig.hitmarker.type), types, 3);
    // Color picker
    ImGui::DragFloat("Duration", &m_worldConfig.hitmarker.duration, 0.1f, 0.1f, 2.0f);
}

void VisualsTab::RenderDamageIndicatorSettings() {
    ImGui::Text("Damage Indicator");
    ImGui::Checkbox("Enabled", &m_worldConfig.damageIndicator.enabled);
    // Color picker
}

void VisualsTab::RenderSpreadCircleSettings() {
    ImGui::Text("Spread Circle");
    ImGui::Checkbox("Enabled", &m_worldConfig.spreadCircle.enabled);
    // Color picker
}

// Effects Helpers
void VisualsTab::RenderBulletTracersSettings() {
    ImGui::Text("Bullet Tracers");
    ImGui::Checkbox("Enabled", &m_effectsConfig.bulletTracers.enabled);
    
    const char* types[] = {"Line", "Beam", "Particle"};
    ImGui::Combo("Type", reinterpret_cast<int*>(&m_effectsConfig.bulletTracers.type), types, 3);
    // Color picker
    ImGui::DragFloat("Duration", &m_effectsConfig.bulletTracers.duration, 0.1f, 0.1f, 5.0f);
}

void VisualsTab::RenderImpactBeamsSettings() {
    ImGui::Text("Impact Beams");
    ImGui::Checkbox("Enabled", &m_effectsConfig.impactBeams.enabled);
    // Color picker
    ImGui::DragFloat("Duration", &m_effectsConfig.impactBeams.duration, 0.1f, 0.1f, 5.0f);
}

void VisualsTab::RenderBulletBeamsSettings() {
    ImGui::Text("Bullet Beams");
    ImGui::Checkbox("Enabled", &m_effectsConfig.bulletBeams.enabled);
    // Color picker
}

void VisualsTab::RenderHitNumbersSettings() {
    ImGui::Text("Hit Numbers");
    ImGui::Checkbox("Enabled", &m_effectsConfig.hitNumbers.enabled);
    // Color picker
    ImGui::DragFloat("Font Size", &m_effectsConfig.hitNumbers.fontSize, 0.5f, 8.0f, 24.0f);
}

} // namespace gui