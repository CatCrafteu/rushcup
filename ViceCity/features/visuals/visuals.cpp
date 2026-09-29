#include "core/pch.hpp"
#include "features/visuals/esp.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"
#include "gui/vice_render.hpp"
#include "gui/vice_fonts.hpp"

namespace features::visuals {

// ========== ESP ==========

void ESP::Initialize() {
    LoadConfig();
    LOG_INFO(Visuals, "ESP initialized");
}

void ESP::Shutdown() {
    SaveConfig();
    LOG_INFO(Visuals, "ESP shutdown");
}

void ESP::Render() {
    if (!m_config.enabled) return;
    
    auto players = GetPlayers();
    for (auto& player : players) {
        RenderPlayer(player);
    }
}

void ESP::OnFrameStageNotify(int stage) {
    if (stage == 0) { // FRAME_NET_UPDATE_POSTDATAUPDATE_START
        UpdatePlayers();
    }
}

std::vector<ESP::PlayerESPData> ESP::GetPlayers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_players;
}

void ESP::UpdatePlayers() {
    std::vector<PlayerESPData> newPlayers;
    
    int maxClients = sdk::g_pEngine->GetMaxClients();
    auto local = GetLocalPlayer();
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(i));
        if (!entity) continue;
        
        PlayerESPData data;
        CollectPlayerData(entity, data);
        
        if (data.isLocalPlayer) continue;
        
        CalculateBox(data);
        newPlayers.push_back(data);
    }
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_players = std::move(newPlayers);
    }
}

void ESP::CollectPlayerData(sdk::CBaseEntity* entity, PlayerESPData& data) {
    data.entity = entity;
    data.origin = {}; // entity->GetAbsOrigin()
    data.angles = {}; // entity->GetAbsAngles()
    data.health = 100; // entity->GetHealth()
    data.armor = 100; // entity->GetArmor()
    data.maxHealth = 100;
    data.name = "Player"; // entity->GetPlayerName()
    data.weaponName = "weapon"; // entity->GetActiveWeapon()->GetWeaponName()
    data.ammo = 30;
    data.maxAmmo = 90;
    data.isDormant = false;
    data.isTeammate = false;
    data.isLocalPlayer = false;
    data.hasDefuser = false;
    data.isScoped = false;
    data.isFlashed = false;
    data.isDefusing = false;
    data.money = 0;
    
    // Get bone matrix
    // entity->SetupBones(data.boneMatrix.data(), 128, 0x100, 0.0f);
    data.hasBoneMatrix = true;
}

bool ESP::WorldToScreen(const sdk::Vector3D& world, sdk::Vector2D& screen) {
    ImVec2 imScreen;
    bool result = gui::ViceRender::Instance().WorldToScreen(world, imScreen);
    screen.x = imScreen.x;
    screen.y = imScreen.y;
    return result;
}

void ESP::CalculateBox(const PlayerESPData& data) {
    // Calculate 2D bounding box from 3D bounds
    // Simplified
}

void ESP::RenderPlayer(PlayerESPData& data) {
    if (!WorldToScreen(data.origin, data.screenPos)) return;
    
    sdk::Color color = GetPlayerColor(data, m_config.box.color, m_config.box.colorTeammate);
    
    DrawBox(data);
    DrawSkeleton(data);
    DrawHealthBar(data);
    DrawHealthText(data);
    DrawArmor(data);
    DrawAmmo(data);
    DrawName(data);
    DrawWeapon(data);
    DrawFlags(data);
    DrawSnapline(data);
    DrawOffscreenArrow(data);
    DrawDormant(data);
}

sdk::Color ESP::GetPlayerColor(const PlayerESPData& data, const sdk::Color& enemyColor, const sdk::Color& teamColor) const {
    return data.isTeammate ? teamColor : enemyColor;
}

sdk::Color ESP::LerpColor(const sdk::Color& a, const sdk::Color& b, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    return sdk::Color(
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t)
    );
}

void ESP::DrawBox(const PlayerESPData& data) {
    if (!m_config.box.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    sdk::Color color = GetPlayerColor(data, m_config.box.color, m_config.box.colorTeammate);
    
    float x = data.boxX;
    float y = data.boxY;
    float w = data.boxWidth;
    float h = data.boxHeight;
    
    if (m_config.box.type == ESPBoxConfig::Type::Box2D) {
        drawList->AddLine(ImVec2(x, y), ImVec2(x + w, y), color.ToU32(), 1.5f); drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), color.ToU32(), 1.5f); drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), color.ToU32(), 1.5f); drawList->AddLine(ImVec2(x, y + h), ImVec2(x, y), color.ToU32(), 1.5f);
    } else if (m_config.box.type == ESPBoxConfig::Type::Corner) {
        float cornerSize = std::min(w, h) * 0.3f;
        float thickness = 1.5f;
        
        // Top-left
        drawList->AddLine(ImVec2(x, y), ImVec2(x + cornerSize, y), color.ToU32(), thickness);
        drawList->AddLine(ImVec2(x, y), ImVec2(x, y + cornerSize), color.ToU32(), thickness);
        // Top-right
        drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w - cornerSize, y), color.ToU32(), thickness);
        drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + cornerSize), color.ToU32(), thickness);
        // Bottom-left
        drawList->AddLine(ImVec2(x, y + h), ImVec2(x + cornerSize, y + h), color.ToU32(), thickness);
        drawList->AddLine(ImVec2(x, y + h), ImVec2(x, y + h - cornerSize), color.ToU32(), thickness);
        // Bottom-right
        drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x + w - cornerSize, y + h), color.ToU32(), thickness);
        drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x + w, y + h - cornerSize), color.ToU32(), thickness);
    }
}

void ESP::DrawCornerBox(const PlayerESPData& data) {
    DrawBox(data);
}

void ESP::Draw3DBox(const PlayerESPData& data) {
    // Would draw 3D box using bone positions
}

void ESP::DrawSkeleton(const PlayerESPData& data) {
    if (!m_config.skeleton.enabled || !data.hasBoneMatrix) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.skeleton.color;
    
    // Bone connections
    const std::pair<int, int> bones[] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, // Spine
        {4, 5}, {5, 6}, // Neck to head
        {4, 7}, {7, 8}, {8, 9}, {9, 10}, // Left arm
        {4, 11}, {11, 12}, {12, 13}, {13, 14}, // Right arm
        {0, 15}, {15, 16}, {16, 17}, // Left leg
        {0, 18}, {18, 19}, {19, 20} // Right leg
    };
    
    for (auto& bone : bones) {
        if (bone.first >= 128 || bone.second >= 128) continue;
        
        sdk::Vector3D pos1 = data.boneMatrix[bone.first].GetOrigin();
        sdk::Vector3D pos2 = data.boneMatrix[bone.second].GetOrigin();
        
        sdk::Vector2D screen1, screen2;
        if (WorldToScreen(pos1, screen1) && WorldToScreen(pos2, screen2)) {
            drawList->AddLine(ImVec2(screen1.x, screen1.y), ImVec2(screen2.x, screen2.y), color.ToU32(), 1.0f);
        }
    }
}

void ESP::DrawHealthBar(const PlayerESPData& data) {
    if (!m_config.healthBar.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    float x = data.boxX - 6.0f;
    float y = data.boxY;
    float h = data.boxHeight;
    float w = 4.0f;
    
    float healthPct = static_cast<float>(data.health) / data.maxHealth;
    healthPct = std::clamp(healthPct, 0.0f, 1.0f);
    
    // Background
    drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 200), 2.0f);
    
    // Health color lerp
    sdk::Color healthColor = LerpColor(m_config.healthBar.colorLow, m_config.healthBar.color, healthPct);
    
    // Foreground
    float fillH = h * healthPct;
    drawList->AddRectFilled(ImVec2(x, y + h - fillH), ImVec2(x + w, y + h), healthColor.ToU32(), 2.0f);
    
    // Outline
    drawList->AddLine(ImVec2(x, y), ImVec2(x + w, y), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x, y + h), ImVec2(x, y), IM_COL32(0, 0, 0, 255), 2.0f);
}

void ESP::DrawHealthText(const PlayerESPData& data) {
    if (!m_config.healthText.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.healthText.color;
    
    std::string text = std::to_string(data.health) + " HP";
    ImVec2 pos = ImVec2(data.boxX - 30.0f, data.boxY + data.boxHeight * (1.0f - static_cast<float>(data.health) / data.maxHealth) - 8.0f);
    
    drawList->AddText(pos, color.ToU32(), text.c_str());
}

void ESP::DrawArmor(const PlayerESPData& data) {
    if (!m_config.armor.enabled || data.armor <= 0) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.armor.color;
    
    float x = data.boxX + data.boxWidth + 2.0f;
    float y = data.boxY;
    float h = data.boxHeight;
    float w = 4.0f;
    
    float armorPct = static_cast<float>(data.armor) / 100.0f;
    armorPct = std::clamp(armorPct, 0.0f, 1.0f);
    
    drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 200), 2.0f);
    float fillH = h * armorPct;
    drawList->AddRectFilled(ImVec2(x, y + h - fillH), ImVec2(x + w, y + h), color.ToU32(), 2.0f);
    drawList->AddLine(ImVec2(x, y), ImVec2(x + w, y), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x, y + h), ImVec2(x, y), IM_COL32(0, 0, 0, 255), 2.0f);
}

void ESP::DrawAmmo(const PlayerESPData& data) {
    if (!m_config.ammo.enabled || data.maxAmmo <= 0) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.ammo.color;
    
    float x = data.boxX;
    float y = data.boxY + data.boxHeight + 2.0f;
    float w = data.boxWidth;
    float h = 4.0f;
    
    float ammoPct = static_cast<float>(data.ammo) / data.maxAmmo;
    ammoPct = std::clamp(ammoPct, 0.0f, 1.0f);
    
    drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 200), 2.0f);
    drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w * ammoPct, y + h), color.ToU32(), 2.0f);
    drawList->AddLine(ImVec2(x, y), ImVec2(x + w, y), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), IM_COL32(0, 0, 0, 255), 2.0f); drawList->AddLine(ImVec2(x, y + h), ImVec2(x, y), IM_COL32(0, 0, 0, 255), 2.0f);
}

void ESP::DrawName(const PlayerESPData& data) {
    if (!m_config.name.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.name.color;
    
    ImVec2 pos = ImVec2(data.boxX + data.boxWidth * 0.5f, data.boxY - 16.0f);
    ImVec2 textSize = ImGui::CalcTextSize(data.name.c_str());
    pos.x -= textSize.x * 0.5f;
    
    drawList->AddText(pos, color.ToU32(), data.name.c_str());
}

void ESP::DrawWeapon(const PlayerESPData& data) {
    if (!m_config.weapon.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.weapon.color;
    
    std::string text = data.weaponName;
    if (m_config.weapon.type == ESPWeaponConfig::Type::Ammo) {
        text += " [" + std::to_string(data.ammo) + "/" + std::to_string(data.maxAmmo) + "]";
    }
    
    ImVec2 pos = ImVec2(data.boxX + data.boxWidth * 0.5f, data.boxY + data.boxHeight + 8.0f);
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    pos.x -= textSize.x * 0.5f;
    
    drawList->AddText(pos, color.ToU32(), text.c_str());
}

void ESP::DrawFlags(const PlayerESPData& data) {
    if (!m_config.flags.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.flags.color;
    
    float x = data.boxX + data.boxWidth + 6.0f;
    float y = data.boxY;
    
    if (m_config.flags.showMoney && data.money > 0) {
        drawList->AddText(ImVec2(x, y), color.ToU32(), fmt::format("${}", data.money).c_str());
        y += 14.0f;
    }
    if (m_config.flags.showArmor && data.armor > 0) {
        drawList->AddText(ImVec2(x, y), color.ToU32(), fmt::format("Kevlar{}", data.hasDefuser ? "+Helmet" : "").c_str());
        y += 14.0f;
    }
    if (m_config.flags.showKit && data.hasDefuser) {
        drawList->AddText(ImVec2(x, y), color.ToU32(), "Defuse Kit");
        y += 14.0f;
    }
    if (m_config.flags.showScoped && data.isScoped) {
        drawList->AddText(ImVec2(x, y), color.ToU32(), "Scoped");
        y += 14.0f;
    }
    if (m_config.flags.showFlashed && data.isFlashed) {
        drawList->AddText(ImVec2(x, y), color.ToU32(), "Flashed");
        y += 14.0f;
    }
    if (m_config.flags.showDefusing && data.isDefusing) {
        drawList->AddText(ImVec2(x, y), color.ToU32(), "Defusing");
        y += 14.0f;
    }
}

void ESP::DrawSnapline(const PlayerESPData& data) {
    if (!m_config.snaplines.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.snaplines.color;
    
    ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y);
    ImVec2 targetPos = data.screenPos.ToImVec2();
    
    if (m_config.snaplines.type == ESPSnaplinesConfig::Type::Center) {
        screenCenter.y = ImGui::GetIO().DisplaySize.y * 0.5f;
    } else if (m_config.snaplines.type == ESPSnaplinesConfig::Type::Top) {
        screenCenter.y = 0.0f;
    }
    
    drawList->AddLine(screenCenter, targetPos, color.ToU32(), 1.0f);
}

void ESP::DrawOffscreenArrow(const PlayerESPData& data) {
    if (!m_config.offscreenArrows.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.offscreenArrows.color;
    
    ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    sdk::Vector2D targetPos = data.screenPos;
    
    // Check if offscreen
    if (targetPos.x > 0 && targetPos.x < ImGui::GetIO().DisplaySize.x &&
        targetPos.y > 0 && targetPos.y < ImGui::GetIO().DisplaySize.y) {
        return;
    }
    
    // Calculate arrow position on screen edge
    sdk::Vector2D dir = targetPos - sdk::Vector2D(screenCenter.x, screenCenter.y);
    float len = dir.Length();
    if (len < 1.0f) return;
    
    dir = dir / len;
    sdk::Vector2D arrowPos = sdk::Vector2D(screenCenter.x, screenCenter.y) + dir * m_config.offscreenArrows.distance;
    
    // Clamp to screen
    arrowPos.x = std::clamp(arrowPos.x, m_config.offscreenArrows.size, ImGui::GetIO().DisplaySize.x - m_config.offscreenArrows.size);
    arrowPos.y = std::clamp(arrowPos.y, m_config.offscreenArrows.size, ImGui::GetIO().DisplaySize.y - m_config.offscreenArrows.size);
    
    // Draw arrow triangle
    float angle = atan2f(dir.y, dir.x);
    float size = m_config.offscreenArrows.size;
    
    ImVec2 p1 = ImVec2(arrowPos.x + cosf(angle) * size, arrowPos.y + sinf(angle) * size);
    ImVec2 p2 = ImVec2(arrowPos.x + cosf(angle + 2.5f) * size * 0.7f, arrowPos.y + sinf(angle + 2.5f) * size * 0.7f);
    ImVec2 p3 = ImVec2(arrowPos.x + cosf(angle - 2.5f) * size * 0.7f, arrowPos.y + sinf(angle - 2.5f) * size * 0.7f);
    
    drawList->AddTriangleFilled(p1, p2, p3, color.ToU32());
}

void ESP::DrawDormant(const PlayerESPData& data) {
    if (!m_config.dormant.enabled || !data.isDormant) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.dormant.color;
    
    // Draw "DORMANT" text
    ImVec2 pos = ImVec2(data.boxX + data.boxWidth * 0.5f, data.boxY - 30.0f);
    ImVec2 textSize = ImGui::CalcTextSize("DORMANT");
    pos.x -= textSize.x * 0.5f;
    
    drawList->AddText(pos, color.ToU32(), "DORMANT");
}

std::string ESP::GetWeaponName(sdk::CBaseWeapon* weapon) {
    if (!weapon) return "Unknown";
    return "Weapon";
}

sdk::CBaseEntity* ESP::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

sdk::CBaseWeapon* ESP::GetActiveWeapon() const {
    return nullptr;
}

std::string ESP::GetWeaponIcon(sdk::CBaseWeapon* weapon) {
    return "";
}

void ESP::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    // Load ESP config
}

void ESP::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    // Save ESP config
}

// ========== Chams ==========

void Chams::Initialize() {
    CreateMaterials();
    LoadConfig();
    LOG_INFO(Visuals, "Chams initialized");
}

void Chams::Shutdown() {
    DestroyMaterials();
    SaveConfig();
    LOG_INFO(Visuals, "Chams shutdown");
}

void Chams::OnDrawModelExecute(void* ctx, void* state, const void* info, const sdk::Matrix3x4* boneToWorld) {
    if (!m_config.enabled) return;
    
    // Would check entity and apply chams
}

void Chams::OnFrameStageNotify(int stage) {
    m_pulseTime += sdk::g_pGlobalVars->frame_time;
}

void Chams::CreateMaterials() {
    // Create VMT materials for different chams types
}

void Chams::DestroyMaterials() {
    // Release materials
}

void Chams::ApplyChams(const ChamsContext& ctx) {
    if (!m_config.enabled) return;
    
    // Apply material based on context
}

sdk::IMaterial* Chams::GetMaterial(ChamsConfig::ChamsMaterialConfig::Material type, bool ignoreZ) {
    switch (type) {
        case ChamsConfig::ChamsMaterialConfig::Material::Flat:
            return ignoreZ ? m_materialFlatIgnoreZ : m_materialFlat;
        case ChamsConfig::ChamsMaterialConfig::Material::Glass:
            return ignoreZ ? m_materialGlassIgnoreZ : m_materialGlass;
        case ChamsConfig::ChamsMaterialConfig::Material::Plastic:
            return ignoreZ ? m_materialPlasticIgnoreZ : m_materialPlastic;
        case ChamsConfig::ChamsMaterialConfig::Material::Metallic:
            return ignoreZ ? m_materialMetallicIgnoreZ : m_materialMetallic;
        case ChamsConfig::ChamsMaterialConfig::Material::Glow:
            return ignoreZ ? m_materialGlowIgnoreZ : m_materialGlow;
        case ChamsConfig::ChamsMaterialConfig::Material::Wireframe:
            return ignoreZ ? m_materialWireframeIgnoreZ : m_materialWireframe;
        case ChamsConfig::ChamsMaterialConfig::Material::Pulse:
            return ignoreZ ? m_materialPulseIgnoreZ : m_materialPulse;
    }
    return m_materialFlat;
}

void Chams::ApplyMaterial(sdk::IMaterial* mat, const sdk::Color& color, bool ignoreZ) {
    if (!mat) return;
    
    mat->ColorModulate(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
    mat->AlphaModulate(color.a / 255.0f);
}

void Chams::SetupPulseMaterial(float time) {
    // Update pulse material
}

void Chams::LoadConfig() {
}

void Chams::SaveConfig() {
}

// ========== World ==========

void World::Initialize() {
    LoadConfig();
    LOG_INFO(Visuals, "World initialized");
}

void World::Shutdown() {
    RemoveNightmode();
    SaveConfig();
    LOG_INFO(Visuals, "World shutdown");
}

void World::Render() {
    if (!m_config.enabled) return; // Would need enabled flag
    
    RenderHitmarkers();
    RenderGrenadePreview();
    RenderDamageIndicators();
    RenderSpreadCircle();
}

void World::OnFrameStageNotify(int stage) {
}

void World::OnFireEvent(void* event) {
    // Handle events for hitmarkers, damage indicators
}

void World::OnDispatchSound(void* sound) {
    // Handle sound ESP
}

void World::ApplyNightmode() {
    if (m_nightmodeApplied) return;
    
    // Find and modify world materials
    m_nightmodeApplied = true;
}

void World::RemoveNightmode() {
    if (!m_nightmodeApplied) return;
    
    // Restore original materials
    m_nightmodeApplied = false;
}

void World::UpdatePropTransparency() {
}

void World::RenderHitmarkers() {
    if (!m_config.hitmarker.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.hitmarker.color;
    
    for (auto& hm : m_hitmarkers) {
        float age = sdk::g_pGlobalVars->current_time - hm.time;
        if (age > m_config.hitmarker.duration) continue;
        
        float alpha = 1.0f - age / m_config.hitmarker.duration;
        ImU32 c = (color.ToU32() & 0x00FFFFFF) | (static_cast<uint8_t>(255 * alpha) << 24);
        
        ImVec2 screen;
        if (gui::ViceRender::Instance().WorldToScreen(hm.position, screen)) {
            float size = 8.0f;
            drawList->AddLine(
                ImVec2(screen.x - size, screen.y - size),
                ImVec2(screen.x + size, screen.y + size),
                c, 2.0f
            );
            drawList->AddLine(
                ImVec2(screen.x + size, screen.y - size),
                ImVec2(screen.x - size, screen.y + size),
                c, 2.0f
            );
        }
    }
}

void World::RenderGrenadePreview() {
    if (!m_config.grenadePreview.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    sdk::Color color = m_config.grenadePreview.color;
    
    for (auto& traj : m_grenadeTrajectories) {
        float age = sdk::g_pGlobalVars->current_time - traj.time;
        if (age > 5.0f) continue;
        
        for (size_t i = 1; i < traj.points.size(); ++i) {
            ImVec2 p1, p2;
            if (gui::ViceRender::Instance().WorldToScreen(traj.points[i-1], p1) &&
                gui::ViceRender::Instance().WorldToScreen(traj.points[i], p2)) {
                drawList->AddLine(p1, p2, color.ToU32(), 2.0f);
            }
        }
    }
}

void World::RenderDamageIndicators() {
    if (!m_config.damageIndicator.enabled) return;
}

void World::RenderSpreadCircle() {
    if (!m_config.spreadCircle.enabled) return;
    
    auto local = GetLocalPlayer();
    if (!local) return;
    
    auto weapon = GetActiveWeapon();
    if (!weapon) return;
    
    // Get spread cone and draw circle at crosshair
}

void World::AddHitmarker(const sdk::Vector3D& pos, int damage, bool headshot) {
    HitmarkerData hm;
    hm.position = pos;
    hm.time = sdk::g_pGlobalVars->current_time;
    hm.damage = damage;
    hm.isHeadshot = headshot;
    m_hitmarkers.push_back(hm);
}

void World::AddGrenadeTrajectory(const std::vector<sdk::Vector3D>& points) {
    GrenadeTrajectory gt;
    gt.points = points;
    gt.color = m_config.grenadePreview.color;
    gt.time = sdk::g_pGlobalVars->current_time;
    m_grenadeTrajectories.push_back(gt);
}

void World::SimulateGrenadeTrajectory(sdk::CBaseEntity* grenade, std::vector<sdk::Vector3D>& points) {
    // Physics simulation for grenade trajectory
}

void World::LoadConfig() {
}

void World::SaveConfig() {
}

sdk::CBaseEntity* World::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

sdk::CBaseWeapon* World::GetActiveWeapon() const {
    return nullptr;
}

// ========== Radar ==========

void Radar::Initialize() {
    LoadConfig();
    LOG_INFO(Visuals, "Radar initialized");
}

void Radar::Shutdown() {
    SaveConfig();
    LOG_INFO(Visuals, "Radar shutdown");
}

void Radar::Render() {
    if (!m_config.enabled) return;
    
    UpdatePlayers();
    RenderBackground();
    RenderPlayers();
    RenderBomb();
}

void Radar::UpdatePlayers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_players.clear();
    
    int maxClients = sdk::g_pEngine->GetMaxClients();
    auto local = GetLocalPlayer();
    if (!local) return;
    
    sdk::Vector3D localOrigin = {}; // local->GetAbsOrigin()
    sdk::QAngle localAngles = {}; // local->GetAbsAngles()
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(i));
        if (!entity) continue;
        
        RadarPlayer rp;
        rp.isTeammate = false; // entity->GetTeam() == local->GetTeam()
        rp.isLocalPlayer = false;
        rp.health = 100;
        rp.isDormant = false;
        
        sdk::Vector3D origin = {}; // entity->GetAbsOrigin()
        rp.position = WorldToRadar(origin);
        
        if (IsInRadarRange(origin)) {
            m_players.push_back(rp);
        }
    }
}

void Radar::RenderBackground() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = m_config.position;
    float size = m_config.size;
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(28, 24, 28, 255), 4.0f);
    drawList->AddRect(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(55, 45, 55, 255), 4.0f, ImDrawFlags_None, 2.0f);
    
    // Center lines
    ImVec2 center = ImVec2(pos.x + size * 0.5f, pos.y + size * 0.5f);
    drawList->AddLine(ImVec2(center.x, pos.y), ImVec2(center.x, pos.y + size), IM_COL32(55, 45, 55, 255));
    drawList->AddLine(ImVec2(pos.x, center.y), ImVec2(pos.x + size, center.y), IM_COL32(55, 45, 55, 255));
    
    // Range circles
    for (int i = 1; i <= 3; ++i) {
        float r = size * 0.5f * (i / 3.0f);
        drawList->AddCircle(center, r, IM_COL32(55, 45, 55, 255), 32);
    }
}

void Radar::RenderPlayers() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = m_config.position;
    float size = m_config.size;
    ImVec2 center = ImVec2(pos.x + size * 0.5f, pos.y + size * 0.5f);
    
    for (auto& player : m_players) {
        ImU32 color = player.isTeammate ? IM_COL32(95, 155, 255, 255) : IM_COL32(255, 95, 155, 255);
        if (player.isDormant) color = IM_COL32(100, 100, 100, 150);
        
        float dotSize = player.isLocalPlayer ? 6.0f : 4.0f;
        drawList->AddCircleFilled(player.position, dotSize, color);
        
        if (player.health < 100) {
            // Health indicator
        }
    }
}

void Radar::RenderBomb() {
    // Render bomb on radar
}

sdk::Vector2D Radar::WorldToRadar(const sdk::Vector3D& worldPos) {
    // Convert world position to radar coordinates
    auto local = GetLocalPlayer();
    if (!local) return m_config.position;
    
    sdk::Vector3D localOrigin = {}; // local->GetAbsOrigin()
    sdk::QAngle localAngles = {}; // local->GetAbsAngles()
    
    sdk::Vector3D delta = worldPos - localOrigin;
    
    // Rotate by inverse view angles
    float yaw = -localAngles.yaw * 3.14159f / 180.0f;
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);
    
    float x = delta.x * cosYaw - delta.y * sinYaw;
    float y = delta.x * sinYaw + delta.y * cosYaw;
    
    // Scale to radar size
    float scale = m_config.size / (m_config.range * 2.0f);
    x = x * scale + m_config.size * 0.5f;
    y = -y * scale + m_config.size * 0.5f;
    
    return ImVec2(m_config.position.x + x, m_config.position.y + y);
}

bool Radar::IsInRadarRange(const sdk::Vector3D& worldPos) {
    auto local = GetLocalPlayer();
    if (!local) return false;
    
    sdk::Vector3D localOrigin = {}; // local->GetAbsOrigin()
    return localOrigin.DistTo(worldPos) <= m_config.range;
}

void Radar::LoadConfig() {
}

void Radar::SaveConfig() {
}

// ========== Effects ==========

void Effects::Initialize() {
    LoadConfig();
    LOG_INFO(Visuals, "Effects initialized");
}

void Effects::Shutdown() {
    SaveConfig();
    LOG_INFO(Visuals, "Effects shutdown");
}

void Effects::Render() {
    RenderTracers();
    RenderImpacts();
    RenderHitNumbers();
}

void Effects::OnFireEvent(void* event) {
    // Handle events for tracers, impacts, hit numbers
}

void Effects::AddTracer(const sdk::Vector3D& start, const sdk::Vector3D& end, const sdk::Color& color) {
    TracerData td;
    td.start = start;
    td.end = end;
    td.color = color;
    td.time = sdk::g_pGlobalVars->current_time;
    td.duration = m_config.bulletTracers.duration;
    m_tracers.push_back(td);
}

void Effects::AddImpact(const sdk::Vector3D& pos, const sdk::Vector3D& normal, const sdk::Color& color) {
    ImpactData id;
    id.position = pos;
    id.normal = normal;
    id.color = color;
    id.time = sdk::g_pGlobalVars->current_time;
    id.duration = m_config.impactBeams.duration;
    m_impacts.push_back(id);
}

void Effects::AddHitNumber(const sdk::Vector3D& pos, int damage, bool headshot) {
    HitNumberData hn;
    hn.position = pos;
    hn.damage = damage;
    hn.headshot = headshot;
    hn.time = sdk::g_pGlobalVars->current_time;
    hn.duration = 1.5f;
    m_hitNumbers.push_back(hn);
}

void Effects::RenderTracers() {
    if (!m_config.bulletTracers.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    for (auto& t : m_tracers) {
        float age = sdk::g_pGlobalVars->current_time - t.time;
        if (age > t.duration) continue;
        
        float alpha = 1.0f - age / t.duration;
        ImU32 c = (t.color.ToU32() & 0x00FFFFFF) | (static_cast<uint8_t>(255 * alpha) << 24);
        
        ImVec2 s1, s2;
        if (gui::ViceRender::Instance().WorldToScreen(t.start, s1) &&
            gui::ViceRender::Instance().WorldToScreen(t.end, s2)) {
            DrawBeam(s1, s2, t.color, 2.0f);
        }
    }
}

void Effects::RenderImpacts() {
    if (!m_config.impactBeams.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    for (auto& i : m_impacts) {
        float age = sdk::g_pGlobalVars->current_time - i.time;
        if (age > i.duration) continue;
        
        float alpha = 1.0f - age / i.duration;
        ImU32 c = (i.color.ToU32() & 0x00FFFFFF) | (static_cast<uint8_t>(255 * alpha) << 24);
        
        ImVec2 screen;
        if (gui::ViceRender::Instance().WorldToScreen(i.position, screen)) {
            DrawBeamRing(screen, ImVec2(0, 0), i.color, 10.0f, 2.0f);
        }
    }
}

void Effects::RenderHitNumbers() {
    if (!m_config.hitNumbers.enabled) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImFont* font = gui::ViceFonts::Instance().GetFont(gui::FontType::UI_Normal);
    
    for (auto& hn : m_hitNumbers) {
        float age = sdk::g_pGlobalVars->current_time - hn.time;
        if (age > hn.duration) continue;
        
        float alpha = 1.0f - age / hn.duration;
        sdk::Color color = m_config.hitNumbers.color;
        ImU32 c = (color.ToU32() & 0x00FFFFFF) | (static_cast<uint8_t>(255 * alpha) << 24);
        
        ImVec2 screen;
        if (gui::ViceRender::Instance().WorldToScreen(hn.position, screen)) {
            // Float up over time
            screen.y -= age * 50.0f;
            
            std::string text = fmt::format("-{}", hn.damage);
            if (hn.headshot) text += " HS";
            
            drawList->AddText(font, m_config.hitNumbers.fontSize, screen, c, text.c_str());
        }
    }
}

void Effects::DrawBeam(const sdk::Vector3D& start, const sdk::Vector3D& end, const sdk::Color& color, float width) {
    // Draw 3D beam
}

void Effects::DrawBeam(const ImVec2& start, const ImVec2& end, const sdk::Color& color, float width) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddLine(start, end, color.ToU32(), width);
}

void Effects::DrawBeamRing(const sdk::Vector3D& pos, const sdk::Vector3D& normal, const sdk::Color& color, float radius, float width) {
    // Draw impact ring
}

void Effects::DrawBeamRing(const ImVec2& pos, const ImVec2& normal, const sdk::Color& color, float radius, float width) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddCircle(pos, radius, color.ToU32(), 32, width);
}

void Effects::LoadConfig() {
}

void Effects::SaveConfig() {
}

sdk::CBaseEntity* Radar::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

sdk::CBaseWeapon* Radar::GetActiveWeapon() const {
    return nullptr;
}

sdk::CBaseEntity* Effects::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

sdk::CBaseWeapon* Effects::GetActiveWeapon() const {
    return nullptr;
}

} // namespace features::visuals