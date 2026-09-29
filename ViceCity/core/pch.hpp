// Precompiled header for ViceCity
#pragma once

// Windows
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dwrite.h>
#include <wincodec.h>
#include <psapi.h>
#include <versionhelpers.h>

// C++ Standard Library
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <cstdarg>

// Third-party (vendor)
#include <imgui.h>`n#ifndef ImDrawFlags_None`n#define ImDrawFlags_None 0`n#endif
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <MinHook.h>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>

// Project includes
#include "core/sdk/structs.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/sdk/virtual_indices.hpp"
#include "core/memory/pattern_scanner.hpp"
#include "core/hooks/vmt_hook.hpp"
#include "core/hooks/d3d11_hook.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

// Module name constants for logging (global scope so LOG_INFO(Module, ...) works everywhere)
inline constexpr const char* Core = "Core";
inline constexpr const char* Hooks = "Hooks";
inline constexpr const char* Memory = "Memory";
inline constexpr const char* Config = "Config";
inline constexpr const char* GUI = "GUI";
inline constexpr const char* Rage = "Rage";
inline constexpr const char* Legit = "Legit";
inline constexpr const char* Visuals = "Visuals";
inline constexpr const char* Misc = "Misc";
inline constexpr const char* Skinchanger = "Skinchanger";
inline constexpr const char* Lua = "Lua";
inline constexpr const char* Resolver = "Resolver";
inline constexpr const char* Netvars = "Netvars";

// Macros
#define VICECITY_VERSION "1.0.0"
#define VICECITY_NAME "ViceCity"

#ifndef VICECITY_ASSERT
    #ifdef _DEBUG
        #define VICECITY_ASSERT(expr, msg) \
            do { \
                if (!(expr)) { \
                    ::MessageBoxA(nullptr, msg, "ViceCity Assertion Failed", MB_ICONERROR | MB_OK); \
                    __debugbreak(); \
                } \
            } while (0)
    #else
        #define VICECITY_ASSERT(expr, msg) ((void)0)
    #endif
#endif

// Logging macros
#define LOG_TRACE(module, ...) core::logger::Logger::Instance().Trace(module, __VA_ARGS__)
#define LOG_DEBUG(module, ...) core::logger::Logger::Instance().Debug(module, __VA_ARGS__)
#define LOG_INFO(module, ...) core::logger::Logger::Instance().Info(module, __VA_ARGS__)
#define LOG_WARN(module, ...) core::logger::Logger::Instance().Warn(module, __VA_ARGS__)
#define LOG_ERROR(module, ...) core::logger::Logger::Instance().Error(module, __VA_ARGS__)
#define LOG_CRITICAL(module, ...) core::logger::Logger::Instance().Critical(module, __VA_ARGS__)

// Module loggers
#define DECLARE_MODULE_LOGGER(name) \
    inline core::logger::Logger::ModuleLogger g_log_##name(#name)

#define GET_MODULE_LOGGER(name) g_log_##name

// Common modules
DECLARE_MODULE_LOGGER(Core);
DECLARE_MODULE_LOGGER(Hooks);
DECLARE_MODULE_LOGGER(Memory);
DECLARE_MODULE_LOGGER(Config);
DECLARE_MODULE_LOGGER(GUI);
DECLARE_MODULE_LOGGER(Rage);
DECLARE_MODULE_LOGGER(Legit);
DECLARE_MODULE_LOGGER(Visuals);
DECLARE_MODULE_LOGGER(Misc);
DECLARE_MODULE_LOGGER(Skinchanger);
DECLARE_MODULE_LOGGER(Lua);
DECLARE_MODULE_LOGGER(Resolver);
DECLARE_MODULE_LOGGER(Netvars);

// Utility macros
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define OFFSET_OF(type, member) ((size_t)&reinterpret_cast<const volatile char&>((((type*)0)->member)))
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define MAKE_UNIQUE_NAME(base) CONCAT(base, __LINE__)

// Scope guard
template<typename F>
struct ScopeGuard {
    F func;
    bool dismissed = false;
    ~ScopeGuard() { if (!dismissed) func(); }
    void Dismiss() { dismissed = true; }
};

template<typename F>
ScopeGuard<F> MakeScopeGuard(F&& f) {
    return ScopeGuard<std::decay_t<F>>{std::forward<F>(f)};
}

#define SCOPE_EXIT(code) \
    auto CONCAT(scope_guard_, __LINE__) = MakeScopeGuard([&](){ code; })

// Singleton macro
#define SINGLETON(classname) \
public: \
    static classname& Instance() { \
        static classname instance; \
        return instance; \
    } \
private: \
    classname() = default; \
    ~classname() = default; \
    classname(const classname&) = delete; \
    classname& operator=(const classname&) = delete;

// Non-copyable
#define NON_COPYABLE(classname) \
    classname(const classname&) = delete; \
    classname& operator=(const classname&) = delete;

// Non-movable
#define NON_MOVABLE(classname) \
    classname(classname&&) = delete; \
    classname& operator=(classname&&) = delete;

// Non-copyable, non-movable
#define NON_COPYABLE_MOVABLE(classname) \
    NON_COPYABLE(classname) \
    NON_MOVABLE(classname)

// Bit flags
#define HAS_FLAG(flags, flag) (((flags) & (flag)) != 0)
#define SET_FLAG(flags, flag) ((flags) |= (flag))
#define CLEAR_FLAG(flags, flag) ((flags) &= ~(flag))
#define TOGGLE_FLAG(flags, flag) ((flags) ^= (flag))

// Alignment
#define ALIGN_UP(val, alignment) (((val) + (alignment) - 1) & ~((alignment) - 1))
#define ALIGN_DOWN(val, alignment) ((val) & ~((alignment) - 1))

// Min/Max
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

// Clamp
template<typename T>
constexpr T Clamp(T value, T min, T max) {
    return value < min ? min : (value > max ? max : value);
}

// Lerp
template<typename T>
constexpr T Lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

// Color conversion
constexpr uint32_t IM_COL32_R(uint32_t c) { return (c >> 24) & 0xFF; }
constexpr uint32_t IM_COL32_G(uint32_t c) { return (c >> 16) & 0xFF; }
constexpr uint32_t IM_COL32_B(uint32_t c) { return (c >> 8) & 0xFF; }
constexpr uint32_t IM_COL32_A(uint32_t c) { return c & 0xFF; }

// Safe release
template<typename T>
void SafeRelease(T*& ptr) {
    if (ptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

// String utilities
inline std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

inline std::string ToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

inline bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

inline bool EndsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Format helpers
template<typename... Args>
std::string FormatString(const char* fmt, Args&&... args) {
    return fmt::format(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
std::wstring FormatWString(const wchar_t* fmt, Args&&... args) {
    return fmt::format(fmt, std::forward<Args>(args)...);
}