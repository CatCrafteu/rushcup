#include "core/pch.hpp"
#include "gui/tabs/lua_tab.hpp"
#include "features/lua/lua_manager.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"
#include "gui/vice_fonts.hpp"

namespace gui {

void LuaTab::Initialize() {
    m_consoleFilterObj = ImGuiTextFilter();
    RefreshScripts();
    LOG_INFO(GUI, "LuaTab initialized");
}

void LuaTab::Shutdown() {
    LOG_INFO(GUI, "LuaTab shutdown");
}

void LuaTab::Render() {
    ImGui::BeginChild("##lua_subtabs", ImVec2(0, 30), false);
    
    const char* subTabs[] = {"Scripts", "Editor", "Console", "API"};
    for (int i = 0; i < 4; ++i) {
        bool selected = static_cast<int>(m_currentSubTab) == i;
        if (ImGui::Selectable(subTabs[i], selected, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x / 4, 0))) {
            m_currentSubTab = static_cast<SubTab>(i);
        }
        if (i < 3) ImGui::SameLine();
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    
    switch (m_currentSubTab) {
        case SubTab::Scripts: RenderScriptsTab(); break;
        case SubTab::Editor: RenderEditorTab(); break;
        case SubTab::Console: RenderConsoleTab(); break;
        case SubTab::API: RenderAPITab(); break;
    }
}

void LuaTab::RenderScriptsTab() {
    ImGui::BeginChild("##scripts_list", ImVec2(0, 0), false);
    
    // Toolbar
    if (ImGui::Button("New Script")) {
        m_showNewScriptDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        RefreshScripts();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto Reload", &m_autoReload);
    ImGui::Separator();
    
    // Script list
    if (ImGui::BeginTable("##scripts_table", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Auto Load", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableHeadersRow();
        
        for (size_t i = 0; i < m_scripts.size(); ++i) {
            const auto& script = m_scripts[i];
            
            ImGui::TableNextRow();
            
            // Name
            ImGui::TableSetColumnIndex(0);
            bool selected = static_cast<int>(i) == m_selectedScriptIndex;
            if (ImGui::Selectable(script.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedScriptIndex = static_cast<int>(i);
                m_selectedScript = script.name;
            }
            
            // Status
            ImGui::TableSetColumnIndex(1);
            if (script.loaded) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), script.enabled ? "Running" : "Loaded");
            } else {
                ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "Unloaded");
            }
            if (script.hasError) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "[Error]");
            }
            
            // Auto Load
            ImGui::TableSetColumnIndex(2);
            bool autoLoad = script.autoLoad;
            if (ImGui::Checkbox(("##autoload_" + script.name).c_str(), &autoLoad)) {
                SetAutoLoad(script.name, autoLoad);
            }
            
            // Actions
            ImGui::TableSetColumnIndex(3);
            if (script.loaded) {
                if (ImGui::SmallButton(("Unload##" + script.name).c_str())) {
                    UnloadScript(script.name);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton(("Reload##" + script.name).c_str())) {
                    ReloadScript(script.name);
                }
            } else {
                if (ImGui::SmallButton(("Load##" + script.name).c_str())) {
                    LoadScript(script.name);
                }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(("Edit##" + script.name).c_str())) {
                OpenScriptInEditor(script.name);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(("Delete##" + script.name).c_str())) {
                m_showDeleteConfirm = true;
            }
        }
        ImGui::EndTable();
    }
    
    // Delete confirmation
    if (m_showDeleteConfirm && !m_selectedScript.empty()) {
        ImGui::OpenPopup("Delete Script");
        m_showDeleteConfirm = false;
    }
    
    if (ImGui::BeginPopupModal("Delete Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete script '%s'?", m_selectedScript.c_str());
        ImGui::Separator();
        if (ImGui::Button("Delete")) {
            DeleteScript(m_selectedScript);
            m_selectedScript.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_selectedScript.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // New script dialog
    if (m_showNewScriptDialog) {
        ImGui::OpenPopup("New Script");
        m_showNewScriptDialog = false;
    }
    
    if (ImGui::BeginPopupModal("New Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", &m_newScriptName);
        ImGui::Separator();
        if (ImGui::Button("Create")) {
            if (!m_newScriptName.empty()) {
                CreateScript(m_newScriptName);
                m_newScriptName.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_newScriptName.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::EndChild();
}

void LuaTab::RenderEditorTab() {
    ImGui::BeginChild("##editor", ImVec2(0, 0), false);
    
    if (m_selectedScript.empty()) {
        ImGui::Text("Select a script from the Scripts tab to edit.");
        ImGui::EndChild();
        return;
    }
    
    // Toolbar
    if (ImGui::Button("Save")) SaveEditorContent();
    ImGui::SameLine();
    if (ImGui::Button("Run")) RunEditorContent();
    ImGui::SameLine();
    if (ImGui::Button("Format")) FormatEditorContent();
    ImGui::SameLine();
    ImGui::Checkbox("Line Numbers", &m_showLineNumbers);
    ImGui::SameLine();
    ImGui::Checkbox("Word Wrap", &m_wordWrap);
    ImGui::SameLine();
    ImGui::DragInt("Tab Size", &m_tabSize, 1, 2, 8);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("Search", &m_searchQuery);
    ImGui::Separator();
    
    // Editor
    ImGui::PushFont(m_fonts.GetFont(gui::FontType::JetBrainsMono_Regular));
    
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    if (!m_wordWrap) flags |= ImGuiInputTextFlags_NoHorizontalScroll;
    
    bool changed = ImGui::InputTextMultiline("##editor", &m_editorContent, ImVec2(-1, -ImGui::GetFrameHeightWithSpacing() - 10), flags);
    
    if (changed) {
        m_editorState.modified = true;
    }
    
    ImGui::PopFont();
    
    // Status bar
    ImGui::Separator();
    ImGui::Text("Line: %d | Col: %d | Size: %zu bytes", m_editorState.cursorLine, m_editorState.cursorCol, m_editorContent.size());
    if (m_editorState.modified) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "[Modified]");
    }
    
    ImGui::EndChild();
}

void LuaTab::RenderConsoleTab() {
    ImGui::BeginChild("##console", ImVec2(0, 0), false);
    
    // Toolbar
    if (ImGui::Button("Clear")) ClearConsole();
    ImGui::SameLine();
    if (ImGui::Button("Copy")) CopyConsoleSelection();
    ImGui::SameLine();
    if (ImGui::Button("Save")) SaveConsoleToFile();
    ImGui::SameLine();
    ImGui::Checkbox("Auto Scroll", &m_consoleAutoScroll);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("Filter", &m_consoleFilter);
    ImGui::Separator();
    
    // Console output
    ImGui::BeginChild("##console_output", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10), false);
    
    auto& lua = features::lua::LuaManager::Instance();
    auto entries = lua.GetConsoleEntries();
    
    for (const auto& entry : entries) {
        if (!m_consoleFilter.empty()) {
            if (entry.message.find(m_consoleFilter) == std::string::npos) continue;
        }
        
        ImU32 color;
        switch (entry.type) {
            case features::lua::LuaManager::ConsoleEntry::Type::Log: color = IM_COL32(200, 200, 200, 255); break;
            case features::lua::LuaManager::ConsoleEntry::Type::Warning: color = IM_COL32(255, 215, 0, 255); break;
            case features::lua::LuaManager::ConsoleEntry::Type::Error: color = IM_COL32(255, 80, 80, 255); break;
            case features::lua::LuaManager::ConsoleEntry::Type::System: color = IM_COL32(0, 200, 255, 255); break;
            case features::lua::LuaManager::ConsoleEntry::Type::Input: color = IM_COL32(0, 255, 100, 255); break;
            default: color = IM_COL32(255, 255, 255, 255); break;
        }
        
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::Text("[%.3f] %s", entry.time, entry.message.c_str());
        ImGui::PopStyleColor();
    }
    
    if (m_consoleAutoScroll) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
    
    // Input
    ImGui::Separator();
    static char inputBuf[512] = "";
    ImGui::InputText("##console_input", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        // Execute command
        lua.ExecuteString(inputBuf);
        inputBuf[0] = '\0';
    }
    
    ImGui::EndChild();
}

void LuaTab::RenderAPITab() {
    ImGui::BeginChild("##api_docs", ImVec2(0, 0), false);
    
    ImGui::Text("Lua API Documentation");
    ImGui::Separator();
    
    RenderAPISearch();
    ImGui::Separator();
    
    RenderAPIDocumentation();
    
    ImGui::EndChild();
}

// Script Operations
void LuaTab::RefreshScripts() {
    m_scripts.clear();
    auto scripts = features::lua::LuaManager::Instance().GetAvailableScripts();
    
    for (const auto& name : scripts) {
        auto info = features::lua::LuaManager::Instance().GetScriptInfo(name);
        if (info) {
            m_scripts.push_back(*info);
        } else {
            LuaManager::ScriptInfo si;
            si.name = name;
            si.path = "ViceCity/lua/scripts/" + name + ".lua";
            si.loaded = false;
            si.enabled = false;
            si.autoLoad = false;
            m_scripts.push_back(si);
        }
    }
}

bool LuaTab::CreateScript(const std::string& name) {
    std::string content = fmt::format(R"(-- {}
-- Created: {}

callbacks.register("on_render", function()
    -- Render code here
end)

callbacks.register("on_create_move", function(cmd)
    -- CreateMove code here
end)
)", name, __DATE__);
    
    if (features::lua::LuaManager::Instance().CreateScript(name, content)) {
        RefreshScripts();
        m_widgets.PushNotification("Script created: " + name);
        return true;
    }
    m_widgets.PushNotification("Failed to create script!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool LuaTab::DeleteScript(const std::string& name) {
    if (features::lua::LuaManager::Instance().DeleteScript(name)) {
        RefreshScripts();
        m_widgets.PushNotification("Script deleted: " + name);
        return true;
    }
    m_widgets.PushNotification("Failed to delete script!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool LuaTab::EnableScript(const std::string& name, bool enable) {
    if (features::lua::LuaManager::Instance().EnableScript(name, enable)) {
        RefreshScripts();
        return true;
    }
    return false;
}

bool LuaTab::SetAutoLoad(const std::string& name, bool autoLoad) {
    if (features::lua::LuaManager::Instance().SetAutoLoad(name, autoLoad)) {
        RefreshScripts();
        return true;
    }
    return false;
}

bool LuaTab::LoadScript(const std::string& name) {
    if (features::lua::LuaManager::Instance().LoadScript(name)) {
        m_widgets.PushNotification("Script loaded: " + name, gui::ViceWidgets::NotificationType::Success);
        RefreshScripts();
        return true;
    }
    m_widgets.PushNotification("Failed to load script: " + name, gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool LuaTab::UnloadScript(const std::string& name) {
    if (features::lua::LuaManager::Instance().UnloadScript(name)) {
        m_widgets.PushNotification("Script unloaded: " + name, gui::ViceWidgets::NotificationType::Success);
        RefreshScripts();
        return true;
    }
    m_widgets.PushNotification("Failed to unload script!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool LuaTab::ReloadScript(const std::string& name) {
    if (features::lua::LuaManager::Instance().ReloadScript(name)) {
        m_widgets.PushNotification("Script reloaded: " + name, gui::ViceWidgets::NotificationType::Success);
        RefreshScripts();
        return true;
    }
    m_widgets.PushNotification("Failed to reload script!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool LuaTab::SaveScript(const std::string& name, const std::string& content) {
    if (features::lua::LuaManager::Instance().SaveScript(name, content)) {
        m_widgets.PushNotification("Script saved: " + name, gui::ViceWidgets::NotificationType::Success);
        return true;
    }
    m_widgets.PushNotification("Failed to save script!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

// Editor Operations
void LuaTab::OpenScriptInEditor(const std::string& name) {
    auto content = features::lua::LuaManager::Instance().GetScriptContent(name);
    if (content) {
        m_editorState.content = *content;
        m_editorState.filename = name;
        m_editorState.modified = false;
        m_selectedScript = name;
    }
}

void LuaTab::CloseEditor() {
    m_selectedScript.clear();
    m_editorState = {};
}

bool LuaTab::SaveEditorContent() {
    if (m_selectedScript.empty()) return false;
    
    if (features::lua::LuaManager::Instance().SaveScript(m_selectedScript, m_editorState.content)) {
        m_editorState.modified = false;
        m_widgets.PushNotification("Script saved: " + m_selectedScript, gui::ViceWidgets::NotificationType::Success);
        return true;
    }
    m_widgets.PushNotification("Failed to save script!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

void LuaTab::RunEditorContent() {
    if (m_selectedScript.empty()) return;
    
    std::string output;
    if (features::lua::LuaManager::Instance().ExecuteString(m_editorState.content, &output)) {
        m_widgets.PushNotification("Script executed successfully", gui::ViceWidgets::NotificationType::Success);
    } else {
        m_widgets.PushNotification("Execution failed: " + output, gui::ViceWidgets::NotificationType::Error);
    }
}

void LuaTab::FormatEditorContent() {
    // Would format Lua code (requires external formatter)
}

void LuaTab::ToggleBreakpoint(int line) {
    auto it = std::find(m_editorState.breakpoints.begin(), m_editorState.breakpoints.end(), line);
    if (it != m_editorState.breakpoints.end()) {
        m_editorState.breakpoints.erase(it);
    } else {
        m_editorState.breakpoints.push_back(line);
    }
}

void LuaTab::ClearBreakpoints() {
    m_editorState.breakpoints.clear();
}

// Console Operations
void LuaTab::AddConsoleEntry(features::lua::LuaManager::ConsoleEntry::Type type, const std::string& message, int line) {
    features::lua::LuaManager::ConsoleEntry entry;
    entry.type = type;
    entry.message = message;
    entry.time = ImGui::GetTime();
    entry.line = line;
    
    auto& lua = features::lua::LuaManager::Instance();
    // Would add to lua console
}

void LuaTab::ClearConsole() {
    features::lua::LuaManager::Instance().ClearConsole();
}

void LuaTab::CopyConsoleSelection() {
    // Copy selected console text
}

void LuaTab::SaveConsoleToFile() {
    // Save console to file
}

// API Documentation
void LuaTab::RenderAPIDocumentation() {
    struct APICategory {
        const char* name;
        const char* description;
        const char* functions[10];
    };
    
    static const APICategory categories[] = {
        {"menu", "Menu control", {"menu.is_open()", "menu.set_open(bool)", "menu.toggle()", "menu.get_key()", "menu.set_key(int)"}},
        {"engine", "Engine interface", {"engine.client_cmd(str)", "engine.client_cmd_unrestricted(str)", "engine.get_local_player()", "engine.get_max_clients()", "engine.is_in_game()", "engine.get_level_name()", "engine.get_screen_size()", "engine.get_view_angles()", "engine.set_view_angles(ang)"}},
        {"entity", "Entity access", {"entity.get_local_player()", "entity.get_player(int)", "entity.get_player_by_handle(handle)", "entity.get_highest_entity_index()", "entity.get_player_info(int)", "entity.is_valid(ent)", "entity.get_origin(ent)", "entity.get_eye_position(ent)", "entity.get_bone_position(ent, bone)"}},
        {"render", "Rendering", {"render.world_to_screen(vec3)", "render.draw_line(x1,y1,x2,y2,color,thickness)", "render.draw_rect(x,y,w,h,color,rounding)", "render.draw_circle(x,y,r,color,segments)", "render.draw_text(text,x,y,color,fontSize)", "render.get_text_size(text,fontSize)", "render.color(r,g,b,a)"}},
        {"callbacks", "Event callbacks", {"callbacks.register(event,func)", "callbacks.unregister(event)", "callbacks.on_create_move(func)", "callbacks.on_frame_stage(func)", "callbacks.on_render(func)", "callbacks.on_unload(func)"}},
        {"util", "Utilities", {"util.sleep(ms)", "util.get_time()", "util.random_int(min,max)", "util.random_float(min,max)", "util.vector(x,y,z)", "util.vector2(x,y)", "util.angle(p,y,r)", "util.normalize_angle(ang)", "util.clamp(val,min,max)", "util.lerp(a,b,t)"}},
        {"config", "Configuration", {"config.get(cat,key,def)", "config.set(cat,key,val)", "config.save(name)", "config.load(name)"}},
        {"logger", "Logging", {"logger.trace(mod,msg)", "logger.debug(mod,msg)", "logger.info(mod,msg)", "logger.warn(mod,msg)", "logger.error(mod,msg)", "logger.critical(mod,msg)"}},
        {"input", "Input", {"input.is_key_down(key)", "input.is_key_pressed(key)", "input.is_key_released(key)", "input.get_mouse_pos()", "input.set_mouse_pos(x,y)"}},
        {"hook", "Hooking", {"hook.create_hook(name,target,detour)", "hook.remove_hook(name)", "hook.enable_hook(name)", "hook.disable_hook(name)"}}
    };
    
    for (const auto& cat : categories) {
        if (ImGui::CollapsingHeader(cat.name)) {
            ImGui::TextDisabled("%s", cat.description);
            ImGui::Separator();
            
            for (int i = 0; i < 10 && cat.functions[i]; ++i) {
                ImGui::Text("  %s", cat.functions[i]);
            }
            ImGui::Spacing();
        }
    }
}

void LuaTab::RenderAPISearch() {
    ImGui::InputText("Search API", &m_searchQuery);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) m_searchQuery.clear();
}

} // namespace gui