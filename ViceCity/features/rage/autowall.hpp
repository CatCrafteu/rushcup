#pragma once

#include <vector>
#include <optional>
#include <array>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"
#include "../../core/logger/logger.hpp"
#include "rage_aimbot.hpp"

namespace features::rage {

struct AutowallData {
    sdk::Vector3D start;
    sdk::Vector3D end;
    sdk::Vector3D direction;
    float damage = 0.0f;
    float penetrationDistance = 0.0f;
    int hitgroup = 0;
    bool valid = false;
    sdk::CBaseEntity* hitEntity = nullptr;
    sdk::Trace_t trace;
};

class Autowall {
public:
    static Autowall& Instance() {
        static Autowall instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    
    AutowallData CalculateDamage(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target = nullptr);
    AutowallData CalculateDamage(const sdk::Vector3D& start, const sdk::Vector3D& direction, float maxDistance, sdk::CBaseEntity* target = nullptr);
    
    bool CanHit(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target, float minDamage = 1.0f);
    bool CanHitHitbox(const sdk::Vector3D& start, sdk::CBaseEntity* target, int hitbox, float minDamage = 1.0f);
    
    float GetDamage(sdk::CBaseEntity* target, int hitbox, const sdk::Vector3D& from);
    float GetDamage(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target);
    
    void ScaleDamage(sdk::CBaseEntity* target, int hitgroup, float& damage);
    bool TraceToExit(sdk::Trace_t& enterTrace, sdk::Vector3D& start, sdk::Vector3D& direction, sdk::Trace_t& exitTrace);
    bool HandleBulletPenetration(sdk::Trace_t& trace, sdk::Vector3D& direction, float& currentDamage, float penetrationPower, int& penetrationCount);
    
    AutowallConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    Autowall() = default;
    AutowallConfig m_config;
    
    struct FireBulletData {
        sdk::Vector3D src;
        sdk::Vector3D direction;
        sdk::Trace_t enterTrace;
        float currentDamage;
        float penetrationPower;
        int penetrationCount;
        sdk::CBaseEntity* target;
        int hitbox;
    };
    
    bool SimulateFireBullet(FireBulletData& data);
    float GetHitgroupDamageMultiplier(int hitgroup);
    float GetArmorRatio(sdk::CBaseWeapon* weapon);
    bool IsArmored(sdk::CBaseEntity* target, int hitgroup);
    
    static constexpr const char* CONFIG_KEY = "rage_autowall";
};

} // namespace features::rage