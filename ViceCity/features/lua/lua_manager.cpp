#include "core/pch.hpp"
#include "features/lua/lua_manager.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"
#include "core/hooks/d3d11_hook.hpp"
#include "gui/menu_manager.hpp"
#include "gui/vice_render.hpp"
#include "gui/vice_fonts.hpp"

namespace features::lua {

void LuaManager::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) return;
    
    // Create scripts directory
    std::filesystem::create_directories(m_scriptsPath);
    std::filesystem::create_directories(m_libsPath);
    
    // Initialize main Lua state
    InitializeLuaState();
    
    // Register APIs
    RegisterAPI();
    
    // Load built-in scripts
    LoadBuiltinScripts();
    
    // Load auto-load scripts
    LoadAutoLoadScripts();
    
    m_initialized = true;
    LOG_INFO(Lua, "LuaManager initialized");
}

void LuaManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) return;
    
    // Call unload callbacks
    OnUnload();
    
    // Unload all scripts
    for (auto& [name, script] : m_scripts) {
        try {
            (*script)["__unload"]();
        } catch (...) {}
    }
    
    m_scripts.clear();
    m_callbacks.clear();
    m_console.clear();
    
    m_initialized = false;
    LOG_INFO(Lua, "LuaManager shutdown");
}

void LuaManager::InitializeLuaState() {
    m_lua.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::table,
        sol::lib::string,
        sol::lib::math,
        sol::lib::coroutine,
        sol::lib::os,
        sol::lib::io,
        sol::lib::utf8
    );
    
    // Set package path
    m_lua["package"]["path"] = m_scriptsPath + "?.lua;" + m_libsPath + "?.lua;" + m_lua["package"]["path"].get<std::string>();
    
    SetupSandbox();
    SetupMemoryLimit();
    SetupTimeout();
    
    // Set panic/error handlers
    m_lua.set_panic(LuaPanicHandler);
    
    // Global print override
    m_lua.set_function("print", [this](const std::string& msg) {
        PrintCallback(msg);
    });
    
    // Global warn/error
    m_lua.set_function("warn", [this](const std::string& msg) {
        WarningCallback(msg);
    });
    
    m_lua.set_function("error", [this](const std::string& msg) {
        ErrorCallback(msg);
    });
}

void LuaManager::SetupSandbox() {
    if (!m_sandboxEnabled) return;
    
    // Remove dangerous functions
    m_lua["os"]["execute"] = sol::lua_nil;
    m_lua["os"]["remove"] = sol::lua_nil;
    m_lua["os"]["rename"] = sol::lua_nil;
    m_lua["os"]["tmpname"] = sol::lua_nil;
    m_lua["io"]["open"] = sol::lua_nil;
    m_lua["io"]["popen"] = sol::lua_nil;
    m_lua["io"]["tmpfile"] = sol::lua_nil;
    m_lua["loadfile"] = sol::lua_nil;
    m_lua["dofile"] = sol::lua_nil;
    m_lua["load"] = sol::lua_nil;
    m_lua["loadstring"] = sol::lua_nil;
    
    // Restrict debug library
    m_lua["debug"] = sol::lua_nil;
}

void LuaManager::SetupMemoryLimit() {
    // Set memory limit via hook - not available in current sol2 version
    // m_lua.set_memory_limit(m_memoryLimit);
}

void LuaManager::SetupTimeout() {
    // Set instruction count hook for timeout - not available in current sol2 version
    // m_lua.set_hook([this](lua_State* L, lua_Debug* ar) {
    //     static int instructionCount = 0;
    //     instructionCount++;
    //     
    //     if (instructionCount > 1000000) { // ~5ms at typical speeds
    //         instructionCount = 0;
    //         // Check timeout
    //         // This is simplified; real implementation would use high-res timer
    //     }
    // }, LUA_MASKCOUNT, 1000);
}

void LuaManager::RegisterAPI() {
    RegisterMenuAPI();
    RegisterEngineAPI();
    RegisterEntityAPI();
    RegisterRenderAPI();
    RegisterCallbackAPI();
    RegisterUtilityAPI();
    RegisterConfigAPI();
    RegisterLoggerAPI();
    RegisterMathAPI();
    RegisterInputAPI();
    RegisterHookAPI();
}

void LuaManager::RegisterMenuAPI() {
    auto menu = m_lua.create_table();
    
    menu["is_open"] = []() -> bool {
        return gui::MenuManager::Instance().IsOpen();
    };
    
    menu["set_open"] = [](bool open) {
        gui::MenuManager::Instance().SetOpen(open);
    };
    
    menu["toggle"] = []() {
        gui::MenuManager::Instance().Toggle();
    };
    
    menu["get_key"] = []() -> int {
        return gui::MenuManager::Instance().GetMenuKey();
    };
    
    menu["set_key"] = [](int key) {
        gui::MenuManager::Instance().SetMenuKey(key);
    };
    
    menu["add_tab"] = [](const std::string& name, sol::function callback) {
        // Custom tab registration (simplified)
        LOG_INFO(Lua, "Custom tab registered: {}", name);
    };
    
    m_lua["menu"] = menu;
}

void LuaManager::RegisterEngineAPI() {
    auto engine = m_lua.create_table();
    
    engine["client_cmd"] = [](const std::string& cmd) {
        sdk::g_pEngine->ExecuteClientCmd(cmd.c_str());
    };
    
    engine["client_cmd_unrestricted"] = [](const std::string& cmd) {
        sdk::g_pEngine->ClientCmd_Unrestricted(cmd.c_str());
    };
    
    engine["get_local_player"] = []() -> int {
        return sdk::g_pEngine->GetLocalPlayer();
    };
    
    engine["get_max_clients"] = []() -> int {
        return sdk::g_pEngine->GetMaxClients();
    };
    
    engine["is_in_game"] = []() -> bool {
        return sdk::g_pEngine->IsInGame();
    };
    
    engine["is_connected"] = []() -> bool {
        return sdk::g_pEngine->IsConnected();
    };
    
    engine["get_level_name"] = []() -> std::string {
        return sdk::g_pEngine->GetLevelName() ? sdk::g_pEngine->GetLevelName() : "";
    };
    
    engine["get_screen_size"] = [=]() -> sol::table {
        int w, h;
        sdk::g_pEngine->GetScreenSize(w, h);
        auto t = m_lua.create_table();
        t["width"] = w;
        t["height"] = h;
        return t;
    };
    
    engine["get_view_angles"] = [=]() -> sol::table {
        sdk::QAngle angles;
        sdk::g_pEngine->GetViewAngles(angles);
        auto t = m_lua.create_table();
        t["pitch"] = angles.pitch;
        t["yaw"] = angles.yaw;
        t["roll"] = angles.roll;
        return t;
    };
    
    engine["set_view_angles"] = [=](const sol::table& angles) {
        sdk::QAngle a;
        a.pitch = angles.get<float>("pitch");
        a.yaw = angles.get<float>("yaw");
        a.roll = angles.get<float>("roll");
        sdk::g_pEngine->SetViewAngles(a);
    };
    
    engine["get_net_channel_info"] = [=]() -> sol::table {
        void* netChan = sdk::g_pEngine->GetNetChannelInfo();
        auto t = m_lua.create_table();
        if (netChan) {
            // Add net channel info
        }
        return t;
    };
    
    m_lua["engine"] = engine;
}

void LuaManager::RegisterEntityAPI() {
    auto entity = m_lua.create_table();
    
    entity["get_local_player"] = [=]() -> sol::object {
        int local = sdk::g_pEngine->GetLocalPlayer();
        if (local > 0) {
            void* ent = sdk::g_pEntityList->GetClientEntity(local);
            return sol::make_object(m_lua, ent);
        }
        return sol::make_object(m_lua, sol::lua_nil);
    };
    
    entity["get_player"] = [=](int index) -> sol::object {
        if (index > 0) {
            void* ent = sdk::g_pEntityList->GetClientEntity(index);
            return sol::make_object(m_lua, ent);
        }
        return sol::make_object(m_lua, sol::lua_nil);
    };
    
    entity["get_player_by_handle"] = [=](uintptr_t handle) -> sol::object {
        void* ent = sdk::g_pEntityList->GetClientEntityFromHandle(handle);
        if (ent) return sol::make_object(m_lua, ent);
        return sol::make_object(m_lua, sol::lua_nil);
    };
    
    entity["get_highest_entity_index"] = [=]() -> int {
        return sdk::g_pEntityList->GetHighestEntityIndex();
    };
    
    entity["get_player_info"] = [=](int index) -> sol::table {
        auto t = m_lua.create_table();
        // Fill player info
        return t;
    };
    
    // Entity methods
    entity["is_valid"] = [=](void* ent) -> bool {
        return ent != nullptr;
    };
    
    entity["get_index"] = [=](void* ent) -> int {
        // Would need actual implementation
        return 0;
    };
    
    entity["get_class_id"] = [=](void* ent) -> int {
        return 0;
    };
    
    entity["get_team"] = [=](void* ent) -> int {
        return 0;
    };
    
    entity["get_health"] = [=](void* ent) -> int {
        return 0;
    };
    
    entity["get_origin"] = [=](void* ent) -> sol::table {
        auto t = m_lua.create_table();
        t["x"] = 0.0f;
        t["y"] = 0.0f;
        t["z"] = 0.0f;
        return t;
    };
    
    entity["get_eye_position"] = [=](void* ent) -> sol::table {
        auto t = m_lua.create_table();
        t["x"] = 0.0f;
        t["y"] = 0.0f;
        t["z"] = 0.0f;
        return t;
    };
    
    entity["get_bone_position"] = [=](void* ent, int bone) -> sol::table {
        auto t = m_lua.create_table();
        t["x"] = 0.0f;
        t["y"] = 0.0f;
        t["z"] = 0.0f;
        return t;
    };
    
    entity["get_velocity"] = [=](void* ent) -> sol::table {
        auto t = m_lua.create_table();
        t["x"] = 0.0f;
        t["y"] = 0.0f;
        t["z"] = 0.0f;
        return t;
    };
    
    entity["is_alive"] = [=](void* ent) -> bool {
        return true;
    };
    
    entity["is_dormant"] = [=](void* ent) -> bool {
        return false;
    };
    
    entity["get_active_weapon"] = [=](void* ent) -> sol::object {
        return sol::make_object(m_lua, sol::lua_nil);
    };
    
    entity["get_weapon"] = [=](void* ent, int index) -> sol::object {
        return sol::make_object(m_lua, sol::lua_nil);
    };
    
    m_lua["entity"] = entity;
}

void LuaManager::RegisterRenderAPI() {
    auto render = m_lua.create_table();
    
    render["world_to_screen"] = [=](const sol::table& world) -> sol::table {
        sdk::Vector3D w;
        w.x = world.get<float>("x");
        w.y = world.get<float>("y");
        w.z = world.get<float>("z");
        
        ImVec2 screen;
        bool success = gui::ViceRender::Instance().WorldToScreen(w, screen);
        
        auto t = m_lua.create_table();
        t["x"] = screen.x;
        t["y"] = screen.y;
        t["valid"] = success;
        return t;
    };
    
    render["draw_line"] = [=](float x1, float y1, float x2, float y2, const sol::table& color, float thickness) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 c = ((static_cast<int>(color["r"]) << 24) | (static_cast<int>(color["g"]) << 16) | (static_cast<int>(color["b"]) << 8) | static_cast<int>(color["a"]));
        drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), c, thickness);
    };
    
    render["draw_rect"] = [=](float x, float y, float w, float h, const sol::table& color, float rounding) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 c = ((static_cast<int>(color["r"]) << 24) | (static_cast<int>(color["g"]) << 16) | (static_cast<int>(color["b"]) << 8) | static_cast<int>(color["a"]));
        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), c, rounding);
    };
    
    render["draw_rect_outline"] = [=](float x, float y, float w, float h, const sol::table& color, float thickness, float rounding) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 c = ((static_cast<int>(color["r"]) << 24) | (static_cast<int>(color["g"]) << 16) | (static_cast<int>(color["b"]) << 8) | static_cast<int>(color["a"]));
        drawList->AddLine(ImVec2(x, y), ImVec2(x + w, y), c, thickness); drawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + h), c, thickness); drawList->AddLine(ImVec2(x + w, y + h), ImVec2(x, y + h), c, thickness); drawList->AddLine(ImVec2(x, y + h), ImVec2(x, y), c, thickness);
    };
    
    render["draw_circle"] = [=](float x, float y, float radius, const sol::table& color, int segments, float thickness) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 c = ((static_cast<int>(color["r"]) << 24) | (static_cast<int>(color["g"]) << 16) | (static_cast<int>(color["b"]) << 8) | static_cast<int>(color["a"]));
        drawList->AddCircle(ImVec2(x, y), radius, c, segments, thickness);
    };
    
    render["draw_circle_filled"] = [=](float x, float y, float radius, const sol::table& color, int segments) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 c = ((static_cast<int>(color["r"]) << 24) | (static_cast<int>(color["g"]) << 16) | (static_cast<int>(color["b"]) << 8) | static_cast<int>(color["a"]));
        drawList->AddCircleFilled(ImVec2(x, y), radius, c, segments);
    };
    
    render["draw_text"] = [=](const std::string& text, float x, float y, const sol::table& color, float fontSize) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 c = ((static_cast<int>(color["r"]) << 24) | (static_cast<int>(color["g"]) << 16) | (static_cast<int>(color["b"]) << 8) | static_cast<int>(color["a"]));
        ImFont* font = gui::ViceFonts::Instance().GetFont(gui::ViceFonts::FontType::UI_Normal);
        drawList->AddText(font, fontSize, ImVec2(x, y), c, text.c_str());
    };
    
    render["get_text_size"] = [=](const std::string& text, float fontSize) -> sol::table {
        ImFont* font = gui::ViceFonts::Instance().GetFont(gui::ViceFonts::FontType::UI_Normal);
        ImVec2 size = font->CalcTextSizeA(fontSize, FLT_MAX, 0, text.c_str());
        auto t = m_lua.create_table();
        t["x"] = size.x;
        t["y"] = size.y;
        return t;
    };
    
    render["color"] = [=](int r, int g, int b, int a) -> sol::table {
        auto t = m_lua.create_table();
        t["r"] = std::clamp(r, 0, 255);
        t["g"] = std::clamp(g, 0, 255);
        t["b"] = std::clamp(b, 0, 255);
        t["a"] = std::clamp(a, 0, 255);
        return t;
    };
    
    m_lua["render"] = render;
}

void LuaManager::RegisterCallbackAPI() {
    auto callbacks = m_lua.create_table();
    
    callbacks["register"] = [this](const std::string& event, sol::function callback) {
        RegisterCallback(event, [callback](sol::state&) {
            try {
                callback();
            } catch (const sol::error& e) {
                LOG_ERROR(Lua, "Callback error: {}", e.what());
            }
        });
    };
    
    callbacks["unregister"] = [this](const std::string& event) {
        UnregisterCallback(event);
    };
    
    // Built-in events
    callbacks["on_create_move"] = [this](sol::function callback) {
        RegisterCallback("create_move", [callback](sol::state&) {
            try {
                // Would need to pass CUserCmd
                callback();
            } catch (const sol::error& e) {
                LOG_ERROR(Lua, "on_create_move error: {}", e.what());
            }
        });
    };
    
    callbacks["on_frame_stage"] = [this](sol::function callback) {
        RegisterCallback("frame_stage", [callback](sol::state&) {
            try {
                callback();
            } catch (const sol::error& e) {
                LOG_ERROR(Lua, "on_frame_stage error: {}", e.what());
            }
        });
    };
    
    callbacks["on_render"] = [this](sol::function callback) {
        RegisterCallback("render", [callback](sol::state&) {
            try {
                callback();
            } catch (const sol::error& e) {
                LOG_ERROR(Lua, "on_render error: {}", e.what());
            }
        });
    };
    
    callbacks["on_unload"] = [this](sol::function callback) {
        RegisterCallback("unload", [callback](sol::state&) {
            try {
                callback();
            } catch (const sol::error& e) {
                LOG_ERROR(Lua, "on_unload error: {}", e.what());
            }
        });
    };
    
    m_lua["callbacks"] = callbacks;
}

void LuaManager::RegisterUtilityAPI() {
    auto util = m_lua.create_table();
    
    util["sleep"] = [](int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    };
    
    util["get_time"] = []() -> double {
        return std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();
    };
    
    util["random_int"] = [](int min, int max) -> int {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    };
    
    util["random_float"] = [](float min, float max) -> float {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    };
    
    util["vector"] = [=](float x, float y, float z) -> sol::table {
        auto t = m_lua.create_table();
        t["x"] = x; t["y"] = y; t["z"] = z;
        return t;
    };
    
    util["vector2"] = [=](float x, float y) -> sol::table {
        auto t = m_lua.create_table();
        t["x"] = x; t["y"] = y;
        return t;
    };
    
    util["angle"] = [=](float pitch, float yaw, float roll) -> sol::table {
        auto t = m_lua.create_table();
        t["pitch"] = pitch; t["yaw"] = yaw; t["roll"] = roll;
        return t;
    };
    
    util["normalize_angle"] = [=](float angle) -> float {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    };
    
    util["clamp"] = [=](float val, float min, float max) -> float {
        return std::clamp(val, min, max);
    };
    
    util["lerp"] = [=](float a, float b, float t) -> float {
        return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
    };
    
    util["distance_2d"] = [=](float x1, float y1, float x2, float y2) -> float {
        float dx = x2 - x1;
        float dy = y2 - y1;
        return sqrtf(dx * dx + dy * dy);
    };
    
    util["distance_3d"] = [=](float x1, float y1, float z1, float x2, float y2, float z2) -> float {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float dz = z2 - z1;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    };
    
    m_lua["util"] = util;
}

void LuaManager::RegisterConfigAPI() {
    auto config = m_lua.create_table();
    
    config["get"] = [=](const std::string& category, const std::string& key, sol::object defaultValue) -> sol::object {
        auto& cfg = core::config::ConfigManager::Instance();
        try {
            auto opt = cfg.Get(category, key);
            if (opt) {
                return sol::make_object(m_lua, *opt);
            }
        } catch (...) {}
        return defaultValue;
    };
    
    config["set"] = [=](const std::string& category, const std::string& key, sol::object value) {
        try {
            core::config::ConfigManager::Instance().Set(category, key, value);
        } catch (...) {}
    };
    
    config["save"] = [=](const std::string& name) {
        core::config::ConfigManager::Instance().Save(name);
    };
    
    config["load"] = [=](const std::string& name) {
        core::config::ConfigManager::Instance().Load(name);
    };
    
    m_lua["config"] = config;
}

void LuaManager::RegisterLoggerAPI() {
    auto logger = m_lua.create_table();
    
    logger["trace"] = [](const std::string& module, const std::string& msg) {
        LOG_TRACE(module.c_str(), msg);
    };
    
    logger["debug"] = [](const std::string& module, const std::string& msg) {
        LOG_DEBUG(module.c_str(), msg);
    };
    
    logger["info"] = [](const std::string& module, const std::string& msg) {
        LOG_INFO(module.c_str(), msg);
    };
    
    logger["warn"] = [](const std::string& module, const std::string& msg) {
        LOG_WARN(module.c_str(), msg);
    };
    
    logger["error"] = [](const std::string& module, const std::string& msg) {
        LOG_ERROR(module.c_str(), msg);
    };
    
    logger["critical"] = [](const std::string& module, const std::string& msg) {
        LOG_CRITICAL(module.c_str(), msg);
    };
    
    m_lua["logger"] = logger;
}

void LuaManager::RegisterMathAPI() {
    // Already in util
}

void LuaManager::RegisterInputAPI() {
    auto input = m_lua.create_table();
    
    input["is_key_down"] = [=](int key) -> bool {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    };
    
    input["is_key_pressed"] = [=](int key) -> bool {
        static std::unordered_map<int, bool> prevState;
        bool current = (GetAsyncKeyState(key) & 0x8000) != 0;
        bool prev = prevState[key];
        prevState[key] = current;
        return current && !prev;
    };
    
    input["is_key_released"] = [=](int key) -> bool {
        static std::unordered_map<int, bool> prevState;
        bool current = (GetAsyncKeyState(key) & 0x8000) != 0;
        bool prev = prevState[key];
        prevState[key] = current;
        return !current && prev;
    };
    
    input["get_mouse_pos"] = [=]() -> sol::table {
        POINT pt;
        GetCursorPos(&pt);
        auto t = m_lua.create_table();
        t["x"] = pt.x;
        t["y"] = pt.y;
        return t;
    };
    
    input["set_mouse_pos"] = [=](int x, int y) {
        SetCursorPos(x, y);
    };
    
    m_lua["input"] = input;
}

void LuaManager::RegisterHookAPI() {
    auto hook = m_lua.create_table();
    
    hook["create_hook"] = [=](const std::string& name, uintptr_t target, uintptr_t detour) -> bool {
        auto h = core::hooks::HookManager::Instance().CreateDetour(target, detour, name.c_str());
        return h != nullptr;
    };
    
    hook["remove_hook"] = [=](const std::string& name) {
        core::hooks::HookManager::Instance().RemoveDetour(name.c_str());
    };
    
    hook["enable_hook"] = [=](const std::string& name) -> bool {
        auto h = core::hooks::HookManager::Instance().GetDetour(name.c_str());
        return h && h->Enable();
    };
    
    hook["disable_hook"] = [=](const std::string& name) -> bool {
        auto h = core::hooks::HookManager::Instance().GetDetour(name.c_str());
        return h && h->Disable();
    };
    
    m_lua["hook"] = hook;
}

void LuaManager::LoadBuiltinScripts() {
    // Create built-in script files if they don't exist
    std::filesystem::create_directories(m_scriptsPath);
    
    // legit_aa.lua
    std::string legitAA = R"(
-- Legit Anti-Aim Helper
callbacks.register("on_create_move", function()
    if not config.get("aimbot_legit", "legit_aa", "enabled", false) then return end
    
    local cmd = ... -- Would get CUserCmd
    local lp = entity.get_local_player()
    if not lp or not entity.is_alive(lp) then return end
    
    -- Simple freestanding
    local aa = config.get("aimbot_legit", "legit_aa", "yaw", "freestanding")
    if aa == "freestanding" then
        -- Freestanding logic here
    end
end)
)";
    
    std::ofstream(m_scriptsPath + "legit_aa.lua") << legitAA;
    
    // indicators.lua
    std::string indicators = R"(
-- Screen Indicators
callbacks.register("on_render", function()
    local lp = entity.get_local_player()
    if not lp or not entity.is_alive(lp) then return end
    
    -- Draw indicators for DT, HS, FL, AA
    local screen = render.get_text_size("DT", 14)
    render.draw_text("DOUBLETAP", 10, 10, render.color(255, 95, 155, 255), 14)
end)
)";
    
    std::ofstream(m_scriptsPath + "indicators.lua") << indicators;
    
    // anim_breaker.lua
    std::string animBreaker = R"(
-- Animation Breaker
callbacks.register("on_create_move", function()
    -- Animation breaking logic
end)
)";
    
    std::ofstream(m_scriptsPath + "anim_breaker.lua") << animBreaker;
}

void LuaManager::LoadAutoLoadScripts() {
    auto& cfg = core::config::ConfigManager::Instance();
    auto autoLoad = cfg.GetArray<std::string>("lua.auto_load");
    
    for (const auto& script : autoLoad) {
        LoadScript(script);
        EnableScript(script, true);
    }
}

bool LuaManager::LoadScript(const std::string& name) {
    std::string path = m_scriptsPath + name;
    if (!path.ends_with(".lua")) path += ".lua";
    
    if (!std::filesystem::exists(path)) {
        LOG_WARN(Lua, "Script not found: {}", path);
        return false;
    }
    
    try {
        // Create new state for script isolation
        auto scriptState = std::make_unique<sol::state>();
        scriptState->open_libraries(
            sol::lib::base, sol::lib::package, sol::lib::table,
            sol::lib::string, sol::lib::math, sol::lib::coroutine
        );
        
        // Share globals from main state
        for (auto& pair : m_lua) {
            scriptState->set(pair.first, pair.second);
        }
        
        // Load and run script
        scriptState->script_file(path);
        
        m_scripts[name] = std::move(scriptState);
        LOG_INFO(Lua, "Loaded script: {}", name);
        return true;
    } catch (const sol::error& e) {
        LOG_ERROR(Lua, "Failed to load script {}: {}", name, e.what());
        return false;
    }
}

bool LuaManager::UnloadScript(const std::string& name) {
    auto it = m_scripts.find(name);
    if (it == m_scripts.end()) return false;
    
    try {
        (*it->second)["__unload"]();
    } catch (...) {}
    
    m_scripts.erase(it);
    LOG_INFO(Lua, "Unloaded script: {}", name);
    return true;
}

bool LuaManager::ReloadScript(const std::string& name) {
    UnloadScript(name);
    return LoadScript(name);
}

bool LuaManager::EnableScript(const std::string& name, bool enable) {
    // Scripts are enabled by default when loaded
    return true;
}

bool LuaManager::SetAutoLoad(const std::string& name, bool autoLoad) {
    auto& cfg = core::config::ConfigManager::Instance();
    auto scripts = cfg.GetArray<std::string>("lua.auto_load");
    
    auto it = std::find(scripts.begin(), scripts.end(), name);
    if (autoLoad) {
        if (it == scripts.end()) {
            scripts.push_back(name);
        }
    } else {
        if (it != scripts.end()) {
            scripts.erase(it);
        }
    }
    
    cfg.SetArray("lua.auto_load", scripts);
    return true;
}

std::vector<std::string> LuaManager::GetLoadedScripts() const {
    std::vector<std::string> result;
    for (const auto& [name, _] : m_scripts) {
        result.push_back(name);
    }
    return result;
}

std::vector<std::string> LuaManager::GetAvailableScripts() const {
    std::vector<std::string> result;
    if (std::filesystem::exists(m_scriptsPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(m_scriptsPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                result.push_back(entry.path().stem().string());
            }
        }
    }
    return result;
}

std::optional<std::string> LuaManager::GetScriptContent(const std::string& name) {
    std::string path = m_scriptsPath + name;
    if (!path.ends_with(".lua")) path += ".lua";
    
    if (!std::filesystem::exists(path)) return std::nullopt;
    
    try {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } catch (...) {
        return std::nullopt;
    }
}

bool LuaManager::SaveScript(const std::string& name, const std::string& content) {
    std::string path = m_scriptsPath + name;
    if (!path.ends_with(".lua")) path += ".lua";
    
    try {
        std::ofstream file(path);
        file << content;
        return true;
    } catch (...) {
        return false;
    }
}

bool LuaManager::CreateScript(const std::string& name, const std::string& content) {
    std::string path = m_scriptsPath + name;
    if (!path.ends_with(".lua")) path += ".lua";
    
    if (std::filesystem::exists(path)) return false;
    
    return SaveScript(name, content);
}

bool LuaManager::DeleteScript(const std::string& name) {
    std::string path = m_scriptsPath + name;
    if (!path.ends_with(".lua")) path += ".lua";
    
    if (!std::filesystem::exists(path)) return false;
    
    try {
        std::filesystem::remove(path);
        UnloadScript(name);
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<LuaManager::ScriptInfo> LuaManager::GetScriptInfo(const std::string& name) {
    ScriptInfo info;
    info.name = name;
    info.path = m_scriptsPath + name + ".lua";
    info.loaded = m_scripts.contains(name);
    info.enabled = info.loaded;
    
    if (std::filesystem::exists(info.path)) {
        auto ftime = std::filesystem::last_write_time(info.path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        info.lastModified = sctp;
        info.size = std::filesystem::file_size(info.path);
    }
    
    // Parse script for metadata
    auto content = GetScriptContent(name);
    if (content) {
        // Simple parsing for -- @author, -- @description, etc.
        std::istringstream iss(*content);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("-- @author") == 0) info.author = line.substr(10);
            else if (line.find("-- @description") == 0) info.description = line.substr(15);
            else if (line.find("-- @version") == 0) info.version = line.substr(11);
        }
    }
    
    return info;
}

bool LuaManager::ExecuteString(const std::string& code, std::string* output) {
    try {
        sol::protected_function_result result = m_lua.script(code);
        if (!result.valid()) {
            sol::error err = result;
            if (output) *output = err.what();
            return false;
        }
        return true;
    } catch (const sol::error& e) {
        if (output) *output = e.what();
        return false;
    }
}

bool LuaManager::ExecuteFile(const std::string& path, std::string* output) {
    try {
        m_lua.script_file(path);
        return true;
    } catch (const sol::error& e) {
        if (output) *output = e.what();
        return false;
    }
}

void LuaManager::RegisterCallback(const std::string& event, LuaCallback callback) {
    m_callbacks[event].push_back(callback);
}

void LuaManager::UnregisterCallback(const std::string& event) {
    m_callbacks.erase(event);
}

void LuaManager::OnCreateMove(sdk::CUserCmd* cmd) {
    auto it = m_callbacks.find("create_move");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::OnFrameStageNotify(int stage) {
    auto it = m_callbacks.find("frame_stage");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::OnRender() {
    auto it = m_callbacks.find("render");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::OnFireEvent(void* event) {
    auto it = m_callbacks.find("fire_event");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::OnDispatchSound(void* sound) {
    auto it = m_callbacks.find("dispatch_sound");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::OnUnload() {
    auto it = m_callbacks.find("unload");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::OnMenuOpen(bool open) {
    auto it = m_callbacks.find("menu_open");
    if (it != m_callbacks.end()) {
        for (auto& cb : it->second) {
            try {
                cb(m_lua);
            } catch (...) {}
        }
    }
}

void LuaManager::AddConsoleEntry(ConsoleEntry::Type type, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    ConsoleEntry entry;
    entry.type = type;
    entry.message = message;
    entry.time = ImGui::GetTime();
    m_console.push_back(entry);
    
    if (m_console.size() > m_consoleMaxEntries) {
        m_console.erase(m_console.begin());
    }
}

std::vector<LuaManager::ConsoleEntry> LuaManager::GetConsoleEntries() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_console;
}

void LuaManager::ClearConsole() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_console.clear();
}

int LuaManager::LuaPanicHandler(lua_State* L) {
    LOG_CRITICAL(Lua, "Lua panic!");
    return 0;
}

void LuaManager::PrintCallback(const std::string& msg) {
    AddConsoleEntry(ConsoleEntry::Type::Log, msg);
    LOG_INFO(Lua, "{}", msg);
}

void LuaManager::WarningCallback(const std::string& msg) {
    AddConsoleEntry(ConsoleEntry::Type::Warning, msg);
    LOG_WARN(Lua, "{}", msg);
}

void LuaManager::ErrorCallback(const std::string& msg) {
    AddConsoleEntry(ConsoleEntry::Type::Error, msg);
    LOG_ERROR(Lua, "{}", msg);
}

void LuaManager::HandleLuaError(const std::string& context, const std::string& error) {
    LOG_ERROR(Lua, "{} error: {}", context, error);
    AddConsoleEntry(ConsoleEntry::Type::Error, fmt::format("[{}] {}", context, error));
}

} // namespace features::lua