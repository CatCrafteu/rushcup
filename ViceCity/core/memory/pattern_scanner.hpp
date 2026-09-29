#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <Windows.h>
#include "../sdk/structs.hpp"

namespace core::memory {

// Pattern scanner
class PatternScanner {
public:
    static std::optional<uintptr_t> Scan(const char* moduleName, const char* pattern, const char* mask = nullptr);
    static std::optional<uintptr_t> Scan(uintptr_t base, size_t size, const char* pattern, const char* mask = nullptr);
    static std::vector<uintptr_t> ScanAll(const char* moduleName, const char* pattern, const char* mask = nullptr);
    static std::optional<uintptr_t> ScanIDA(const char* moduleName, const char* idaPattern);
    static std::optional<uintptr_t> ScanIDA(uintptr_t base, size_t size, const char* idaPattern);

private:
    static bool PatternCompare(const uint8_t* data, const uint8_t* pattern, const char* mask);
    static std::pair<uintptr_t, size_t> GetModuleInfo(const char* moduleName);
};

// Netvar manager
class NetvarManager {
public:
    struct NetvarInfo {
        uintptr_t offset = 0;
        uintptr_t propPtr = 0;
        std::string name;
        std::string tableName;
    };

    bool Initialize();
    void Shutdown();
    
    std::optional<uintptr_t> GetOffset(const char* tableName, const char* propName);
    std::optional<uintptr_t> GetOffset(const char* propName);
    std::optional<NetvarInfo> GetNetvar(const char* tableName, const char* propName);
    
    void DumpNetvars(const char* outputPath = "netvars.txt");
    void DumpNetvarsRecursive(void* recvTable, int depth = 0);

    static NetvarManager& Instance() {
        static NetvarManager instance;
        return instance;
    }

private:
    struct RecvTable;
    struct RecvProp;
    
    void* GetRecvTable(const char* name);
    std::optional<uintptr_t> GetOffsetRecursive(void* recvTable, const char* propName, uintptr_t baseOffset = 0);
    
    std::unordered_map<std::string, NetvarInfo> m_netvars;
    bool m_initialized = false;
};

// Offset dumper (for SDK generation)
class OffsetDumper {
public:
    struct OffsetEntry {
        std::string name;
        uintptr_t offset;
        std::string type;
    };

    bool Initialize();
    void Dump(const char* outputPath = "offsets.hpp");
    void DumpJSON(const char* outputPath = "offsets.json");
    
    std::vector<OffsetEntry> GetOffsets() const { return m_offsets; }

private:
    std::vector<OffsetEntry> m_offsets;
    void AddOffset(const char* name, uintptr_t offset, const char* type = "uintptr_t");
    void DumpClass(const char* className, void* classPtr);
};

// Memory utilities
class Memory {
public:
    // Module handling
    static uintptr_t GetModuleBase(const char* moduleName);
    static std::pair<uintptr_t, size_t> GetModuleRange(const char* moduleName);
    static bool IsValidPtr(uintptr_t ptr);
    
    // Protection
    static bool Protect(uintptr_t addr, size_t size, DWORD newProtect, DWORD& oldProtect);
    static bool Unprotect(uintptr_t addr, size_t size);
    static bool RestoreProtection(uintptr_t addr, size_t size, DWORD oldProtect);
    
    // Reading/Writing
    template<typename T>
    static std::optional<T> Read(uintptr_t addr) {
        if (!IsValidPtr(addr)) return std::nullopt;
        return *reinterpret_cast<T*>(addr);
    }
    
    template<typename T>
    static bool Write(uintptr_t addr, T value) {
        if (!IsValidPtr(addr)) return false;
        DWORD oldProtect;
        if (!Unprotect(addr, sizeof(T))) return false;
        *reinterpret_cast<T*>(addr) = value;
        RestoreProtection(addr, sizeof(T), oldProtect);
        return true;
    }
    
    template<typename T>
    static bool WriteRaw(uintptr_t addr, const T& value) {
        if (!IsValidPtr(addr)) return false;
        DWORD oldProtect;
        if (!Unprotect(addr, sizeof(T))) return false;
        std::memcpy(reinterpret_cast<void*>(addr), &value, sizeof(T));
        RestoreProtection(addr, sizeof(T), oldProtect);
        return true;
    }
    
    // Pattern scanning wrappers
    static std::optional<uintptr_t> FindPattern(const char* module, const char* pattern, const char* mask = nullptr) {
        return PatternScanner::Scan(module, pattern, mask);
    }
    
    static std::optional<uintptr_t> FindPatternIDA(const char* module, const char* idaPattern) {
        return PatternScanner::ScanIDA(module, idaPattern);
    }
    
    // VTable hooking helpers
    static void** GetVTable(uintptr_t instance) {
        return *reinterpret_cast<void***>(instance);
    }
    
    static uintptr_t GetVFunc(void** vtable, size_t index) {
        return reinterpret_cast<uintptr_t>(vtable[index]);
    }
    
    static bool HookVFunc(void** vtable, size_t index, uintptr_t newFunc, uintptr_t* original = nullptr) {
        if (!vtable) return false;
        DWORD oldProtect;
        if (!Unprotect(reinterpret_cast<uintptr_t>(&vtable[index]), sizeof(uintptr_t))) return false;
        if (original) *original = reinterpret_cast<uintptr_t>(vtable[index]);
        vtable[index] = reinterpret_cast<void*>(newFunc);
        RestoreProtection(reinterpret_cast<uintptr_t>(&vtable[index]), sizeof(uintptr_t), oldProtect);
        return true;
    }
    
    // Relative call/jmp resolution
    static uintptr_t ResolveRelativeCall(uintptr_t instructionAddr) {
        if (!IsValidPtr(instructionAddr)) return 0;
        int32_t offset = *reinterpret_cast<int32_t*>(instructionAddr + 1);
        return instructionAddr + 5 + offset;
    }
    
    static uintptr_t ResolveRelativeJmp(uintptr_t instructionAddr) {
        return ResolveRelativeCall(instructionAddr);
    }
    
    // Follow jumps
    static uintptr_t FollowJumps(uintptr_t addr, int maxDepth = 10) {
        for (int i = 0; i < maxDepth && IsValidPtr(addr); ++i) {
            uint8_t opcode = *reinterpret_cast<uint8_t*>(addr);
            if (opcode == 0xE9 || opcode == 0xE8) { // JMP / CALL rel32
                addr = ResolveRelativeCall(addr);
            } else if (opcode == 0xFF) { // JMP/CALL [reg] or JMP/CALL [mem]
                uint8_t modrm = *reinterpret_cast<uint8_t*>(addr + 1);
                if ((modrm & 0x38) == 0x20 || (modrm & 0x38) == 0x28) { // JMP/CALL [reg+offset]
                    int32_t offset = *reinterpret_cast<int32_t*>(addr + 2);
                    uintptr_t target = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(addr + 2) + offset);
                    addr = target;
                }
            } else {
                break;
            }
        }
        return addr;
    }
    
    // Signature to pattern conversion
    static std::string IDAToPattern(const char* idaSig);
    
    // Byte patching
    static bool PatchBytes(uintptr_t addr, const std::vector<uint8_t>& bytes);
    static bool NopBytes(uintptr_t addr, size_t count);
    static std::vector<uint8_t> GetOriginalBytes(uintptr_t addr, size_t count);
    
    // Module enumeration
    static std::vector<std::pair<std::string, uintptr_t>> GetLoadedModules();
    
    // Export resolution
    static uintptr_t GetExport(uintptr_t moduleBase, const char* exportName);
    static uintptr_t GetExport(const char* moduleName, const char* exportName);
};

// Signature definitions (auto-updated by offset dumper)
namespace signatures {
    // Client.dll
    inline std::optional<uintptr_t> g_pClientMode;
    inline std::optional<uintptr_t> g_pInput;
    inline std::optional<uintptr_t> g_pGlobalVars;
    inline std::optional<uintptr_t> g_pMoveHelper;
    inline std::optional<uintptr_t> g_pPredictionSeed;
    inline std::optional<uintptr_t> g_pPredictionPlayer;
    
    // Engine.dll
    inline std::optional<uintptr_t> g_pEngineTrace;
    inline std::optional<uintptr_t> g_pDebugOverlay;
    inline std::optional<uintptr_t> g_pModelInfo;
    inline std::optional<uintptr_t> g_pModelRender;
    inline std::optional<uintptr_t> g_pRenderView;
    inline std::optional<uintptr_t> g_pMaterialSystem;
    inline std::optional<uintptr_t> g_pStudioRender;
    
    // Functions
    inline std::optional<uintptr_t> CreateMove;
    inline std::optional<uintptr_t> FrameStageNotify;
    inline std::optional<uintptr_t> DrawModelExecute;
    inline std::optional<uintptr_t> Present;
    inline std::optional<uintptr_t> ResizeBuffers;
    inline std::optional<uintptr_t> FireEventClientSide;
    inline std::optional<uintptr_t> DispatchSound;
    inline std::optional<uintptr_t> OverrideView;
    inline std::optional<uintptr_t> GetViewModelFOV;
    inline std::optional<uintptr_t> DoPostScreenEffects;
    inline std::optional<uintptr_t> RunCommand;
    inline std::optional<uintptr_t> SetupMove;
    inline std::optional<uintptr_t> FinishMove;
    inline std::optional<uintptr_t> ProcessMovement;
    inline std::optional<uintptr_t> TraceRay;
    inline std::optional<uintptr_t> AddBoxOverlay;
    inline std::optional<uintptr_t> WorldToScreen;
    inline std::optional<uintptr_t> GetScreenSize;
    inline std::optional<uintptr_t> GetPlayerInfo;
    inline std::optional<uintptr_t> GetLocalPlayer;
    inline std::optional<uintptr_t> GetViewAngles;
    inline std::optional<uintptr_t> SetViewAngles;
    inline std::optional<uintptr_t> ExecuteClientCmd;
    inline std::optional<uintptr_t> ClientCmd_Unrestricted;
    inline std::optional<uintptr_t> IsConnected;
    inline std::optional<uintptr_t> IsInGame;
    inline std::optional<uintptr_t> GetMaxClients;
    inline std::optional<uintptr_t> GetLevelName;
    inline std::optional<uintptr_t> GetGameDirectory;
    inline std::optional<uintptr_t> GetProductVersionString;
    inline std::optional<uintptr_t> GetNetChannelInfo;
    inline std::optional<uintptr_t> IsTakingScreenshot;
    inline std::optional<uintptr_t> GetColorModulation;
    inline std::optional<uintptr_t> SetColorModulation;
    inline std::optional<uintptr_t> GetAlphaModulation;
    inline std::optional<uintptr_t> SetAlphaModulation;
    inline std::optional<uintptr_t> ForcedMaterialOverride;
    inline std::optional<uintptr_t> DrawModelExecuteEx;
    inline std::optional<uintptr_t> SceneEnd;
    inline std::optional<uintptr_t> BeginScene;
    inline std::optional<uintptr_t> FindMaterial;
    inline std::optional<uintptr_t> CreateMaterial;
    inline std::optional<uintptr_t> GetStudioModel;
    inline std::optional<uintptr_t> GetModelName;
    inline std::optional<uintptr_t> GetModelIndex;
    inline std::optional<uintptr_t> GetModel;
    inline std::optional<uintptr_t> GetModelMaterials;
    inline std::optional<uintptr_t> GetModelRenderBounds;
    inline std::optional<uintptr_t> GetPointContents;
    inline std::optional<uintptr_t> GetPointContentsWorldOnly;
    inline std::optional<uintptr_t> GetPointContents_Collideable;
    inline std::optional<uintptr_t> ClipRayCollideToEntity;
    inline std::optional<uintptr_t> AddLineOverlay;
    inline std::optional<uintptr_t> AddTextOverlay;
    inline std::optional<uintptr_t> AddTriangleOverlay;
    inline std::optional<uintptr_t> AddSweptBoxOverlay;
    inline std::optional<uintptr_t> ScreenPosition;
    inline std::optional<uintptr_t> GetActiveSounds;
    inline std::optional<uintptr_t> EmitAmbientSound;
    inline std::optional<uintptr_t> StopAllSounds;
    inline std::optional<uintptr_t> StopSound;
    inline std::optional<uintptr_t> FindVar;
    inline std::optional<uintptr_t> RegisterConCommand;
    inline std::optional<uintptr_t> UnregisterConCommand;
    inline std::optional<uintptr_t> RegisterConVar;
    inline std::optional<uintptr_t> UnregisterConVar;
    inline std::optional<uintptr_t> ConsoleColorPrintf;
    inline std::optional<uintptr_t> ConsoleDPrintf;
    inline std::optional<uintptr_t> AddListener;
    inline std::optional<uintptr_t> RemoveListener;
    inline std::optional<uintptr_t> FireEvent;
    inline std::optional<uintptr_t> FireEventClientSideEx;
    inline std::optional<uintptr_t> SerializeEvent;
    inline std::optional<uintptr_t> Update;
    inline std::optional<uintptr_t> GetSurfaceData;
    inline std::optional<uintptr_t> GetSurfaceIndex;
    inline std::optional<uintptr_t> GetSurfaceDataByIndex;
    inline std::optional<uintptr_t> GetClientEntity;
    inline std::optional<uintptr_t> GetClientEntityFromHandle;
    inline std::optional<uintptr_t> GetHighestEntityIndex;
    inline std::optional<uintptr_t> GetClientNetworkable;
    inline std::optional<uintptr_t> GetClientEntityByIndex;
    inline std::optional<uintptr_t> GetAllClasses;
    inline std::optional<uintptr_t> LevelInitPreEntity;
    inline std::optional<uintptr_t> LevelInitPostEntity;
    inline std::optional<uintptr_t> LevelShutdown;
    inline std::optional<uintptr_t> INetChannelInfo;
    inline std::optional<uintptr_t> CreateCmd;
    inline std::optional<uintptr_t> WriteCmd;
    inline std::optional<uintptr_t> ReadCmd;
    inline std::optional<uintptr_t> ProcessCmd;
    inline std::optional<uintptr_t> WriteDelta;
    inline std::optional<uintptr_t> ReadDelta;
    inline std::optional<uintptr_t> GetClientInfo;
    inline std::optional<uintptr_t> GetViewModelFOVEx;
    inline std::optional<uintptr_t> DispatchSoundEx;
    inline std::optional<uintptr_t> OnChatMessage;
}

// Initialize all signatures
bool InitializeSignatures();
void DumpSignatures(const char* path = "signatures.txt");

} // namespace core::memory