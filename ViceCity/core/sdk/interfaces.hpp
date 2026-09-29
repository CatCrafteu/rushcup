#pragma once

#include <cstdint>
#include <string_view>
#include "structs.hpp"

namespace sdk {

// Forward declarations
struct IBaseClientDLL;
struct IVEngineClient;
struct IClientEntityList;
struct IClientMode;
struct IInput;
struct IInputSystem;
struct ICvar;
struct IGameEventManager2;
struct IGameMovement;
struct IPrediction;
struct IMoveHelper;
struct IPhysicsSurfaceProps;
struct IEngineTrace;
struct IVDebugOverlay;
struct IVModelInfo;
struct IVModelRender;
struct IVRenderView;
struct IMaterialSystem;
struct IMaterial;
struct IStudioRender;
struct IEngineSound;
struct ILocalize;
struct IGameUIFuncs;
struct IMatchFramework;
struct IGlobalVars;
struct CGameResourceService;
struct CGameEntitySystem;
struct CEntitySystem;
struct CResourceSystem;
struct CSchemaSystem;
struct CNetworkClient;
struct CNetworkServer;
struct IEngineTool;
struct IEngineVGui;
struct IClientLeafSystem;
struct IClientTools;
struct IClientShadowMgr;
struct IClientSound;
struct IClientVehicle;
struct IClientEntityListTools;
struct IClientNetworkable;
struct IClientRenderable;
struct IClientUnknown;
struct IClientThinkable;
struct IClientAlphaProperty;
struct IClientModelRender;
struct IClientNetworkedVariable;
struct IClientNetworkedVariableManager;
struct IClientNetworkedVariableCallback;
struct IClientNetworkedVariableWatcher;
struct IClientNetworkedVariableProxy;
struct IClientNetworkedVariableDataTable;
struct IClientNetworkedVariableSendTable;
struct IClientNetworkedVariableRecvTable;
struct IClientNetworkedVariableRecvProp;
struct IClientNetworkedVariableSendProp;
struct IClientNetworkedVariableDataMap;
struct IClientNetworkedVariableDataDesc;
struct IClientNetworkedVariableTypedDescription;
struct IClientNetworkedVariableField;
struct IClientNetworkedVariableArray;
struct IClientNetworkedVariableVector;
struct IClientNetworkedVariableQuaternion;
struct IClientNetworkedVariableMatrix;
struct IClientNetworkedVariableTransform;
struct IClientNetworkedVariablePoseParameter;
struct IClientNetworkedVariableBone;
struct IClientNetworkedVariableAttachment;
struct IClientNetworkedVariableSequence;
struct IClientNetworkedVariableActivity;
struct IClientNetworkedVariableCycle;
struct IClientNetworkedVariablePlaybackRate;
struct IClientNetworkedVariableWeight;
struct IClientNetworkedVariableOrder;
struct IClientNetworkedVariableSequenceCount;
struct IClientNetworkedVariableDuration;
struct IClientNetworkedVariableLooping;
struct IClientNetworkedVariableAutoplay;
struct IClientNetworkedVariableFadeIn;
struct IClientNetworkedVariableFadeOut;
struct IClientNetworkedVariableBlendIn;
struct IClientNetworkedVariableBlendOut;
struct IClientNetworkedVariableTransition;
struct IClientNetworkedVariableLayer;
struct IClientNetworkedVariableLayerCount;
struct IClientNetworkedVariableLayerWeight;
struct IClientNetworkedVariableLayerCycle;
struct IClientNetworkedVariableLayerPlaybackRate;
struct IClientNetworkedVariableLayerOrder;
struct IClientNetworkedVariableLayerSequence;
struct IClientNetworkedVariableLayerActivity;
struct IClientNetworkedVariableLayerDuration;
struct IClientNetworkedVariableLayerLooping;
struct IClientNetworkedVariableLayerAutoplay;
struct IClientNetworkedVariableLayerFadeIn;
struct IClientNetworkedVariableLayerFadeOut;
struct IClientNetworkedVariableLayerBlendIn;
struct IClientNetworkedVariableLayerBlendOut;
struct IClientNetworkedVariableLayerTransition;

// Interface pointers (global)
inline IBaseClientDLL* g_pClient = nullptr;
inline IVEngineClient* g_pEngine = nullptr;
inline IClientEntityList* g_pEntityList = nullptr;
inline IClientMode* g_pClientMode = nullptr;
inline IInput* g_pInput = nullptr;
inline IInputSystem* g_pInputSystem = nullptr;
inline ICvar* g_pCvar = nullptr;
inline IGameEventManager2* g_pGameEvents = nullptr;
inline IGameMovement* g_pGameMovement = nullptr;
inline IPrediction* g_pPrediction = nullptr;
inline IMoveHelper* g_pMoveHelper = nullptr;
inline IPhysicsSurfaceProps* g_pPhysicsProps = nullptr;
inline IEngineTrace* g_pEngineTrace = nullptr;
inline IVDebugOverlay* g_pDebugOverlay = nullptr;
inline IVModelInfo* g_pModelInfo = nullptr;
inline IVModelRender* g_pModelRender = nullptr;
inline IVRenderView* g_pRenderView = nullptr;
inline IMaterialSystem* g_pMaterialSystem = nullptr;
inline IStudioRender* g_pStudioRender = nullptr;
inline IEngineSound* g_pEngineSound = nullptr;
inline ILocalize* g_pLocalize = nullptr;
inline IGlobalVars* g_pGlobalVars = nullptr;

// Interface definitions
struct IBaseClientDLL {
    virtual int Connect(void* appSystemFactory) = 0;
    virtual int Disconnect() = 0;
    virtual int Init(void* appSystemFactory, void* physicsFactory, void* fileSystemFactory) = 0;
    virtual void PostInit() = 0;
    virtual void Shutdown() = 0;
    virtual void LevelInitPreEntity(const char* mapName) = 0;
    virtual void LevelInitPostEntity() = 0;
    virtual void LevelShutdown() = 0;
    virtual void* GetAllClasses() = 0;
    virtual void* INetChannelInfo() = 0;
    virtual void CreateCmd(int slot, void* cmd) = 0;
    virtual void WriteCmd(void* buf, void* from, void* to) = 0;
    virtual void ReadCmd(void* buf, void* cmd) = 0;
    virtual void ProcessCmd(void* cmd) = 0;
    virtual void WriteDelta(void* buf, void* from, void* to) = 0;
    virtual void ReadDelta(void* buf, void* from, void* to) = 0;
    virtual void GetClientInfo(int slot, void* info) = 0;
    virtual int GetLocalPlayer() = 0;
    virtual int GetViewModelFOV() = 0;
    virtual void CreateMove(int sequenceNumber, float inputSampleTime, bool active) = 0;
    virtual void FrameStageNotify(int stage) = 0;
    virtual void DispatchSound(void* sound) = 0;
    virtual void OnChatMessage(void* msg) = 0;
};

struct IVEngineClient {
    virtual int GetScreenSize(int& w, int& h) = 0;
    virtual void GetPlayerInfo(int entNum, void* info) = 0;
    virtual int GetPlayerForUserID(int userID) = 0;
    virtual int GetLocalPlayer() = 0;
    virtual void GetViewAngles(QAngle& va) = 0;
    virtual void SetViewAngles(const QAngle& va) = 0;
    virtual int GetMaxClients() = 0;
    virtual bool IsInGame() = 0;
    virtual bool IsConnected() = 0;
    virtual const char* GetGameDirectory() = 0;
    virtual const char* GetLevelName() = 0;
    virtual const char* GetLevelNameShort() = 0;
    virtual const char* GetProductVersionString() = 0;
    virtual void ExecuteClientCmd(const char* cmd) = 0;
    virtual void ClientCmd_Unrestricted(const char* cmd) = 0;
    virtual void* GetNetChannelInfo() = 0;
    virtual bool IsTakingScreenshot() = 0;
    virtual bool IsPlayingDemo() = 0;
    virtual bool IsPlayingTimeDemo() = 0;
    virtual void GetDemoPlaybackParameters(float* time, float* frames, int* ticks) = 0;
    virtual void* GetViewModel(int index) = 0;
    virtual void* GetViewModelByIndex(int index) = 0;
    virtual int GetViewModelCount() = 0;
    virtual void* GetViewModelByName(const char* name) = 0;
};

struct IClientEntityList {
    virtual void* GetClientEntity(int entNum) = 0;
    virtual void* GetClientEntityFromHandle(uintptr_t handle) = 0;
    virtual int GetHighestEntityIndex() = 0;
    virtual void* GetClientNetworkable(int entNum) = 0;
    virtual void* GetClientEntityByIndex(int entNum) = 0;
    virtual int GetClientEntityCount() = 0;
    virtual void* GetClientEntityByHandle(uintptr_t handle) = 0;
};

struct IClientMode {
    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Enable() = 0;
    virtual void Disable() = 0;
    virtual void Layout() = 0;
    virtual void CreateMove(float inputSampleTime, void* cmd) = 0;
    virtual bool OverrideView(void* setup) = 0;
    virtual int GetViewModelFOV() = 0;
    virtual bool ShouldDrawViewModel() = 0;
    virtual void DoPostScreenEffects(void* setup) = 0;
    virtual int GetKeyForBinding(const char* binding) = 0;
    virtual void StartMessageMode(int mode) = 0;
    virtual void GetHudBounds(int& x, int& y, int& w, int& h) = 0;
};

struct IInput {
    virtual void* GetUserCmd(int slot, int sequenceNumber) = 0;
    virtual void* GetUserCmd2(int slot, int sequenceNumber) = 0;
    virtual void* GetUserCmdByIndex(int slot, int index) = 0;
    virtual void* GetUserCmdByIndex2(int slot, int index) = 0;
    virtual bool IsButtonDown(int code) = 0;
    virtual int GetButtonState(int code) = 0;
    virtual void GetMouseDelta(int& x, int& y) = 0;
    virtual int GetKeyState(int code) = 0;
    virtual bool GetKeyPressed(int code) = 0;
    virtual bool GetKeyReleased(int code) = 0;
    virtual bool GetKeyToggled(int code) = 0;
    virtual int GetKeyRepeatCount(int code) = 0;
    virtual bool IsKeyDown(int code) = 0;
    virtual bool IsKeyPressed(int code) = 0;
    virtual bool IsKeyReleased(int code) = 0;
    virtual bool IsKeyToggled(int code) = 0;
};

struct IInputSystem {
    virtual void EnableInput(bool enable) = 0;
    virtual void DisableInput(bool disable) = 0;
    virtual bool IsButtonDown(int code) = 0;
    virtual int GetButtonState(int code) = 0;
    virtual void GetMouseDelta(int& x, int& y) = 0;
    virtual int GetKeyState(int code) = 0;
    virtual bool GetKeyPressed(int code) = 0;
    virtual bool GetKeyReleased(int code) = 0;
    virtual bool GetKeyToggled(int code) = 0;
    virtual int GetKeyRepeatCount(int code) = 0;
    virtual void SetMousePosition(int x, int y) = 0;
    virtual void GetMousePosition(int& x, int& y) = 0;
    virtual void* GetWindowID() = 0;
    virtual void SetCursorVisible(bool visible) = 0;
    virtual bool IsCursorVisible() = 0;
    virtual void GetCursorPosition(int& x, int& y) = 0;
};

struct ICvar {
    virtual void* FindVar(const char* name) = 0;
    virtual void RegisterConCommand(void* cmd) = 0;
    virtual void UnregisterConCommand(void* cmd) = 0;
    virtual void RegisterConVar(void* var) = 0;
    virtual void UnregisterConVar(void* var) = 0;
    virtual void* GetCommands() = 0;
    virtual int GetCommandCount() = 0;
    virtual void ConsoleColorPrintf(const Color& clr, const char* fmt, ...) = 0;
    virtual void ConsoleDPrintf(const char* fmt, ...) = 0;
    virtual void* GetConVar(const char* name) = 0;
    virtual void* FindCommand(const char* name) = 0;
};

struct IGameEventManager2 {
    virtual void AddListener(void* listener, const char* name, bool serverSide) = 0;
    virtual void RemoveListener(void* listener) = 0;
    virtual bool FireEvent(void* event, bool async) = 0;
    virtual bool FireEventClientSide(void* event) = 0;
    virtual void* FindListener(void* listener) = 0;
    virtual int GetEventDebugID() = 0;
    virtual void SerializeEvent(void* event, void* buf) = 0;
    virtual void* CreateEvent(const char* name, bool force) = 0;
    virtual void FreeEvent(void* event) = 0;
};

struct IGameMovement {
    virtual void ProcessMovement(void* player, void* move) = 0;
    virtual void StartTrackPredictionErrors(void* player) = 0;
    virtual void FinishTrackPredictionErrors(void* player) = 0;
    virtual void GetPlayerMins(bool ducked, void* mins, void* maxs) = 0;
    virtual void GetPlayerMaxs(bool ducked, void* mins, void* maxs) = 0;
    virtual void GetPlayerViewOffset(bool ducked, void* offset) = 0;
    virtual void OnLandOnGround(void* player) = 0;
    virtual void SetAbsOrigin(void* player, const Vector3D& origin) = 0;
};

struct IPrediction {
    virtual void Update(int startFrame, bool validFrame, int incomingAcknowledged, int outgoingCommand) = 0;
    virtual void SetupMove(void* player, void* cmd, void* helper, void* move) = 0;
    virtual void FinishMove(void* player, void* cmd, void* move) = 0;
    virtual void RunCommand(void* player, void* cmd, void* helper) = 0;
    virtual void CheckMovingGround(void* player, float frameTime) = 0;
    virtual bool InPrediction() = 0;
};

struct IMoveHelper {
    virtual void SetHost(void* host) = 0;
    virtual void ProcessImpacts() = 0;
    virtual void* GetHost() = 0;
    virtual void CreateMove(float flInputSampleTime, void* cmd) = 0;
};

struct IPhysicsSurfaceProps {
    virtual void* GetSurfaceData(int surfaceDataIndex) = 0;
    virtual int GetSurfaceIndex(const char* surfacePropName) = 0;
    virtual void GetPhysicsProperties(int surfaceDataIndex, float& density, float& thickness, float& friction, float& elasticity) = 0;
    virtual void* GetSurfaceDataByIndex(int index) = 0;
};

struct IEngineTrace {
    virtual int GetPointContents(const Vector3D& pos, int mask, void* entity) = 0;
    virtual int GetPointContentsWorldOnly(const Vector3D& pos, int mask) = 0;
    virtual int GetPointContents_Collideable(void* collide, const Vector3D& pos) = 0;
    virtual void TraceRay(const Ray_t& ray, unsigned int mask, void* filter, Trace_t* trace) = 0;
    virtual void ClipRayCollideToEntity(const Ray_t& ray, unsigned int mask, void* entity, Trace_t* trace) = 0;
    virtual void TraceRayEx(const Ray_t& ray, unsigned int mask, void* filter, Trace_t* trace) = 0;
};

struct IVDebugOverlay {
    virtual void AddBoxOverlay(const Vector3D& origin, const Vector3D& mins, const Vector3D& maxs, const QAngle& angles, int r, int g, int b, int a, float duration) = 0;
    virtual void AddTextOverlay(const Vector3D& origin, float duration, const char* text) = 0;
    virtual void AddLineOverlay(const Vector3D& start, const Vector3D& end, int r, int g, int b, bool noDepthTest, float duration) = 0;
    virtual void AddTriangleOverlay(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual bool ScreenPosition(const Vector3D& world, Vector2D& screen) = 0;
    virtual bool WorldToScreen(const Vector3D& world, Vector2D& screen) = 0;
    virtual void AddSweptBoxOverlay(const Vector3D& start, const Vector3D& end, const Vector3D& mins, const Vector3D& maxs, const QAngle& angles, int r, int g, int b, int a, float duration) = 0;
};

struct IVModelInfo {
    virtual const char* GetModelName(void* model) = 0;
    virtual void* GetStudioModel(void* model) = 0;
    virtual int GetModelIndex(const char* name) = 0;
    virtual void* GetModel(int index) = 0;
    virtual void GetModelMaterials(void* model, int& count, void** materials) = 0;
    virtual void GetModelRenderBounds(void* model, Vector3D& mins, Vector3D& maxs) = 0;
};

struct IVModelRender {
    virtual void DrawModelExecute(void* ctx, void* state, const void* info, const Matrix3x4* boneToWorld) = 0;
    virtual void ForcedMaterialOverride(void* material, int type = 0, int index = 0) = 0;
    virtual bool IsForcedMaterialOverride() = 0;
    virtual void GetColorModulation(float* r, float* g, float* b) = 0;
    virtual void SetColorModulation(const float* r, const float* g, const float* b) = 0;
    virtual void GetAlphaModulation(float* a) = 0;
    virtual void SetAlphaModulation(float a) = 0;
};

struct IVRenderView {
    virtual void GetColorModulation(float* r, float* g, float* b) = 0;
    virtual void SetColorModulation(const float* r, const float* g, const float* b) = 0;
    virtual void GetAlphaModulation(float* a) = 0;
    virtual void SetAlphaModulation(float a) = 0;
    virtual void SetBlend(float blend) = 0;
    virtual void GetBlend(float* blend) = 0;
    virtual void SceneEnd() = 0;
    virtual void BeginScene() = 0;
};

struct IMaterialSystem {
    virtual void* CreateMaterial(const char* name, void* keyValues) = 0;
    virtual void* FindMaterial(const char* name, const char* textureGroup, bool complain, const char* complainPrefix) = 0;
    virtual void* FirstMaterial() = 0;
    virtual void* NextMaterial(void* handle) = 0;
    virtual void* InvalidMaterial() = 0;
    virtual void* GetMaterial(void* handle) = 0;
    virtual int GetNumMaterials() = 0;
};

struct IMaterial {
    virtual const char* GetName() = 0;
    virtual const char* GetTextureGroupName() = 0;
    virtual void IncrementReferenceCount() = 0;
    virtual void DecrementReferenceCount() = 0;
    virtual void AlphaModulate(float alpha) = 0;
    virtual void ColorModulate(float r, float g, float b) = 0;
    virtual void GetAlphaModulation(float& alpha) = 0;
    virtual void GetColorModulation(float& r, float& g, float& b) = 0;
    virtual void SetShaderValue(const char* name, float x, float y, float z, float w) = 0;
    virtual void GetShaderValue(const char* name, float& x, float& y, float& z, float& w) = 0;
    virtual void SetShaderValueFloat(const char* name, float value) = 0;
    virtual void SetShaderValueVec3(const char* name, const Vector3D& value) = 0;
    virtual void SetShaderValueVec4(const char* name, const Vector4D& value) = 0;
    virtual void SetShaderValueMatrix(const char* name, const Matrix4x4& value) = 0;
    virtual float GetShaderParamFloat(const char* name) = 0;
    virtual void GetShaderParamVec3(const char* name, Vector3D& value) = 0;
    virtual void GetShaderParamVec4(const char* name, Vector4D& value) = 0;
    virtual void GetShaderParamMatrix(const char* name, Matrix4x4& value) = 0;
    virtual int GetNumShaderParams() = 0;
    virtual const char* GetShaderParamName(int index) = 0;
    virtual int GetShaderParamType(int index) = 0;
    virtual void SetFlags(int flags) = 0;
    virtual int GetFlags() = 0;
    virtual bool IsError() = 0;
    virtual void Refresh() = 0;
    virtual bool IsTranslucent() = 0;
    virtual bool IsAlphaTested() = 0;
    virtual bool IsVertexLit() = 0;
    virtual bool NeedsPowerOfTwoFrameBufferTexture() = 0;
    virtual int GetMappingWidth() = 0;
    virtual int GetMappingHeight() = 0;
    virtual void* GetShader() = 0;
    virtual int GetShaderVertexSize() = 0;
    virtual int GetShaderVertexFormat() = 0;
};

struct IStudioRender {
    virtual void DrawModel(void* ctx, void* state, const void* info, const Matrix3x4* boneToWorld) = 0;
    virtual void DrawModels(void* ctx, void* state, const void* info, const Matrix3x4* boneToWorld, int count) = 0;
    virtual void ForcedMaterialOverride(void* material) = 0;
    virtual void SetColorModulation(const float* color) = 0;
    virtual void SetAlphaModulation(float alpha) = 0;
    virtual void SetAmbientLightColors(const float* color) = 0;
    virtual void SetLocalLights(int count, void* lights) = 0;
    virtual void SetViewTarget(void* target) = 0;
    virtual void SetEyePosition(const Vector3D& pos) = 0;
    virtual void SetSkin(int skin) = 0;
    virtual void SetBodyGroup(int group, int value) = 0;
    virtual void SetHitboxSet(int set) = 0;
    virtual void SetRenderFlags(int flags) = 0;
    virtual void SetForceFaceCull(int cull) = 0;
    virtual void SetMaterialOverride(void* material) = 0;
    virtual void SetMaterialOverrideDepthWrite(bool depthWrite) = 0;
    virtual void SetColorModulationEx(const float* color, bool isLocalPlayer) = 0;
};

struct IEngineSound {
    virtual void GetActiveSounds(void* sounds) = 0;
    virtual void SetRoomType(int roomType) = 0;
    virtual void EmitAmbientSound(const char* name, float volume, int pitch) = 0;
    virtual void StopAllSounds() = 0;
    virtual void StopSound(int handle) = 0;
};

struct ILocalize {
    virtual const wchar_t* Find(const char* token) = 0;
    virtual const wchar_t* FindSafe(const char* token) = 0;
    virtual void ConvertANSIToUnicode(const char* ansi, wchar_t* unicode, int len) = 0;
    virtual void ConvertUnicodeToANSI(const wchar_t* unicode, char* ansi, int len) = 0;
    virtual void ConvertUnicodeToUTF8(const wchar_t* unicode, char* utf8, int len) = 0;
    virtual void ConvertUTF8ToUnicode(const char* utf8, wchar_t* unicode, int len) = 0;
    virtual int GetCharacterCount(const wchar_t* unicode) = 0;
    virtual void ConvertToUTF8(const wchar_t* unicode, char* utf8, int len) = 0;
    virtual void ConvertFromUTF8(const char* utf8, wchar_t* unicode, int len) = 0;
};

struct IGlobalVars {
    float real_time;
    int frame_count;
    float absolute_frame_time;
    float absolute_frame_start_time_std_dev;
    float current_time;
    float frame_time;
    int max_clients;
    int tick_count;
    float interval_per_tick;
    float interpolation_amount;
    int simulation_ticks_this_frame;
    int network_protocol;
    void* save_data;
    bool client;
    bool remote_client;
    int last_timestamp_networked;
    int timestamp_randomize_window;
};

// Interface initialization
namespace interfaces {
    bool Initialize();
    void Shutdown();
    
    // Helper to get interface from module
    void* GetInterface(const char* moduleName, const char* interfaceName);
    template<typename T>
    T* GetInterface(const char* moduleName, const char* interfaceName) {
        return static_cast<T*>(GetInterface(moduleName, interfaceName));
    }
}

} // namespace sdk