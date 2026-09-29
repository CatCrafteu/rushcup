#pragma once

#include <vector>
#include <optional>
#include <array>
#include <memory>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/memory/pattern_scanner.hpp"
#include "../../core/config/config_manager.hpp"

namespace features::rage {

struct TargetInfo {
    sdk::CBaseEntity* entity = nullptr;
    int hitbox = 0;
    sdk::Vector3D hitboxPos;
    float damage = 0.0f;
    float hitchance = 0.0f;
    int multipointIndex = 0;
    bool backtrack = false;
    int backtrackTick = 0;
};

struct AimPoint {
    sdk::Vector3D position;
    float damage = 0.0f;
    float hitchance = 0.0f;
    int hitbox = 0;
    bool isCenter = false;
};

struct MultipointConfig {
    bool enabled = true;
    float headScale = 0.8f;
    float bodyScale = 0.8f;
    float pointScale = 1.0f;
    int headPoints = 9; // 3, 5, 9, 13, 17
};

struct HitchanceConfig {
    bool enabled = true;
    float minimum = 80.0f;
    enum class Mode { Standard, Advanced } mode = Mode::Advanced;
    bool seedSync = true;
};

struct MinimumDamageConfig {
    float visible = 1.0f;
    float autoWall = 1.0f;
    std::string overrideKey;
    float overrideValue = 100.0f;
};

struct AutowallConfig {
    bool enabled = true;
    float minDamage = 1.0f;
    float minDamageAuto = 1.0f;
    bool autoWall = true;
    bool autoScope = false;
    int minHitChance = 75;
    bool hitChance = true;
    bool multipoint = true;
    float multipointScale = 0.8f;
    bool bodyAimIfLethal = true;
    bool bodyAimIfHP = true;
    int bodyAimHP = 30;
    bool preferBodyAim = false;
    bool forceBodyAimOnKey = false;
    std::string forceBodyAimKey;
    bool scaleDamage = true;
    float damageScale = 1.0f;
};

struct PreferBodyAimConfig {
    bool lethal = true;
    int hpThreshold = 0;
    bool inAir = false;
    std::string onKey;
};

struct RageAimbotConfig {
    bool enabled = true;
    bool silentAim = true;
    bool autoScope = true;
    bool autoStop = true;
    std::vector<std::string> autoStopModifiers;
    
    enum class TargetSelection { FOV, Distance, Damage, Health, Threat };
    TargetSelection targetSelection = TargetSelection::Damage;
    
    std::vector<int> hitboxPriority = {0, 1, 2, 3, 4, 5, 6, 7}; // Head, Neck, Chest, Stomach, Pelvis, Arms, Legs
    
    MultipointConfig multipoint;
    HitchanceConfig hitchance;
    MinimumDamageConfig minimumDamage;
    AutowallConfig autowall;
    PreferBodyAimConfig preferBodyAim;
    std::string forceBodyAimKey;
    std::string safePointsKey;
};

class RageAimbot {
public:
    static RageAimbot& Instance() {
        static RageAimbot instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    
    // Config
    RageAimbotConfig& GetConfig() { return m_config; }
    const RageAimbotConfig& GetConfig() const { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    // Target selection
    std::optional<TargetInfo> FindBestTarget(sdk::CUserCmd* cmd);
    std::vector<TargetInfo> GetValidTargets(sdk::CUserCmd* cmd);
    
    // Aim logic
    bool CalculateAim(sdk::CUserCmd* cmd, const TargetInfo& target, sdk::QAngle& outAngles);
    bool CanHit(const TargetInfo& target, sdk::CUserCmd* cmd);
    float GetHitchance(const TargetInfo& target, sdk::CUserCmd* cmd);
    
    // Multipoint
    std::vector<AimPoint> GenerateMultipoints(sdk::CBaseEntity* entity, int hitbox);
    std::vector<sdk::Vector3D> GetHitboxPoints(sdk::CBaseEntity* entity, int hitbox, float scale);
    
    // Autowall
    struct AutowallResult {
        bool hit = false;
        float damage = 0.0f;
        sdk::Vector3D endPos;
        int hitbox = -1;
        float penetrationDistance = 0.0f;
    };
    AutowallResult TraceAutowall(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target, sdk::CBaseEntity* ignore = nullptr);
    float CalculateDamage(const sdk::Vector3D& start, const sdk::Vector3D& end, sdk::CBaseEntity* target);
    bool TraceToExit(const sdk::Vector3D& start, const sdk::Vector3D& dir, sdk::Vector3D& end, float maxDistance = 90.0f);
    
    // RCS (Recoil Control System)
    sdk::QAngle GetRecoilAngles();
    void ApplyRCS(sdk::CUserCmd* cmd, const sdk::QAngle& aimAngles);
    
    // Auto stop
    void AutoStop(sdk::CUserCmd* cmd);
    bool ShouldAutoStop(sdk::CUserCmd* cmd);
    
    // Auto scope
    void AutoScope(sdk::CUserCmd* cmd);
    
    // Silent aim
    void FixMovement(sdk::CUserCmd* cmd, const sdk::QAngle& originalAngles);
    
    // Utility
    sdk::CBaseEntity* GetLocalPlayer() const;
    sdk::CBaseWeapon* GetActiveWeapon() const;
    bool IsWeaponValid(sdk::CBaseWeapon* weapon) const;
    bool IsAbleToShoot(sdk::CUserCmd* cmd) const;
    
    // Target sorting
    float GetTargetPriority(sdk::CBaseEntity* entity);
    void SortTargets(std::vector<TargetInfo>& targets);

private:
    RageAimbot() = default;
    ~RageAimbot() = default;
    
    RageAimbotConfig m_config;
    std::mutex m_mutex;
    
    // State
    sdk::QAngle m_lastAimAngles;
    bool m_wasFiring = false;
    int m_lastTargetIndex = -1;
    int m_shotsFired = 0;
    int m_shotsHit = 0;
    int m_shotsMissed = 0;
    
    // Config keys
    static constexpr const char* CONFIG_KEY = "aimbot_rage";
    
    void LoadConfigValue(const char* key, auto& value, const auto& defaultValue) {
        auto opt = core::config::ConfigManager::Instance().Get(CONFIG_KEY, key, defaultValue);
        if (opt) value = *opt;
    }
    
    template<typename T>
    void SaveConfigValue(const char* key, const T& value) {
        core::config::ConfigManager::Instance().Set(CONFIG_KEY, key, value);
    }
};

} // namespace features::rage