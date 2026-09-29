#pragma once

#include <cstdint>

namespace sdk::virtual_indices {

// IBaseClientDLL
enum IBaseClientDLL : uint32_t {
    Client_Connect = 0,
    Client_Disconnect = 1,
    Client_Init = 2,
    Client_PostInit = 3,
    Client_Shutdown = 4,
    Client_LevelInitPreEntity = 5,
    Client_LevelInitPostEntity = 6,
    Client_LevelShutdown = 7,
    Client_GetAllClasses = 8,
    Client_INetChannelInfo = 9,
    Client_CreateCmd = 10,
    Client_WriteCmd = 11,
    Client_ReadCmd = 12,
    Client_ProcessCmd = 13,
    Client_GetLocalPlayer = 17,
    Client_GetViewModelFOV = 18,
    Client_CreateMove = 21,
    Client_FrameStageNotify = 36,
    Client_DispatchSound = 38,
    Client_OnChatMessage = 39
};

// IVEngineClient
enum IVEngineClient : uint32_t {
    Engine_GetScreenSize = 5,
    Engine_GetPlayerInfo = 8,
    Engine_GetPlayerForUserID = 9,
    Engine_GetLocalPlayer = 12,
    Engine_GetViewAngles = 18,
    Engine_SetViewAngles = 19,
    Engine_GetMaxClients = 20,
    Engine_IsInGame = 26,
    Engine_IsConnected = 27,
    Engine_GetGameDirectory = 35,
    Engine_GetLevelName = 53,
    Engine_GetLevelNameShort = 54,
    Engine_ExecuteClientCmd = 108,
    Engine_ClientCmd_Unrestricted = 114,
    Engine_GetNetChannelInfo = 78,
    Engine_IsTakingScreenshot = 92
};

// IClientEntityList
enum IClientEntityList : uint32_t {
    EntityList_GetClientEntity = 3,
    EntityList_GetClientEntityFromHandle = 4,
    EntityList_GetHighestEntityIndex = 6,
    EntityList_GetClientNetworkable = 10
};

// IClientMode
enum IClientMode : uint32_t {
    Mode_CreateMove = 21,
    Mode_OverrideView = 18,
    Mode_GetViewModelFOV = 35,
    Mode_ShouldDrawViewModel = 27,
    Mode_DoPostScreenEffects = 43,
    Mode_StartMessageMode = 39
};

// IVModelRender
enum IVModelRender : uint32_t {
    ModelRender_ForcedMaterialOverride = 1,
    ModelRender_DrawModelExecute = 21
};

// IVRenderView
enum IVRenderView : uint32_t {
    RenderView_BeginScene = 5,
    RenderView_SceneEnd = 6
};

// IMaterialSystem
enum IMaterialSystem : uint32_t {
    MatSys_CreateMaterial = 83,
    MatSys_FindMaterial = 84,
    MatSys_FirstMaterial = 86,
    MatSys_NextMaterial = 87,
    MatSys_InvalidMaterial = 88,
    MatSys_GetMaterial = 89,
    MatSys_GetNumMaterials = 90
};

// IMaterial
enum IMaterial : uint32_t {
    Material_GetName = 0,
    Material_GetTextureGroupName = 1,
    Material_IncrementReferenceCount = 12,
    Material_DecrementReferenceCount = 13,
    Material_AlphaModulate = 27,
    Material_ColorModulate = 28,
    Material_IsError = 60,
    Material_Refresh = 61,
    Material_IsTranslucent = 62,
    Material_IsAlphaTested = 63
};

// IVModelInfo
enum IVModelInfo : uint32_t {
    ModelInfo_GetModelName = 3,
    ModelInfo_GetStudioModel = 30,
    ModelInfo_GetModelIndex = 2,
    ModelInfo_GetModel = 1
};

// IEngineTrace
enum IEngineTrace : uint32_t {
    Trace_GetPointContents = 0,
    Trace_GetPointContentsWorldOnly = 1,
    Trace_GetPointContentsCollideable = 2,
    Trace_TraceRay = 5
};

// IVDebugOverlay
enum IVDebugOverlay : uint32_t {
    Debug_AddBoxOverlay = 1,
    Debug_AddTextOverlay = 5,
    Debug_AddLineOverlay = 9,
    Debug_ScreenPosition = 13
};

// IInput
enum IInput : uint32_t {
    Input_GetUserCmd = 8,
    Input_GetUserCmdByIndex = 10,
    Input_IsButtonDown = 15,
    Input_GetButtonState = 16,
    Input_GetMouseDelta = 17,
    Input_GetKeyState = 18
};

// IInputSystem
enum IInputSystem : uint32_t {
    InputSys_EnableInput = 11,
    InputSys_DisableInput = 12,
    InputSys_IsButtonDown = 15,
    InputSys_SetMousePosition = 32,
    InputSys_GetMousePosition = 33,
    InputSys_SetCursorVisible = 35,
    InputSys_IsCursorVisible = 36
};

// IPrediction
enum IPrediction : uint32_t {
    Prediction_Update = 3,
    Prediction_RunCommand = 19,
    Prediction_SetupMove = 20,
    Prediction_FinishMove = 21,
    Prediction_InPrediction = 14
};

// IGameMovement
enum IGameMovement : uint32_t {
    GameMovement_ProcessMovement = 1,
    GameMovement_StartTrackPredictionErrors = 3,
    GameMovement_FinishTrackPredictionErrors = 4,
    GameMovement_Reset = 5
};

// ICvar
enum ICvar : uint32_t {
    Cvar_RegisterConCommand = 9,
    Cvar_UnregisterConCommand = 10,
    Cvar_FindVar = 15,
    Cvar_ConsoleColorPrintf = 25,
    Cvar_ConsoleDPrintf = 26
};

// IGameEventManager2
enum IGameEventManager2 : uint32_t {
    GameEvents_AddListener = 3,
    GameEvents_RemoveListener = 4,
    GameEvents_FireEvent = 8,
    GameEvents_FireEventClientSide = 9,
    GameEvents_GetEventDebugID = 12
};

// IPhysicsSurfaceProps
enum IPhysicsSurfaceProps : uint32_t {
    Physics_GetSurfaceData = 5,
    Physics_GetSurfaceIndex = 10
};

// IEngineSound
enum IEngineSound : uint32_t {
    EngineSound_GetActiveSounds = 19,
    EngineSound_EmitAmbientSound = 21,
    EngineSound_StopAllSounds = 22
};

// ILocalize
enum ILocalize : uint32_t {
    Localize_Find = 12,
    Localize_FindSafe = 13,
    Localize_ConvertANSIToUnicode = 15
};

// CGlobalVarsBase offsets
enum CGlobalVarsBase : uint32_t {
    GlobalVars_RealTime = 0,
    GlobalVars_FrameCount = 1,
    GlobalVars_AbsoluteFrameTime = 2,
    GlobalVars_CurrentTime = 3,
    GlobalVars_FrameTime = 4,
    GlobalVars_MaxClients = 5,
    GlobalVars_TickCount = 6,
    GlobalVars_IntervalPerTick = 7,
    GlobalVars_InterpolationAmount = 8,
    GlobalVars_SimulationTicksThisFrame = 9,
    GlobalVars_NetworkProtocol = 10
};

} // namespace sdk::virtual_indices