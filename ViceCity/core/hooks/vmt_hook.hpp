#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <Windows.h>

namespace core::hooks {

// VMT Hook class
class VMTHook {
public:
    VMTHook() = default;
    VMTHook(void* instance);
    ~VMTHook();

    bool Initialize(void* instance);
    void Shutdown();
    
    template<typename T>
    T GetOriginal(size_t index) const {
        if (index >= m_originalVTable.size()) return nullptr;
        return reinterpret_cast<T>(m_originalVTable[index]);
    }
    
    template<typename T>
    bool Hook(size_t index, T newFunc) {
        return Hook(index, reinterpret_cast<void*>(newFunc));
    }
    
    bool Hook(size_t index, void* newFunc);
    bool Unhook(size_t index);
    void UnhookAll();
    bool IsHooked(size_t index) const;
    
    size_t GetVTableSize() const { return m_originalVTable.size(); }
    void* GetInstance() const { return m_instance; }

private:
    void* m_instance = nullptr;
    std::vector<void*> m_originalVTable;
    std::unordered_map<size_t, void*> m_hooks;
    mutable std::mutex m_mutex;
};

// MinHook wrapper for detours
class DetourHook {
public:
    DetourHook() = default;
    DetourHook(uintptr_t target, uintptr_t detour);
    ~DetourHook();

    bool Initialize(uintptr_t target, uintptr_t detour);
    void Shutdown();
    
    bool Enable();
    bool Disable();
    bool Toggle();
    
    template<typename T>
    T GetOriginal() const {
        return reinterpret_cast<T>(m_original);
    }
    
    bool IsEnabled() const { return m_enabled; }
    uintptr_t GetTarget() const { return m_target; }
    uintptr_t GetDetour() const { return m_detour; }
    uintptr_t GetOriginalFunc() const { return m_original; }

private:
    uintptr_t m_target = 0;
    uintptr_t m_detour = 0;
    uintptr_t m_original = 0;
    bool m_enabled = false;
    bool m_initialized = false;
};

// Hook manager - manages all hooks
class HookManager {
public:
    static HookManager& Instance() {
        static HookManager instance;
        return instance;
    }
    
    bool Initialize();
    void Shutdown();
    
    // VMT hooks
    std::shared_ptr<VMTHook> CreateVMTHook(void* instance, const char* name = "");
    std::shared_ptr<VMTHook> GetVMTHook(const char* name);
    void RemoveVMTHook(const char* name);
    
    // Detour hooks
    std::shared_ptr<DetourHook> CreateDetour(uintptr_t target, uintptr_t detour, const char* name = "");
    std::shared_ptr<DetourHook> GetDetour(const char* name);
    void RemoveDetour(const char* name);
    
    // Specific hook accessors
    std::shared_ptr<VMTHook> GetClientModeHook() { return GetVMTHook("ClientMode"); }
    std::shared_ptr<VMTHook> GetClientHook() { return GetVMTHook("Client"); }
    std::shared_ptr<VMTHook> GetModelRenderHook() { return GetVMTHook("ModelRender"); }
    std::shared_ptr<VMTHook> GetRenderViewHook() { return GetVMTHook("RenderView"); }
    std::shared_ptr<VMTHook> GetPanelHook() { return GetVMTHook("Panel"); }
    std::shared_ptr<VMTHook> GetSurfaceHook() { return GetVMTHook("Surface"); }
    std::shared_ptr<VMTHook> GetD3D11Hook() { return GetVMTHook("D3D11"); }
    std::shared_ptr<VMTHook> GetEngineHook() { return GetVMTHook("Engine"); }
    std::shared_ptr<VMTHook> GetClientEntityListHook() { return GetVMTHook("ClientEntityList"); }
    std::shared_ptr<VMTHook> GetInputHook() { return GetVMTHook("Input"); }
    std::shared_ptr<VMTHook> GetGameEventManagerHook() { return GetVMTHook("GameEventManager"); }
    
    // Detour accessors
    std::shared_ptr<DetourHook> GetCreateMoveHook() { return GetDetour("CreateMove"); }
    std::shared_ptr<DetourHook> GetPresentHook() { return GetDetour("Present"); }
    std::shared_ptr<DetourHook> GetResizeBuffersHook() { return GetDetour("ResizeBuffers"); }
    std::shared_ptr<DetourHook> GetDrawIndexedPrimitiveHook() { return GetDetour("DrawIndexedPrimitive"); }
    std::shared_ptr<DetourHook> GetFireEventClientSideHook() { return GetDetour("FireEventClientSide"); }
    std::shared_ptr<DetourHook> GetDispatchSoundHook() { return GetDetour("DispatchSound"); }
    std::shared_ptr<DetourHook> GetOverrideViewHook() { return GetDetour("OverrideView"); }
    std::shared_ptr<DetourHook> GetFrameStageNotifyHook() { return GetDetour("FrameStageNotify"); }
    std::shared_ptr<DetourHook> GetRunCommandHook() { return GetDetour("RunCommand"); }
    std::shared_ptr<DetourHook> GetSetupMoveHook() { return GetDetour("SetupMove"); }
    std::shared_ptr<DetourHook> GetFinishMoveHook() { return GetDetour("FinishMove"); }
    std::shared_ptr<DetourHook> GetProcessMovementHook() { return GetDetour("ProcessMovement"); }
    std::shared_ptr<DetourHook> GetTraceRayHook() { return GetDetour("TraceRay"); }
    std::shared_ptr<DetourHook> GetWorldToScreenHook() { return GetDetour("WorldToScreen"); }

private:
    HookManager() = default;
    ~HookManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<VMTHook>> m_vmtHooks;
    std::unordered_map<std::string, std::shared_ptr<DetourHook>> m_detourHooks;
    std::mutex m_mutex;
};

// Macro for creating hook functions
#define HOOK_FN(ret, name, ...) \
    using name##_fn = ret(__fastcall*)(__VA_ARGS__); \
    static name##_fn o##name = nullptr; \
    ret __fastcall hk##name(__VA_ARGS__)

// Hook indices (CS2 specific)
namespace vmt_indices {
    // IClientMode
    constexpr size_t CREATE_MOVE = 21;
    constexpr size_t OVERRIDE_VIEW = 18;
    constexpr size_t GET_VIEWMODEL_FOV = 35;
    constexpr size_t SHOULD_DRAW_VIEWMODEL = 27;
    constexpr size_t DO_POST_SCREEN_EFFECTS = 43;
    
    // IBaseClientDLL
    constexpr size_t FRAME_STAGE_NOTIFY = 36;
    constexpr size_t CREATE_MOVE_CLIENT = 21;
    constexpr size_t DISPATCH_SOUND = 38;
    
    // IVModelRender
    constexpr size_t DRAW_MODEL_EXECUTE = 21;
    constexpr size_t FORCED_MATERIAL_OVERRIDE = 1;
    
    // IVRenderView
    constexpr size_t SCENE_END = 6;
    constexpr size_t BEGIN_SCENE = 5;
    
    // IEngineClient
    constexpr size_t IS_CONNECTED = 27;
    constexpr size_t IS_IN_GAME = 26;
    constexpr size_t GET_LOCAL_PLAYER = 12;
    constexpr size_t GET_VIEW_ANGLES = 18;
    constexpr size_t SET_VIEW_ANGLES = 19;
    constexpr size_t EXECUTE_CLIENT_CMD = 108;
    constexpr size_t GET_SCREEN_SIZE = 5;
    constexpr size_t GET_PLAYER_INFO = 8;
    constexpr size_t GET_MAX_CLIENTS = 20;
    
    // IClientEntityList
    constexpr size_t GET_CLIENT_ENTITY = 3;
    constexpr size_t GET_CLIENT_ENTITY_FROM_HANDLE = 4;
    
    // IGameEventManager2
    constexpr size_t FIRE_EVENT_CLIENT_SIDE = 9;
    
    // IPrediction
    constexpr size_t RUN_COMMAND = 19;
    constexpr size_t SETUP_MOVE = 20;
    constexpr size_t FINISH_MOVE = 21;
    constexpr size_t UPDATE = 3;
    
    // IGameMovement
    constexpr size_t PROCESS_MOVEMENT = 1;
    
    // IEngineTrace
    constexpr size_t TRACE_RAY = 5;
    
    // IVDebugOverlay
    constexpr size_t WORLD_TO_SCREEN = 13;
    constexpr size_t ADD_BOX_OVERLAY = 1;
    constexpr size_t ADD_LINE_OVERLAY = 9;
    constexpr size_t ADD_TEXT_OVERLAY = 5;
    
    // IMaterialSystem
    constexpr size_t FIND_MATERIAL = 84;
    constexpr size_t CREATE_MATERIAL = 83;
    
    // IStudioRender
    constexpr size_t DRAW_MODEL = 29;
}

} // namespace core::hooks
