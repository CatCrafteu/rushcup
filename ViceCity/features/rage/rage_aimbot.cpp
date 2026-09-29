#include "core/pch.hpp"
#include "features/rage/rage_aimbot.hpp"
#include "features/rage/autowall.hpp"
#include "features/rage/exploitation.hpp"
#include "features/rage/anti_aim.hpp"
#include "features/rage/resolver.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/memory/pattern_scanner.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

namespace features::rage {

void RageAimbot::Initialize() {
    LoadConfig();
    LOG_INFO(Rage, "RageAimbot initialized");
}

void RageAimbot::Shutdown() {
    SaveConfig();
    LOG_INFO(Rage, "RageAimbot shutdown");
}

void RageAimbot::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
    if (!cmd) return;
    
    auto local = GetLocalPlayer();
    if (!local) return;
    
    auto weapon = GetActiveWeapon();
    if (!weapon || !IsWeaponValid(weapon)) return;
    
    // Find best target
    auto target = FindBestTarget(cmd);
    if (!target) return;
    
    // Calculate aim angles
    sdk::QAngle aimAngles;
    if (!CalculateAim(cmd, *target, aimAngles)) return;
    
    // Apply RCS
    if (m_config.enabled) {
        ApplyRCS(cmd, aimAngles);
    }
    
    // Auto stop
    if (m_config.autoStop) {
        AutoStop(cmd);
    }
    
    // Auto scope
    if (m_config.autoScope) {
        AutoScope(cmd);
    }
    
    // Fix movement for silent aim
    if (m_config.silentAim) {
        FixMovement(cmd, cmd->view_angles);
    }
    
    // Set view angles
    cmd->view_angles = aimAngles;
    
    m_lastAimAngles = aimAngles;
    m_wasFiring = (cmd->buttons & 1) != 0; // IN_ATTACK
}

void RageAimbot::OnFrameStageNotify(int stage) {
    // Update resolver data, etc.
}

std::optional<TargetInfo> RageAimbot::FindBestTarget(sdk::CUserCmd* cmd) {
    auto targets = GetValidTargets(cmd);
    if (targets.empty()) return std::nullopt;
    
    SortTargets(targets);
    
    if (!targets.empty()) {
        m_lastTargetIndex = targets[0].entity ? 
            reinterpret_cast<uintptr_t>(targets[0].entity) : -1;
        return targets[0];
    }
    
    return std::nullopt;
}

std::vector<TargetInfo> RageAimbot::GetValidTargets(sdk::CUserCmd* cmd) {
    std::vector<TargetInfo> targets;
    
    auto local = GetLocalPlayer();
    if (!local) return targets;
    
    int localTeam = 0; // Would get from entity
    int maxClients = sdk::g_pEngine->GetMaxClients();
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(i));
        if (!entity) continue;
        
        // Check if alive, not dormant, enemy team, etc.
        // This would use actual SDK calls
        
        // For each hitbox in priority order
        for (int hitbox : m_config.hitboxPriority) {
            sdk::Vector3D hitboxPos; // Would get from entity
            
            // Check visibility
            bool visible = true; // Would trace
            
            // Calculate damage
            float damage = 100.0f; // Would calculate
            
            // Calculate hitchance
            float hc = GetHitchance({entity, hitbox, hitboxPos, damage, 0, 0, false, 0}, cmd);
            
            if (hc >= m_config.hitchance.minimum) {
                TargetInfo info;
                info.entity = entity;
                info.hitbox = hitbox;
                info.hitboxPos = hitboxPos;
                info.damage = damage;
                info.hitchance = hc;
                targets.push_back(info);
            }
        }
    }
    
    return targets;
}

bool RageAimbot::CalculateAim(sdk::CUserCmd* cmd, const TargetInfo& target, sdk::QAngle& outAngles) {
    if (!target.entity) return false;
    
    auto local = GetLocalPlayer();
    if (!local) return false;
    
    sdk::Vector3D localEyePos; // Would get from local player
    sdk::Vector3D targetPos = target.hitboxPos;
    
    sdk::Vector3D delta = targetPos - localEyePos;
    outAngles = sdk::VectorAngles(delta);
    outAngles.Normalize();
    
    return true;
}

bool RageAimbot::CanHit(const TargetInfo& target, sdk::CUserCmd* cmd) {
    return GetHitchance(target, cmd) >= m_config.hitchance.minimum;
}

float RageAimbot::GetHitchance(const TargetInfo& target, sdk::CUserCmd* cmd) {
    if (!m_config.hitchance.enabled) return 100.0f;
    
    // Simplified hitchance calculation
    // Real implementation would trace multiple rays
    return 95.0f;
}

std::vector<AimPoint> RageAimbot::GenerateMultipoints(sdk::CBaseEntity* entity, int hitbox) {
    std::vector<AimPoint> points;
    
    if (!m_config.multipoint.enabled) {
        // Just center point
        AimPoint center;
        center.position = sdk::Vector3D{}; // Would get hitbox center
        center.damage = 100.0f;
        center.hitchance = 95.0f;
        center.hitbox = hitbox;
        center.isCenter = true;
        points.push_back(center);
        return points;
    }
    
    float scale = (hitbox == 0) ? m_config.multipoint.headScale : m_config.multipoint.bodyScale;
    scale *= m_config.multipoint.pointScale;
    
    auto hitboxPoints = GetHitboxPoints(entity, hitbox, scale);
    
    for (size_t i = 0; i < hitboxPoints.size(); ++i) {
        AimPoint p;
        p.position = hitboxPoints[i];
        p.damage = 100.0f;
        p.hitchance = 90.0f;
        p.hitbox = hitbox;
        p.isCenter = (i == 0);
        points.push_back(p);
    }
    
    return points;
}

std::vector<sdk::Vector3D> RageAimbot::GetHitboxPoints(sdk::CBaseEntity* entity, int hitbox, float scale) {
    std::vector<sdk::Vector3D> points;
    
    // Would get hitbox min/max from entity and generate points
    // Center point
    points.push_back(sdk::Vector3D{}); // Hitbox center
    
    if (scale > 0) {
        // Add multipoint points around hitbox
        int numPoints = m_config.multipoint.headPoints;
        for (int i = 0; i < numPoints; ++i) {
            float angle = (static_cast<float>(i) / numPoints) * 2.0f * 3.14159f;
            points.push_back(sdk::Vector3D{
                cosf(angle) * scale,
                sinf(angle) * scale,
                0
            });
        }
    }
    
    return points;
}

RageAimbot::AutowallResult RageAimbot::TraceAutowall(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target, sdk::CBaseEntity* ignore) {
    AutowallResult result;
    
    // Would trace through walls and calculate damage
    // Simplified
    result.hit = true;
    result.damage = 80.0f;
    result.endPos = end;
    result.hitbox = 0;
    result.penetrationDistance = 0.0f;
    
    return result;
}

float RageAimbot::CalculateDamage(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target) {
    // Would calculate damage through walls
    return 100.0f;
}

bool RageAimbot::TraceToExit(const sdk::Vector3D& start, const sdk::Vector3D& dir, sdk::Vector3D& end, float maxDistance) {
    // Would trace to find exit point
    return false;
}

sdk::QAngle RageAimbot::GetRecoilAngles() {
    auto local = GetLocalPlayer();
    if (!local) return {};
    
    // Would get aim punch angle
    return {};
}

void RageAimbot::ApplyRCS(sdk::CUserCmd* cmd, const sdk::QAngle& aimAngles) {
    auto local = GetLocalPlayer();
    if (!local) return;
    
    sdk::QAngle punch = GetRecoilAngles();
    sdk::QAngle compensated = aimAngles - punch * 2.0f; // 2x for RCS
    compensated.Normalize();
    
    cmd->view_angles = compensated;
}

void RageAimbot::AutoStop(sdk::CUserCmd* cmd) {
    auto local = GetLocalPlayer();
    if (!local) return;
    
    // Would check velocity and apply counter-movement
    sdk::Vector3D velocity = {}; // Would get from local
    
    if (velocity.Length2D() > 5.0f) {
        // Calculate direction to stop
        sdk::QAngle moveAngles = sdk::VectorAngles(velocity);
        moveAngles.yaw += 180.0f;
        moveAngles.Normalize();
        
        sdk::Vector3D moveDir = sdk::AngleVectors(moveAngles);
        
        cmd->forward_move = -moveDir.x * 450.0f;
        cmd->side_move = -moveDir.y * 450.0f;
    }
}

bool RageAimbot::ShouldAutoStop(sdk::CUserCmd* cmd) {
    auto local = GetLocalPlayer();
    if (!local) return false;
    
    // Would check velocity, in-air, etc.
    return false;
}

void RageAimbot::AutoScope(sdk::CUserCmd* cmd) {
    auto weapon = GetActiveWeapon();
    if (!weapon) return;
    
    // Would check if weapon is sniper and not scoped
    // If so, set IN_ATTACK2
}

void RageAimbot::FixMovement(sdk::CUserCmd* cmd, const sdk::QAngle& originalAngles) {
    // Fix movement for silent aim
    sdk::Vector3D move = {cmd->forward_move, cmd->side_move, cmd->up_move};
    float speed = move.Length2D();
    
    sdk::QAngle delta = cmd->view_angles - originalAngles;
    delta.Normalize();
    
    float yawDelta = delta.yaw * 3.14159f / 180.0f;
    
    cmd->forward_move = cosf(yawDelta) * move.x - sinf(yawDelta) * move.y;
    cmd->side_move = sinf(yawDelta) * move.x + cosf(yawDelta) * move.y;
}

sdk::CBaseEntity* RageAimbot::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

sdk::CBaseWeapon* RageAimbot::GetActiveWeapon() const {
    auto local = GetLocalPlayer();
    if (!local) return nullptr;
    
    // Would get active weapon handle and resolve
    return nullptr;
}

bool RageAimbot::IsWeaponValid(sdk::CBaseWeapon* weapon) const {
    if (!weapon) return false;
    
    // Would check weapon type, ammo, etc.
    return true;
}

bool RageAimbot::IsAbleToShoot(sdk::CUserCmd* cmd) const {
    auto weapon = GetActiveWeapon();
    if (!weapon) return false;
    
    // Would check next attack time, reload, etc.
    return true;
}

float RageAimbot::GetTargetPriority(sdk::CBaseEntity* entity) {
    float priority = 0.0f;
    
    switch (m_config.targetSelection) {
        case RageAimbotConfig::TargetSelection::FOV:
            priority = 100.0f; // Would calculate FOV
            break;
        case RageAimbotConfig::TargetSelection::Distance:
            priority = 1000.0f; // Would calculate distance
            break;
        case RageAimbotConfig::TargetSelection::Damage:
            priority = 100.0f; // Would calculate damage
            break;
        case RageAimbotConfig::TargetSelection::Health:
            priority = 100.0f; // Would get health
            break;
        case RageAimbotConfig::TargetSelection::Threat:
            priority = 100.0f; // Would calculate threat
            break;
    }
    
    return priority;
}

void RageAimbot::SortTargets(std::vector<TargetInfo>& targets) {
    std::sort(targets.begin(), targets.end(), [this](const TargetInfo& a, const TargetInfo& b) {
        float pa = GetTargetPriority(a.entity);
        float pb = GetTargetPriority(b.entity);
        return pa > pb;
    });
}

void RageAimbot::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("aimbot_rage", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, true);
    load("silent_aim", m_config.silentAim, true);
    load("auto_scope", m_config.autoScope, true);
    load("auto_stop", m_config.autoStop, true);
    
    // Load more config values...
}

void RageAimbot::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("aimbot_rage", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("silent_aim", m_config.silentAim);
    save("auto_scope", m_config.autoScope);
    save("auto_stop", m_config.autoStop);
    
    // Save more config values...
}

} // namespace features::rage