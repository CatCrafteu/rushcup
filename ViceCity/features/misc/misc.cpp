#include "core/pch.hpp"
#include "features/misc/misc.hpp"

#include "features/lua/lua_manager.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "skinchanger_system/case_opening.hpp"
#include "skinchanger_system/sticker_tool.hpp"
#include "skinchanger_system/inspect_panel.hpp"
#include "skinchanger_system/pattern_seed.hpp"
#include "gui/vice_render.hpp"
#include "gui/vice_fonts.hpp"

namespace features::misc {

// ========== MovementEx ==========

void MovementEx::Initialize() {
    LoadConfig();
    LOG_INFO(Misc, "MovementEx initialized");
}

void MovementEx::Shutdown() {
    SaveConfig();
    LOG_INFO(Misc, "MovementEx shutdown");
}

void MovementEx::OnCreateMove(sdk::CUserCmd* cmd) {
    DoAutoPeek(cmd);
    DoThirdperson();
    DoFOVOverride();
    DoViewmodelChanger();
    DoAspectRatio();
}

void MovementEx::OnFrameStageNotify(int stage) {
    // Handle thirdperson view
}

void MovementEx::Render() {
    RenderAutoPeek();
    RenderThirdperson();
}

void MovementEx::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("misc_movement", key, def);
        if (opt) value = *opt;
    };
    
    load("auto_peek.enabled", m_config.autoPeek.enabled, false);
    load("auto_peek.keybind", m_config.autoPeek.keybind, std::string(""));
    load("auto_peek.render", m_config.autoPeek.render, true);
    load("thirdperson.enabled", m_config.thirdperson.enabled, false);
    load("thirdperson.keybind", m_config.thirdperson.keybind, std::string(""));
    load("thirdperson.distance", m_config.thirdperson.distance, 150.0f);
    load("thirdperson.collision", m_config.thirdperson.collision, true);
    load("fov_override.enabled", m_config.fovOverride.enabled, false);
    load("fov_override.value", m_config.fovOverride.value, 90.0f);
    load("viewmodel_changer.enabled", m_config.viewmodelChanger.enabled, false);
    load("viewmodel_changer.x", m_config.viewmodelChanger.x, 0.0f);
    load("viewmodel_changer.y", m_config.viewmodelChanger.y, 0.0f);
    load("viewmodel_changer.z", m_config.viewmodelChanger.z, 0.0f);
    load("viewmodel_changer.fov", m_config.viewmodelChanger.fov, 68.0f);
    load("aspect_ratio.enabled", m_config.aspectRatio.enabled, false);
    load("aspect_ratio.value", m_config.aspectRatio.value, 1.33f);
    load("recoil_crosshair.enabled", m_config.recoilCrosshair.enabled, true);
    load("penetration_crosshair.enabled", m_config.penetrationCrosshair.enabled, false);
}

void MovementEx::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("misc_movement", key, value);
    };
    
    save("auto_peek.enabled", m_config.autoPeek.enabled);
    save("auto_peek.keybind", m_config.autoPeek.keybind);
    save("auto_peek.render", m_config.autoPeek.render);
    save("thirdperson.enabled", m_config.thirdperson.enabled);
    save("thirdperson.keybind", m_config.thirdperson.keybind);
    save("thirdperson.distance", m_config.thirdperson.distance);
    save("thirdperson.collision", m_config.thirdperson.collision);
    save("fov_override.enabled", m_config.fovOverride.enabled);
    save("fov_override.value", m_config.fovOverride.value);
    save("viewmodel_changer.enabled", m_config.viewmodelChanger.enabled);
    save("viewmodel_changer.x", m_config.viewmodelChanger.x);
    save("viewmodel_changer.y", m_config.viewmodelChanger.y);
    save("viewmodel_changer.z", m_config.viewmodelChanger.z);
    save("viewmodel_changer.fov", m_config.viewmodelChanger.fov);
    save("aspect_ratio.enabled", m_config.aspectRatio.enabled);
    save("aspect_ratio.value", m_config.aspectRatio.value);
    save("recoil_crosshair.enabled", m_config.recoilCrosshair.enabled);
    save("penetration_crosshair.enabled", m_config.penetrationCrosshair.enabled);
}

void MovementEx::DoAutoPeek(sdk::CUserCmd* cmd) {
    if (!m_config.autoPeek.enabled) return;
    
    bool keyPressed = false; // Check keybind
    
    if (keyPressed) {
        if (!m_wasAutoPeeking) {
            auto local = GetLocalPlayer();
            if (local) m_autoPeekStartPos = {}; // local->GetAbsOrigin();
            m_wasAutoPeeking = true;
        }
    } else if (m_wasAutoPeeking) {
        // Return to start position
        m_wasAutoPeeking = false;
    }
}

void MovementEx::DoThirdperson() {
    if (!m_config.thirdperson.enabled) return;
    
    bool keyPressed = false; // Check keybind
    
    static bool wasPressed = false;
    if (keyPressed && !wasPressed) {
        m_config.thirdperson.enabled = !m_config.thirdperson.enabled;
    }
    wasPressed = keyPressed;
}

void MovementEx::DoFOVOverride() {
    if (!m_config.fovOverride.enabled) return;
    
    // Would override FOV via clientmode
}

void MovementEx::DoViewmodelChanger() {
    if (!m_config.viewmodelChanger.enabled) return;
    
    // Would modify viewmodel cvars
}

void MovementEx::DoAspectRatio() {
    if (!m_config.aspectRatio.enabled) return;
    
    // Would set r_aspectratio cvar
}

void MovementEx::RenderAutoPeek() {
    if (!m_config.autoPeek.enabled || !m_config.autoPeek.render || !m_wasAutoPeeking) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Draw circle at start position
    ImVec2 screen;
    if (gui::ViceRender::Instance().WorldToScreen(m_autoPeekStartPos, screen)) {
        drawList->AddCircle(screen, 32.0f, IM_COL32(255, 95, 155, 255), 32, 2.0f);
    }
}

void MovementEx::RenderThirdperson() {
    // Thirdperson rendering handled by hooks
}

bool MovementEx::IsKeyPressed(const std::string& keybind) {
    if (keybind.empty()) return false;
    int key = 0; // Parse keybind
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

sdk::CBaseEntity* MovementEx::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

// ========== Logs ==========

void Logs::Initialize() {
    LoadConfig();
    LOG_INFO(Misc, "Logs initialized");
}

void Logs::Shutdown() {
    SaveConfig();
    LOG_INFO(Misc, "Logs shutdown");
}

void Logs::Render() {
    if (!m_config.hitLogs && !m_config.damageLogs && !m_config.purchaseLogs && !m_config.consoleLogs) return;
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    
    if (ImGui::Begin("Logs", nullptr, flags)) {
        for (size_t i = 0; i < m_logs.size(); ++i) {
            RenderLog(m_logs[i], static_cast<float>(i) * 20.0f);
        }
    }
    ImGui::End();
}

void Logs::OnFireEvent(void* event) {
    if (!m_config.hitLogs && !m_config.damageLogs && !m_config.purchaseLogs) return;
    ProcessEvent(event);
}

void Logs::OnDispatchSound(void* sound) {
    // Sound ESP logging
}

void Logs::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("misc_logs", key, def);
        if (opt) value = *opt;
    };
    
    load("hit_logs", m_config.hitLogs, true);
    load("damage_logs", m_config.damageLogs, true);
    load("purchase_logs", m_config.purchaseLogs, true);
    load("console_logs", m_config.consoleLogs, false);
    load("filter_local", m_config.filterLocal, true);
    load("filter_teammates", m_config.filterTeammates, false);
    load("filter_enemies", m_config.filterEnemies, true);
}

void Logs::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("misc_logs", key, value);
    };
    
    save("hit_logs", m_config.hitLogs);
    save("damage_logs", m_config.damageLogs);
    save("purchase_logs", m_config.purchaseLogs);
    save("console_logs", m_config.consoleLogs);
    save("filter_local", m_config.filterLocal);
    save("filter_teammates", m_config.filterTeammates);
    save("filter_enemies", m_config.filterEnemies);
}

void Logs::AddLog(LogEntry::Type type, const std::string& msg, const sdk::Color& color) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    LogEntry entry;
    entry.type = type;
    entry.message = msg;
    entry.color = color;
    entry.time = 0.0f;
    entry.duration = 5.0f;
    
    m_logs.push_front(entry);
    if (m_logs.size() > MAX_LOGS) {
        m_logs.pop_back();
    }
}

void Logs::ProcessEvent(void* event) {
    // Parse game events and add logs
}

std::string Logs::GetHitgroupName(int hitgroup) {
    switch (hitgroup) {
        case 1: return "Head";
        case 2: return "Chest";
        case 3: return "Stomach";
        case 4: return "Left Arm";
        case 5: return "Right Arm";
        case 6: return "Left Leg";
        case 7: return "Right Leg";
        case 8: return "Neck";
        default: return "Generic";
    }
}

std::string Logs::GetWeaponName(int itemDefIndex) {
    return "Weapon";
}

sdk::Color Logs::GetLogColor(LogEntry::Type type) {
    switch (type) {
        case LogEntry::Type::Hit: return sdk::Color{0, 255, 0, 255};
        case LogEntry::Type::Damage: return sdk::Color{255, 150, 0, 255};
        case LogEntry::Type::Purchase: return sdk::Color{0, 200, 255, 255};
        case LogEntry::Type::Console: return sdk::Color{200, 200, 200, 255};
    }
    return sdk::Color{255, 255, 255, 255};
}

void Logs::RenderLog(const LogEntry& log, float y) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.y += y;
    
    drawList->AddText(pos, log.color.ToU32(), log.message.c_str());
}

// ========== Skinchanger ==========

void Skinchanger::Initialize() {
    LoadConfig();
    LOG_INFO(::Skinchanger, "Skinchanger initialized");
}

void Skinchanger::Shutdown() {
    SaveConfig();
    LOG_INFO(::Skinchanger, "Skinchanger shutdown");
}

void Skinchanger::OnFrameStageNotify(int stage) {
    if (stage == 0) { // FRAME_NET_UPDATE_POSTDATAUPDATE_START
        if (m_config.autoApply) {
            ApplyAllSkins();
        }
    }
}

void Skinchanger::OnPostDataUpdate(int updateType) {
    // Update weapon skins after data update
}

void Skinchanger::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("misc_skinchanger", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, true);
    load("auto_apply", m_config.autoApply, true);
    load("show_in_inventory", m_config.showInInventory, true);
}

void Skinchanger::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("misc_skinchanger", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("auto_apply", m_config.autoApply);
    save("show_in_inventory", m_config.showInInventory);
}

void Skinchanger::ApplySkin(sdk::CBaseWeapon* weapon, const WeaponSkin& skin) {
    if (!weapon) return;
    
    // Apply skin data to weapon entity
    // weapon->SetFallbackPaintKit(skin.paintKit);
    // weapon->SetFallbackWear(skin.wear);
    // weapon->SetFallbackSeed(skin.seed);
    // weapon->SetFallbackStatTrak(skin.statTrak);
    // weapon->SetCustomName(skin.customName);
    // Apply stickers...
}

std::optional<Skinchanger::WeaponSkin> Skinchanger::GetSkinForWeapon(int itemDefIndex) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_skins.find(itemDefIndex);
    if (it == m_skins.end()) return std::nullopt;
    return it->second;
}

void Skinchanger::SetSkinForWeapon(int itemDefIndex, const WeaponSkin& skin) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_skins[itemDefIndex] = skin;
    SaveConfig();
}

void Skinchanger::RemoveSkinForWeapon(int itemDefIndex) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_skins.erase(itemDefIndex);
    SaveConfig();
}

void Skinchanger::ForceKnifeModel(sdk::CBaseEntity* viewModel, int knifeModel) {
    // Force knife model on viewmodel
}

void Skinchanger::ForceGloveModel(sdk::CBaseEntity* viewModel, int gloveModel) {
    // Force glove model on viewmodel
}

void Skinchanger::UpdateStatTrak(sdk::CBaseWeapon* weapon, int kills) {
    // Update StatTrak counter
}

void Skinchanger::ApplyAllSkins() {
    // Apply all configured skins to weapons
}

int Skinchanger::GetKnifeModel(int itemDefIndex) {
    switch (itemDefIndex) {
        case 500: return 1; // Bayonet
        case 503: return 2; // Classic Knife
        case 505: return 3; // Flip Knife
        case 506: return 4; // Gut Knife
        case 507: return 5; // Karambit
        case 508: return 6; // M9 Bayonet
        case 509: return 7; // Huntsman Knife
        case 512: return 8; // Falchion Knife
        case 514: return 9; // Bowie Knife
        case 515: return 10; // Butterfly Knife
        case 516: return 11; // Shadow Daggers
        case 517: return 12; // Paracord Knife
        case 518: return 13; // Survival Knife
        case 519: return 14; // Ursus Knife
        case 520: return 15; // Navaja Knife
        case 521: return 16; // Nomad Knife
        case 522: return 17; // Stiletto Knife
        case 523: return 18; // Talon Knife
        case 525: return 19; // Skeleton Knife
        default: return 0;
    }
}

int Skinchanger::GetGloveModel(int itemDefIndex) {
    switch (itemDefIndex) {
        case 5027: return 1; // Bloodhound
        case 5028: return 2; // T-Side
        case 5029: return 3; // CT-Side
        case 5030: return 4; // Sporty
        case 5031: return 5; // Slick
        case 5032: return 6; // Leather Wrap
        case 5033: return 7; // Motorcycle
        case 5034: return 8; // Specialist
        case 5035: return 9; // Hydra
        default: return 0;
    }
}

void Skinchanger::UpdateWeaponNetworkable(sdk::CBaseWeapon* weapon) {
    // Force weapon to re-network
}

// ========== InventoryUI ==========

void InventoryUI::Initialize() {
    LoadConfig();
    LOG_INFO(::Skinchanger, "InventoryUI initialized");
}

void InventoryUI::Shutdown() {
    SaveConfig();
    LOG_INFO(::Skinchanger, "InventoryUI shutdown");
}

void InventoryUI::Render() {
    if (m_caseOpening) RenderCaseOpening();
    RenderStickerTool();
    RenderInspectPanel();
    RenderPatternSeedBrowser();
}

void InventoryUI::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("misc_inventory_ui", key, def);
        if (opt) value = *opt;
    };
    
    load("show_case_opening", m_config.showCaseOpening, true);
    load("show_sticker_tool", m_config.showStickerTool, true);
    load("show_inspect_panel", m_config.showInspectPanel, true);
    load("show_pattern_seed_browser", m_config.showPatternSeedBrowser, true);
}

void InventoryUI::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("misc_inventory_ui", key, value);
    };
    
    save("show_case_opening", m_config.showCaseOpening);
    save("show_sticker_tool", m_config.showStickerTool);
    save("show_inspect_panel", m_config.showInspectPanel);
    save("show_pattern_seed_browser", m_config.showPatternSeedBrowser);
}

void InventoryUI::OpenCase(int caseId, int keyId) {
    auto& caseOpening = skinchanger::CaseOpening::Instance();
    if (caseOpening.StartOpening(caseId, keyId)) {
        m_caseOpening = true;
        m_caseId = caseId;
        m_keyId = keyId;
    }
}

void InventoryUI::RenderCaseOpening() {
    if (!m_caseOpening) return;
    
    skinchanger::CaseOpening::Instance().Render();
}

void InventoryUI::RenderStickerTool() {
    if (!m_config.showStickerTool) return;
    
    skinchanger::StickerTool::Instance().Render();
}

void InventoryUI::RenderInspectPanel() {
    if (!m_config.showInspectPanel) return;
    
    skinchanger::InspectPanel::Instance().Render();
}

void InventoryUI::RenderPatternSeedBrowser() {
    if (!m_config.showPatternSeedBrowser) return;
    
    skinchanger::PatternSeed::Instance().Render();
}

} // namespace features::misc