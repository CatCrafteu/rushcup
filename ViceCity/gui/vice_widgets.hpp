#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include "vice_theme.hpp"

namespace gui {

// Custom ViceCity widgets
class ViceWidgets {
public:
    static ViceWidgets& Instance() {
        static ViceWidgets instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    
    // ========== Color Picker with Gradient ==========
    struct ColorPickerConfig {
        bool showAlpha = true;
        bool showHSV = true;
        bool showRGB = true;
        bool showHEX = true;
        bool showPresets = true;
        bool showGradient = true;
        ImVec2 pickerSize = {200, 200};
        ImVec2 barWidth = {16, 0};
        std::vector<ImU32> presets;
    };
    
    bool ColorPicker(const char* label, ImU32& color, const ColorPickerConfig& config = {});
    bool ColorPicker4(const char* label, float color[4], ImGuiColorEditFlags flags = 0);
    bool ColorPicker3(const char* label, float color[3], ImGuiColorEditFlags flags = 0);
    
    // Gradient color picker (ViceCity signature)
    bool GradientColorPicker(const char* label, ImU32& color1, ImU32& color2, 
                              bool& horizontal, const ColorPickerConfig& config = {});
    
    // ========== Keybind ==========
    enum class KeybindMode { Toggle, Hold, Always, DoubleTap };
    
    struct KeybindConfig {
        bool allowNone = true;
        bool allowMouse = true;
        ImVec2 size = {120, 24};
        std::string noneText = "None";
    };
    
    bool Keybind(const char* label, int& key, KeybindMode& mode, const KeybindConfig& config = {});
    bool KeybindEx(const char* label, int& key, KeybindMode& mode, bool& enabled, const KeybindConfig& config = {});
    
    // Keybind mode selector popup
    void KeybindModePopup(const char* id, KeybindMode& mode);
    
    // ========== Combo / Multi-select ==========
    struct ComboConfig {
        bool searchable = false;
        bool multiSelect = false;
        int maxVisibleItems = 10;
        ImVec2 size = {200, 0};
        std::string previewText = "";
        std::string noItemsText = "No items";
    };
    
    bool Combo(const char* label, int& current, const std::vector<std::string>& items, const ComboConfig& config = {});
    bool Combo(const char* label, int& current, const char* const* items, int itemsCount, const ComboConfig& config = {});
    bool MultiCombo(const char* label, std::vector<int>& selected, const std::vector<std::string>& items, const ComboConfig& config = {});
    bool MultiCombo(const char* label, std::vector<bool>& selected, const std::vector<std::string>& items, const ComboConfig& config = {});
    
    // Searchable combo
    bool SearchableCombo(const char* label, int& current, const std::vector<std::string>& items, const ComboConfig& config = {});
    
    // ========== Slider ==========
    struct SliderConfig {
        float min = 0.0f;
        float max = 100.0f;
        float step = 1.0f;
        const char* format = "%.0f";
        bool showValue = true;
        bool logarithmic = false;
        ImVec2 size = {200, 0};
        std::string tooltip = "";
        std::function<std::string(float)> customFormat = nullptr;
    };
    
    bool SliderFloat(const char* label, float& value, const SliderConfig& config);
    bool SliderInt(const char* label, int& value, const SliderConfig& config);
    bool SliderFloat2(const char* label, float value[2], const SliderConfig& config);
    bool SliderFloat3(const char* label, float value[3], const SliderConfig& config);
    bool SliderFloat4(const char* label, float value[4], const SliderConfig& config);
    
    // Dual slider (min/max)
    bool SliderFloatRange(const char* label, float& min, float& max, const SliderConfig& config);
    bool SliderIntRange(const char* label, int& min, int& max, const SliderConfig& config);
    
    // ========== Tab Bar (Animated) ==========
    struct TabBarConfig {
        bool animated = true;
        bool closable = false;
        bool reorderable = false;
        ImVec2 tabMinSize = {80, 28};
        ImVec2 tabMaxSize = {200, 28};
        float animationSpeed = 0.2f;
        std::vector<ImU32> tabColors; // Custom colors per tab
    };
    
    bool BeginTabBar(const char* id, const TabBarConfig& config = {});
    void EndTabBar();
    bool TabItem(const char* label, bool* open = nullptr, ImGuiTabItemFlags flags = 0);
    bool TabItemEx(const char* label, bool* open, ImGuiTabItemFlags flags, ImU32 customColor);
    
    // Vertical tab bar
    bool BeginVerticalTabBar(const char* id, const TabBarConfig& config = {});
    void EndVerticalTabBar();
    bool VerticalTabItem(const char* label, bool* open = nullptr);
    
    // ========== Custom Button ==========
    struct ButtonConfig {
        ImVec2 size = {0, 0};
        bool disabled = false;
        ImU32 customColor = 0;
        ImU32 customHoverColor = 0;
        ImU32 customActiveColor = 0;
        ImU32 textColor = 0;
        float rounding = -1.0f; // -1 = use theme
        bool icon = false;
        const char* iconFont = nullptr;
        ImU32 iconColor = 0;
    };
    
    bool Button(const char* label, const ButtonConfig& config = {});
    bool SmallButton(const char* label, const ButtonConfig& config = {});
    bool InvisibleButton(const char* id, const ImVec2& size, const ButtonConfig& config = {});
    bool ArrowButton(const char* id, ImGuiDir dir, const ButtonConfig& config = {});
    
    // Toggle button
    bool ToggleButton(const char* label, bool& value, const ButtonConfig& config = {});
    
    // ========== Checkbox ==========
    struct CheckboxConfig {
        ImVec2 size = {18, 18};
        float rounding = 3.0f;
        ImU32 checkColor = 0;
        ImU32 bgColor = 0;
        bool animated = true;
    };
    
    bool Checkbox(const char* label, bool& value, const CheckboxConfig& config = {});
    bool CheckboxFlags(const char* label, int& flags, int flagValue, const CheckboxConfig& config = {});
    
    // ========== Input Text ==========
    struct InputTextConfig {
        ImVec2 size = {200, 0};
        ImGuiInputTextFlags flags = 0;
        std::string hint = "";
        bool readOnly = false;
        std::function<bool(const std::string&)> validator = nullptr;
    };
    
    bool InputText(const char* label, std::string& value, const InputTextConfig& config = {});
    bool InputTextMultiline(const char* label, std::string& value, const ImVec2& size, const InputTextConfig& config = {});
    bool InputInt(const char* label, int& value, const InputTextConfig& config = {});
    bool InputFloat(const char* label, float& value, const InputTextConfig& config = {});
    
    // ========== Tooltip ==========
    void Tooltip(const char* text);
    void TooltipBegin();
    void TooltipEnd();
    void TooltipEx(const char* title, const char* description, const char* shortcut = nullptr);
    
    // ========== Separator ==========
    void Separator(const char* label = nullptr);
    void SeparatorText(const char* label);
    // void SeparatorEx(ImGuiSeparatorFlags flags); // Requires newer ImGui
    
    // ========== Group / Panel ==========
    struct PanelConfig {
        ImVec2 size = {0, 0};
        bool bordered = true;
        bool rounded = true;
        ImU32 bgColor = 0;
        ImU32 borderColor = 0;
        float rounding = -1.0f;
        float padding = 12.0f;
    };
    
    void BeginPanel(const char* id, const PanelConfig& config = {});
    void EndPanel();
    void BeginGroupPanel(const char* label, const ImVec2& size = {0, 0});
    void EndGroupPanel();
    
    // ========== Progress Bar ==========
    struct ProgressConfig {
        ImVec2 size = {0, 0};
        float rounding = 4.0f;
        ImU32 bgColor = 0;
        ImU32 fillColor = 0;
        bool showText = true;
        std::string format = "%.0f%%";
        bool animated = true;
    };
    
    void ProgressBar(float fraction, const ProgressConfig& config = {});
    void ProgressBarEx(float fraction, const char* overlay, const ProgressConfig& config = {});
    
    // ========== Spinner / Loading ==========
    void Spinner(const char* id, float radius, int thickness, ImU32 color, float speed = 2.0f);
    void SpinnerEx(const char* id, float radius, int thickness, ImU32 color, int segments, float speed);
    
    // ========== Notification / Toast ==========
    enum class NotificationType { Info, Success, Warning, Error };
    
    struct Notification {
        std::string title;
        std::string message;
        NotificationType type = NotificationType::Info;
        float duration = 3.0f;
        float elapsed = 0.0f;
        ImVec2 size = {300, 0};
    };
    
    void PushNotification(const Notification& notification);
    void PushNotification(const std::string& title, const std::string& message, NotificationType type, float duration = 3.0f);
    void RenderNotifications();
    void ClearNotifications();
    
    // ========== Knob (Circular Slider) ==========
    struct KnobConfig {
        float radius = 30.0f;
        float min = 0.0f;
        float max = 1.0f;
        float step = 0.01f;
        ImU32 bgColor = 0;
        ImU32 trackColor = 0;
        ImU32 indicatorColor = 0;
        bool showValue = true;
        std::string format = "%.2f";
    };
    
    bool Knob(const char* label, float& value, const KnobConfig& config = {});
    
    // ========== Color Preview ==========
    void ColorPreview(ImU32 color, const ImVec2& size = {24, 24}, bool border = true, float rounding = 3.0f);
    void ColorPreviewWithAlpha(ImU32 color, const ImVec2& size = {24, 24}, bool border = true, float rounding = 3.0f);
    
    // ========== Icon Button ==========
    bool IconButton(const char* icon, const char* tooltip = nullptr, const ButtonConfig& config = {});
    
    // ========== Search Bar ==========
    bool SearchBar(const char* id, std::string& query, const char* placeholder = "Search...", const ImVec2& size = {200, 0});
    
    // ========== Property Grid ==========
    struct PropertyConfig {
        float labelWidth = 120.0f;
        float spacing = 4.0f;
    };
    
    void BeginPropertyGrid(const char* id, const PropertyConfig& config = {});
    void EndPropertyGrid();
    bool Property(const char* label, bool& value);
    bool Property(const char* label, int& value);
    bool Property(const char* label, float& value);
    bool Property(const char* label, std::string& value);
    bool Property(const char* label, ImU32& color);
    bool Property(const char* label, int& value, const std::vector<std::string>& items);
    
    // ========== Collapsible Section ==========
    bool CollapsibleSection(const char* label, bool& open, bool defaultOpen = true);
    
    // ========== Hotkey Display ==========
    void HotkeyDisplay(int key, KeybindMode mode, const ImVec2& size = {100, 20});

private:
    ViceWidgets() = default;
    std::vector<Notification> m_notifications;
    std::mutex m_mutex;
    
    // Internal helpers
    void RenderNotification(const Notification& notif, int index);
    ImU32 GetKeybindModeColor(KeybindMode mode) const;
    std::string GetKeyName(int key) const;
    bool IsKeyPressed(int key) const;
};

} // namespace gui