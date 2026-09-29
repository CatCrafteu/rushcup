#pragma once

#include <vector>
#include <optional>
#include <array>
#include <memory>
#include <mutex>
#include <deque>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"

namespace features::legit {

struct TriggerbotConfig {
    bool enabled = false;
    std::string keybind;
    enum class KeybindMode { Toggle, Hold, DoubleTap } keybindMode = KeybindMode::Hold;
    int delayMin = 0;
    int delayMax = 50;
    int burstShots = 1;
    int burstDelay = 0;
    std::vector<int> hitgroups = {1, 2, 3}; // Head, Chest, Stomach
    bool checkVisible = true;
    bool checkScope = false;
    bool checkFlash = true;
    bool checkSmoke = true;
    bool checkTeammates = false;
    bool magnumRevolver = false;
    float minDamage = 1.0f;
    int magazineCheck = 0;
};

struct BacktrackConfig {
    bool enabled = false;
    int timeLimit = 200; // ms
    bool visualize = false;
    enum class Bone { Head, Neck, Body } bone = Bone::Head;
    enum class LegitMode { OnKey, OnShot, Always } legitMode = LegitMode::OnKey;
    std::string keybind;
};

struct LegitAAConfig {
    bool enabled = false;
    std::string keybind;
    enum class KeybindMode { Toggle, Hold } keybindMode = KeybindMode::Toggle;
    enum class Pitch { Off, Down, Up, Jitter } pitch = Pitch::Off;
    enum class Yaw { Off, Static, Jitter, Freestanding } yaw = Yaw::Off;
    float yawStatic = 0.0f;
    float yawJitterRange = 30.0f;
    bool atTargets = false;
    float atTargetsDistance = 0.0f;
    enum class LBYMode { Off, Opposite, Jitter } lbyMode = LBYMode::Off;
};

struct MovementConfig {
    bool bhopEnabled = false;
    float bhopHitchance = 95.0f;
    int bhopMaxHop = 0;
    bool airStrafe = false;
    bool autoStrafeEnabled = false;
    enum class AutoStrafeMode { Silent, Normal } autoStrafeMode = AutoStrafeMode::Silent;
    float autoStrafeRetrack = 1.0f;
    bool edgeJumpEnabled = false;
    std::string edgeJumpKeybind;
    bool fastDuckEnabled = false;
    bool slowWalkEnabled = false;
    std::string slowWalkKeybind;
    float slowWalkSpeed = 50.0f;
    bool autoPeekEnabled = false;
    std::string autoPeekKeybind;
    bool autoPeekRender = true;
};

struct LegitConfig {
    TriggerbotConfig triggerbot;
    BacktrackConfig backtrack;
    LegitAAConfig legitAA;
    MovementConfig movement;
};

class Triggerbot {
public:
    static Triggerbot& Instance() {
        static Triggerbot instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    
    TriggerbotConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    Triggerbot() = default;
    TriggerbotConfig m_config;
    
    bool m_wasTriggered = false;
    int m_triggerDelay = 0;
    int m_burstShotsFired = 0;
    int m_burstDelayTimer = 0;
    std::chrono::steady_clock::time_point m_lastShotTime;
    
    static constexpr const char* CONFIG_KEY = "aimbot_legit.triggerbot";
    
    bool CheckKeybind();
    bool CheckConditions(sdk::CUserCmd* cmd);
    bool CheckHitgroup(int hitgroup);
    int GetBestHitgroup(sdk::CBaseEntity* target);
    bool IsVisible(sdk::CBaseEntity* target, int hitbox);
    sdk::CBaseEntity* GetLocalPlayer() const;
    sdk::CBaseWeapon* GetActiveWeapon() const;
};

class Backtrack {
public:
    struct BacktrackRecord {
        sdk::Vector3D origin;
        sdk::Vector3D angles;
        sdk::Vector3D velocity;
        sdk::QAngle eyeAngles;
        std::array<sdk::Matrix3x4, 128> boneMatrix;
        float simulationTime = 0.0f;
        int flags = 0;
        bool valid = false;
        int tickCount = 0;
    };
    
    static Backtrack& Instance() {
        static Backtrack instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    
    BacktrackConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    std::optional<BacktrackRecord> GetBestRecord(sdk::CBaseEntity* entity);
    std::vector<BacktrackRecord> GetRecords(sdk::CBaseEntity* entity);
    void ApplyBacktrack(sdk::CUserCmd* cmd, sdk::CBaseEntity* target, const BacktrackRecord& record);
    
    // Visualization
    void RenderBacktrack();

private:
    Backtrack() = default;
    BacktrackConfig m_config;
    
    std::array<std::deque<BacktrackRecord>, 64> m_records;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "aimbot_legit.backtrack";
    static constexpr int MAX_RECORDS = 128;
    static constexpr float MAX_TIME = 1.0f; // 1 second max
    
    void UpdateRecords(int stage);
    void PurgeOldRecords();
    bool IsRecordValid(const BacktrackRecord& record, sdk::CBaseEntity* entity);
    float GetLerpTime();
};

class LegitAA {
public:
    static LegitAA& Instance() {
        static LegitAA instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    
    LegitAAConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    LegitAA() = default;
    LegitAAConfig m_config;
    
    bool m_lastKeyState = false;
    sdk::QAngle m_lastAngles;
    
    static constexpr const char* CONFIG_KEY = "aimbot_legit.legit_aa";
    
    bool CheckKeybind();
    void DoFreestanding(sdk::CUserCmd* cmd);
    void DoEdgeYaw(sdk::CUserCmd* cmd);
    void DoStaticYaw(sdk::CUserCmd* cmd);
    void DoJitterYaw(sdk::CUserCmd* cmd);
    void DoLBY(sdk::CUserCmd* cmd);
    bool IsKeyPressed(const std::string& keybind);
};

class Movement {
public:
    static Movement& Instance() {
        static Movement instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    
    MovementConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    Movement() = default;
    MovementConfig m_config;
    
    sdk::Vector3D m_autoPeekStartPos;
    bool m_wasAutoPeeking = false;
    bool m_lastEdgeJumpState = false;
    bool m_lastSlowWalkState = false;
    
    static constexpr const char* CONFIG_KEY = "aimbot_legit.movement";
    
    void DoBhop(sdk::CUserCmd* cmd);
    void DoAutoStrafe(sdk::CUserCmd* cmd);
    void DoEdgeJump(sdk::CUserCmd* cmd);
    void DoFastDuck(sdk::CUserCmd* cmd);
    void DoSlowWalk(sdk::CUserCmd* cmd);
    void DoAutoPeek(sdk::CUserCmd* cmd);
    bool IsOnGround();
    float GetMaxSpeed();
    bool IsKeyPressed(const std::string& keybind);
    sdk::CBaseEntity* GetLocalPlayer() const;
};

} // namespace features::legit