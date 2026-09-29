#include "core/pch.hpp"
#include "features/rage/autowall.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

namespace features::rage {

void Autowall::Initialize() {
    LoadConfig();
    LOG_INFO(Rage, "Autowall initialized");
}

void Autowall::Shutdown() {
    SaveConfig();
    LOG_INFO(Rage, "Autowall shutdown");
}

AutowallData Autowall::CalculateDamage(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target) {
    AutowallData data;
    data.start = start;
    data.end = end;
    data.direction = (end - start).Normalized();
    data.damage = 0.0f;
    data.penetrationDistance = 0.0f;
    data.hitgroup = 0;
    data.valid = false;
    data.hitEntity = target;

    // Simplified - would do full bullet penetration simulation
    FireBulletData bulletData;
    bulletData.src = start;
    bulletData.direction = data.direction;
    bulletData.currentDamage = 100.0f; // Would get from weapon
    bulletData.penetrationPower = 1.0f;
    bulletData.penetrationCount = 0;
    bulletData.target = target;
    bulletData.hitbox = 0;

    if (SimulateFireBullet(bulletData)) {
        data.damage = bulletData.currentDamage;
        data.valid = true;
        data.hitgroup = 1; // Would be actual hitgroup
    }

    return data;
}

AutowallData Autowall::CalculateDamage(const sdk::Vector3D& start, const sdk::Vector3D& direction, float maxDistance, sdk::CBaseEntity* target) {
    sdk::Vector3D end = start + direction * maxDistance;
    return CalculateDamage(start, end, target);
}

bool Autowall::CanHit(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target, float minDamage) {
    auto data = CalculateDamage(start, end, target);
    return data.valid && data.damage >= minDamage;
}

bool Autowall::CanHitHitbox(const sdk::Vector3D& start, sdk::CBaseEntity* target, int hitbox, float minDamage) {
    // Would get hitbox position and trace
    return false;
}

float Autowall::GetDamage(sdk::CBaseEntity* target, int hitbox, const sdk::Vector3D& from) {
    // Simplified
    return 50.0f;
}

float Autowall::GetDamage(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target) {
    auto data = CalculateDamage(start, end, target);
    return data.damage;
}

void Autowall::ScaleDamage(sdk::CBaseEntity* target, int hitgroup, float& damage) {
    // Scale damage based on hitgroup
    switch (hitgroup) {
        case 1: damage *= 4.0f; break; // Head
        case 2: damage *= 1.0f; break; // Chest
        case 3: damage *= 1.25f; break; // Stomach
        case 4: case 5: damage *= 0.75f; break; // Arms/Legs
        default: damage *= 1.0f; break;
    }

    // Apply armor reduction
    if (IsArmored(target, hitgroup)) {
        float armorRatio = GetArmorRatio(nullptr); // Would get from weapon
        float heavyArmor = 1.0f; // Would check for heavy armor
        float newDamage = damage * armorRatio * heavyArmor;
        damage = newDamage;
    }
}

bool Autowall::TraceToExit(sdk::Trace_t& enterTrace, sdk::Vector3D& start, sdk::Vector3D& direction, sdk::Trace_t& exitTrace) {
    // Trace to find exit point
    return false;
}

bool Autowall::HandleBulletPenetration(sdk::Trace_t& trace, sdk::Vector3D& direction, float& currentDamage, float penetrationPower, int& penetrationCount) {
    // Handle bullet penetration through surfaces
    return false;
}

bool Autowall::SimulateFireBullet(FireBulletData& data) {
    // Simplified fire bullet simulation
    // Would trace, handle penetration, calculate damage
    return true;
}

float Autowall::GetHitgroupDamageMultiplier(int hitgroup) {
    switch (hitgroup) {
        case 1: return 4.0f; // Head
        case 2: return 1.0f; // Chest
        case 3: return 1.25f; // Stomach
        case 4: case 5: return 0.75f; // Arms/Legs
        default: return 1.0f;
    }
}

float Autowall::GetArmorRatio(sdk::CBaseWeapon* weapon) {
    return 0.5f; // Default armor ratio
}

bool Autowall::IsArmored(sdk::CBaseEntity* target, int hitgroup) {
    // Check if target has armor on hitgroup
    return false;
}

void Autowall::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();

    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("aimbot_rage.autowall", key, def);
        if (opt) value = *opt;
    };

    load("enabled", m_config.enabled, true);
    load("min_damage", m_config.minDamage, 1.0f);
    load("min_damage_auto", m_config.minDamageAuto, 1.0f);
    load("auto_wall", m_config.autoWall, true);
    load("auto_scope", m_config.autoScope, false);
    load("min_hit_chance", m_config.minHitChance, 75);
    load("hit_chance", m_config.hitChance, true);
    load("multipoint", m_config.multipoint, true);
    load("multipoint_scale", m_config.multipointScale, 0.8f);
    load("body_aim_if_lethal", m_config.bodyAimIfLethal, true);
    load("body_aim_if_hp", m_config.bodyAimIfHP, true);
    load("body_aim_hp", m_config.bodyAimHP, 30);
    load("prefer_body_aim", m_config.preferBodyAim, false);
    load("force_body_aim_on_key", m_config.forceBodyAimOnKey, false);
    load("force_body_aim_key", m_config.forceBodyAimKey, std::string(""));
    load("scale_damage", m_config.scaleDamage, true);
    load("damage_scale", m_config.damageScale, 1.0f);
}

void Autowall::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();

    auto save = [&](const char* key, const auto& value) {
        config.Set("aimbot_rage.autowall", key, value);
    };

    save("enabled", m_config.enabled);
    save("min_damage", m_config.minDamage);
    save("min_damage_auto", m_config.minDamageAuto);
    save("auto_wall", m_config.autoWall);
    save("auto_scope", m_config.autoScope);
    save("min_hit_chance", m_config.minHitChance);
    save("hit_chance", m_config.hitChance);
    save("multipoint", m_config.multipoint);
    save("multipoint_scale", m_config.multipointScale);
    save("body_aim_if_lethal", m_config.bodyAimIfLethal);
    save("body_aim_if_hp", m_config.bodyAimIfHP);
    save("body_aim_hp", m_config.bodyAimHP);
    save("prefer_body_aim", m_config.preferBodyAim);
    save("force_body_aim_on_key", m_config.forceBodyAimOnKey);
    save("force_body_aim_key", m_config.forceBodyAimKey);
    save("scale_damage", m_config.scaleDamage);
    save("damage_scale", m_config.damageScale);
}

} // namespace features::rage