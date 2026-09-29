#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <filesystem>
#include "../../features/lua/lua_manager.hpp"
#include "../vice_widgets.hpp"
#include "../vice_theme.hpp"

namespace gui {

class LuaTab {
public:
    static LuaTab& Instance() {
        static LuaTab instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
private:
    LuaTab() = default;
    
    enum class SubTab { Scripts, Editor, Console, API };
    SubTab m_currentSubTab = SubTab::Scripts;
    
    // State
    std::string m_newScriptName;
    std::string m_editorContent;
    std::string m_selectedScript;
    int m_selectedScriptIndex = -1;
    bool m_showNewScriptDialog = false;
    bool m_showDeleteConfirm = false;
    bool m_autoReload = true;
    bool m_showLineNumbers = true;
    bool m_wordWrap = false;
    int m_tabSize = 4;
    std::string m_searchQuery;
    std::string m_consoleFilter;
    ImGuiTextFilter m_consoleFilterObj;
    
    // Editor
    struct EditorState {
        std::string content;
        std::string filename;
        bool modified = false;
        int cursorLine = 0;
        int cursorCol = 0;
        std::vector<int> breakpoints;
        std::vector<int> errorLines;
        std::vector<int> warningLines;
    };
    EditorState m_editorState;
    
    // Console
    struct ConsoleEntry {
        enum class Type { Log, Warning, Error, System, Input } type;
        std::string message;
        float time = 0.0f;
        int line = 0;
    };
    std::vector<ConsoleEntry> m_consoleEntries;
    std::mutex m_consoleMutex;
    bool m_consoleAutoScroll = true;
    int m_consoleMaxEntries = 1000;
    
    // Script list
    struct ScriptInfo {
        std::string name;
        std::string path;
        std::string author;
        std::string description;
        std::string version;
        bool enabled = false;
        bool autoLoad = false;
        bool hasError = false;
        std::string errorMessage;
        std::chrono::system_clock::time_point lastModified;
        size_t size = 0;
    };
    std::vector<ScriptInfo> m_scripts;
    
    void RenderScriptsTab();
    void RenderEditorTab();
    void RenderConsoleTab();
    void RenderAPITab();
    
    // Script operations
    void RefreshScripts();
    bool CreateScript(const std::string& name);
    bool DeleteScript(const std::string& name);
    bool EnableScript(const std::string& name, bool enable);
    bool SetAutoLoad(const std::string& name, bool autoLoad);
    bool LoadScript(const std::string& name);
    bool UnloadScript(const std::string& name);
    bool ReloadScript(const std::string& name);
    bool SaveScript(const std::string& name, const std::string& content);
    
    // Editor operations
    void OpenScriptInEditor(const std::string& name);
    void CloseEditor();
    bool SaveEditorContent();
    void RunEditorContent();
    void FormatEditorContent();
    void ToggleBreakpoint(int line);
    void ClearBreakpoints();
    
    // Console operations
    void AddConsoleEntry(ConsoleEntry::Type type, const std::string& message, int line = 0);
    void ClearConsole();
    void CopyConsoleSelection();
    void SaveConsoleToFile();
    
    // Lua callbacks
    static void LuaPrintCallback(const std::string& msg);
    static void LuaWarningCallback(const std::string& msg);
    static void LuaErrorCallback(const std::string& msg);
    
    // API documentation
    void RenderAPIDocumentation();
    void RenderAPISearch();
    
    // Theme access
    gui::ViceTheme::Colors& m_colors = gui::ViceTheme::Instance().GetColors();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
    gui::ViceFonts& m_fonts = gui::ViceFonts::Instance();
};

} // namespace gui