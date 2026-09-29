#pragma once

#include <vector>
#include <array>
#include <optional>
#include <string>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"
#include "../../core/logger/logger.hpp"

namespace features::rage {

struct AntiAimConfig {
    bool enabled = true;
    std::string keybind;
    enum class KeybindMode { Toggle, Hold } keybindMode = KeybindMode::Toggle;
    
    // Pitch
    enum class Pitch { Off, Down, Up, Jitter, Zero, Custom } pitch = Pitch::Off;
    float pitchCustom = 0.0f;
    
    // Yaw base
    enum class YawBase { LocalView, AtTargets, Freestanding } yawBase = YawBase::LocalView;
    
    // Yaw
    enum class Yaw { Off, Backward, Jitter, Spin, Static, Custom, AtTargets, Freestanding } yaw = Yaw::Off;
    float yawStatic = 0.0f;
    float yawJitterRange = 30.0f;
    float yawSpinSpeed = 50.0f;
    bool atTargets = false;
    float atTargetsDistance = 0.0f;
    
    // LBY
    enum class LBYMode { Off, Opposite, Jitter, Break } lbyMode = LBYMode::Off;
    float lbyJitterRange = 60.0f;
    bool lbyBreaker = true;
    
    // Desync
    enum class Desync { Off, Static, Jitter, Adaptive, AvoidOverlap } desync = Desync::Jitter;
    float desyncAmount = 58.0f;
    
    // Freestanding
    bool freestanding = false;
    float freestandingRange = 180.0f;
    
    // Edge
    bool edgeAntiAim = false;
    float edgeDistance = 50.0f;
    
    // Fakelag
    bool fakelag = true;
    int fakelagLimit = 14;
    enum class FakelagMode { Static, Random, Adaptive, BreakLC, OnPeek } fakelagMode = FakelagMode::Adaptive;
    int fakelagVariance = 0;
    bool fakelagWhileShooting = false;
    bool fakelagOnKey = false;
    std::string fakelagKey;
    
    // Manual AA
    bool manualAA = false;
    std::string manualLeftKey;
    std::string manualRightKey;
    std::string manualBackKey;
};

class AntiAim {
public:
    static AntiAim& Instance() {
        static AntiAim instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    
    AntiAimConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    void DoFreestanding(sdk::CUserCmd* cmd);
    void DoEdgeAntiAim(sdk::CUserCmd* cmd);
    void DoDesync(sdk::CUserCmd* cmd);
    void DoFakelag(sdk::CUserCmd* cmd, bool& sendPacket);
    
    float GetDesyncAngle() const { return m_desyncAngle; }
    bool IsDesyncReady() const { return m_desyncReady; }

private:
    AntiAim() = default;
    AntiAimConfig m_config;
    
    bool m_lastKeyState = false;
    sdk::QAngle m_lastAngles;
    float m_desyncAngle = 0.0f;
    bool m_desyncReady = false;
    int m_fakelagTicks = 0;
    int m_fakelagLimit = 14;
    bool m_sendPacket = true;
    
    static constexpr const char* CONFIG_KEY = "rage_antiaim";
    
    bool CheckKeybind();
float GetBestDesyncAngle(sdk::CUserCmd* cmd);
    void UpdateFakelag(sdk::CUserCmd* cmd, bool& sendPacket);
    float GetFreestandingYaw();
    float GetAtTargetsYaw();
    void CorrectMovement(sdk::CUserCmd* cmd, const sdk::QAngle& oldAngles);
    sdk::CBaseEntity* GetLocalPlayer() const;
    
    // Implementation methods
    void ApplyDesync(sdk::CUserCmd* cmd);
    void ApplyLBYBreaker(sdk::CUserCmd* cmd);
    void ApplyFreestanding(sdk::CUserCmd* cmd);
    void ApplyManualAA(sdk::CUserCmd* cmd);
};

} // namespace features::rage