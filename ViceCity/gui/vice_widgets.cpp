#include "core/pch.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"
#include "gui/vice_fonts.hpp"
#include "gui/vice_render.hpp"

namespace gui {

void ViceWidgets::Initialize() {
    LOG_INFO(GUI, "ViceWidgets initialized");
}

void ViceWidgets::Shutdown() {
    ClearNotifications();
    LOG_INFO(GUI, "ViceWidgets shutdown");
}

// ========== Color Picker ==========

bool ViceWidgets::ColorPicker(const char* label, ImU32& color, const ColorPickerConfig& config) {
    ImGui::PushID(label);
    
    float col[4] = {
        IM_COL32_R(color) / 255.0f,
        IM_COL32_G(color) / 255.0f,
        IM_COL32_B(color) / 255.0f,
        IM_COL32_A(color) / 255.0f
    };
    
    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar;
    if (config.showHSV) flags |= ImGuiColorEditFlags_HSV;
    if (config.showRGB) flags |= ImGuiColorEditFlags_RGB;
    if (config.showHEX) flags |= ImGuiColorEditFlags_HEX;
    
    bool changed = ImGui::ColorEdit4(label, col, flags);
    
    if (changed) {
        color = IM_COL32(
            static_cast<int>(col[0] * 255),
            static_cast<int>(col[1] * 255),
            static_cast<int>(col[2] * 255),
            static_cast<int>(col[3] * 255)
        );
    }
    
    if (config.showGradient) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = config.pickerSize;
        
        ViceTheme::Instance().DrawGradientRect(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y),
                                               IM_COL32(255, 95, 155, 255), IM_COL32(255, 215, 0, 255), true);
        
        ImGui::Dummy(size);
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::ColorPicker4(const char* label, float color[4], ImGuiColorEditFlags flags) {
    return ImGui::ColorEdit4(label, color, flags | ImGuiColorEditFlags_NoLabel);
}

bool ViceWidgets::ColorPicker3(const char* label, float color[3], ImGuiColorEditFlags flags) {
    return ImGui::ColorEdit3(label, color, flags | ImGuiColorEditFlags_NoLabel);
}

bool ViceWidgets::GradientColorPicker(const char* label, ImU32& color1, ImU32& color2, 
                                       bool& horizontal, const ColorPickerConfig& config) {
    ImGui::PushID(label);
    
    bool changed = false;
    changed |= ColorPicker("##col1", color1, config);
    ImGui::SameLine();
    changed |= ColorPicker("##col2", color2, config);
    
    ImGui::Checkbox("Horizontal", &horizontal);
    
    ImGui::PopID();
    return changed;
}

// ========== Keybind ==========

bool ViceWidgets::Keybind(const char* label, int& key, KeybindMode& mode, const KeybindConfig& config) {
    ImGui::PushID(label);
    
    bool changed = false;
    
    std::string keyName = GetKeyName(key);
    if (keyName.empty()) keyName = config.noneText;
    
    ImVec2 btnSize = config.size;
    if (ImGui::Button(keyName.c_str(), btnSize)) {
        key = 0;
        mode = KeybindMode::Hold;
    }
    
    if (key == 0) {
        for (int k = 1; k < 256; ++k) {
            if (GetAsyncKeyState(k) & 0x8000) {
                if (k == VK_ESCAPE) {
                    key = 0;
                } else if (k != VK_INSERT) {
                    key = k;
                }
                break;
            }
        }
        if (key != 0) changed = true;
    }
    
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("KeybindModePopup");
    }
    
    KeybindModePopup("KeybindModePopup", mode);
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::KeybindEx(const char* label, int& key, KeybindMode& mode, bool& enabled, const KeybindConfig& config) {
    ImGui::PushID(label);
    
    bool changed = false;
    changed |= ImGui::Checkbox("##enable", &enabled);
    ImGui::SameLine();
    changed |= Keybind(label, key, mode, config);
    
    ImGui::PopID();
    return changed;
}

void ViceWidgets::KeybindModePopup(const char* id, KeybindMode& mode) {
    if (ImGui::BeginPopup(id)) {
        if (ImGui::Selectable("Toggle", mode == KeybindMode::Toggle)) mode = KeybindMode::Toggle;
        if (ImGui::Selectable("Hold", mode == KeybindMode::Hold)) mode = KeybindMode::Hold;
        if (ImGui::Selectable("Always", mode == KeybindMode::Always)) mode = KeybindMode::Always;
        if (ImGui::Selectable("Double Tap", mode == KeybindMode::DoubleTap)) mode = KeybindMode::DoubleTap;
        ImGui::EndPopup();
    }
}

// ========== Combo ==========

bool ViceWidgets::Combo(const char* label, int& current, const std::vector<std::string>& items, const ComboConfig& config) {
    ImGui::PushID(label);
    
    std::string preview = (current >= 0 && current < (int)items.size()) ? items[current] : config.previewText;
    if (preview.empty()) preview = config.noItemsText;
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    bool changed = false;
    
    if (ImGui::BeginCombo(label, preview.c_str(), ImGuiComboFlags_HeightLarge)) {
        for (size_t i = 0; i < items.size(); ++i) {
            bool selected = (current == (int)i);
            if (ImGui::Selectable(items[i].c_str(), selected)) {
                current = static_cast<int>(i);
                changed = true;
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::Combo(const char* label, int& current, const char* const* items, int itemsCount, const ComboConfig& config) {
    std::vector<std::string> vec(items, items + itemsCount);
    return Combo(label, current, vec, config);
}

bool ViceWidgets::MultiCombo(const char* label, std::vector<int>& selected, const std::vector<std::string>& items, const ComboConfig& config) {
    ImGui::PushID(label);
    
    std::string preview;
    for (int idx : selected) {
        if (idx >= 0 && idx < (int)items.size()) {
            if (!preview.empty()) preview += ", ";
            preview += items[idx];
        }
    }
    if (preview.empty()) preview = config.noItemsText;
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    bool changed = false;
    
    if (ImGui::BeginCombo(label, preview.c_str(), ImGuiComboFlags_HeightLarge)) {
        for (size_t i = 0; i < items.size(); ++i) {
            bool isSelected = std::find(selected.begin(), selected.end(), (int)i) != selected.end();
            if (ImGui::Selectable(items[i].c_str(), isSelected)) {
                if (isSelected) {
                    selected.erase(std::remove(selected.begin(), selected.end(), (int)i), selected.end());
                } else {
                    selected.push_back((int)i);
                }
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::MultiCombo(const char* label, std::vector<bool>& selected, const std::vector<std::string>& items, const ComboConfig& config) {
    std::vector<int> indices;
    for (size_t i = 0; i < selected.size(); ++i) {
        if (selected[i]) indices.push_back((int)i);
    }
    
    bool changed = MultiCombo(label, indices, items, config);
    
    std::fill(selected.begin(), selected.end(), false);
    for (int idx : indices) {
        if (idx >= 0 && idx < (int)selected.size()) selected[idx] = true;
    }
    
    return changed;
}

bool ViceWidgets::SearchableCombo(const char* label, int& current, const std::vector<std::string>& items, const ComboConfig& config) {
    ImGui::PushID(label);
    
    static std::string searchQuery;
    std::string preview = (current >= 0 && current < (int)items.size()) ? items[current] : config.previewText;
    if (preview.empty()) preview = config.noItemsText;
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    bool changed = false;
    
    if (ImGui::BeginCombo(label, preview.c_str(), ImGuiComboFlags_HeightLarge)) {
        ImGui::InputText("##search", &searchQuery);
        ImGui::Separator();
        
        for (size_t i = 0; i < items.size(); ++i) {
            if (!searchQuery.empty()) {
                std::string lowerItem = items[i];
                std::transform(lowerItem.begin(), lowerItem.end(), lowerItem.begin(), ::tolower);
                std::string lowerQuery = searchQuery;
                std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
                if (lowerItem.find(lowerQuery) == std::string::npos) continue;
            }
            
            bool selected = (current == (int)i);
            if (ImGui::Selectable(items[i].c_str(), selected)) {
                current = static_cast<int>(i);
                changed = true;
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::PopID();
    return changed;
}

// ========== Slider ==========

bool ViceWidgets::SliderFloat(const char* label, float& value, const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    char formatBuf[32];
    if (config.customFormat) {
        std::string fmt = config.customFormat(value);
        strcpy_s(formatBuf, fmt.c_str());
    } else {
        strcpy_s(formatBuf, config.format);
    }
    
    bool changed = ImGui::SliderFloat(label, &value, config.min, config.max, formatBuf);
    
    if (!config.tooltip.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", config.tooltip.c_str());
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::SliderInt(const char* label, int& value, const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    char formatBuf[32];
    if (config.customFormat) {
        std::string fmt = config.customFormat(value);
        strcpy_s(formatBuf, fmt.c_str());
    } else {
        strcpy_s(formatBuf, config.format);
    }
    
    bool changed = ImGui::SliderInt(label, &value, static_cast<int>(config.min), static_cast<int>(config.max), formatBuf);
    
    if (!config.tooltip.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", config.tooltip.c_str());
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::SliderFloat2(const char* label, float value[2], const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    bool changed = ImGui::SliderFloat2(label, value, config.min, config.max, config.format);
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::SliderFloat3(const char* label, float value[3], const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    bool changed = ImGui::SliderFloat3(label, value, config.min, config.max, config.format);
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::SliderFloat4(const char* label, float value[4], const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    bool changed = ImGui::SliderFloat4(label, value, config.min, config.max, config.format);
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::SliderFloatRange(const char* label, float& min, float& max, const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    bool changed = ImGui::SliderFloat2(label, &min, config.min, config.max, config.format);
    if (min > max) std::swap(min, max);
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::SliderIntRange(const char* label, int& min, int& max, const SliderConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    float fmin = static_cast<float>(min);
    float fmax = static_cast<float>(max);
    bool changed = ImGui::SliderFloat2(label, &fmin, config.min, config.max, config.format);
    min = static_cast<int>(fmin);
    max = static_cast<int>(fmax);
    if (min > max) std::swap(min, max);
    
    ImGui::PopID();
    return changed;
}

// ========== Tab Bar ==========

bool ViceWidgets::BeginTabBar(const char* id, const TabBarConfig& config) {
    ImGuiTabBarFlags flags = ImGuiTabBarFlags_None;
    if (config.reorderable) flags |= ImGuiTabBarFlags_Reorderable;
    if (config.closable) flags |= ImGuiTabBarFlags_TabListPopupButton;
    
    return ImGui::BeginTabBar(id, flags);
}

void ViceWidgets::EndTabBar() {
    ImGui::EndTabBar();
}

bool ViceWidgets::TabItem(const char* label, bool* open, ImGuiTabItemFlags flags) {
    return ImGui::BeginTabItem(label, open, flags);
}

bool ViceWidgets::TabItemEx(const char* label, bool* open, ImGuiTabItemFlags flags, ImU32 customColor) {
    ImGui::PushStyleColor(ImGuiCol_Tab, customColor);
    bool result = ImGui::BeginTabItem(label, open, flags);
    ImGui::PopStyleColor();
    return result;
}

bool ViceWidgets::BeginVerticalTabBar(const char* id, const TabBarConfig& config) {
    ImGui::BeginChild(id, ImVec2(200, 0), true);
    return true;
}

void ViceWidgets::EndVerticalTabBar() {
    ImGui::EndChild();
}

bool ViceWidgets::VerticalTabItem(const char* label, bool* open) {
    bool clicked = ImGui::Selectable(label, open ? *open : false, ImGuiSelectableFlags_None, ImVec2(-1, 30));
    if (open) *open = clicked;
    return clicked;
}

// ========== Button ==========

bool ViceWidgets::Button(const char* label, const ButtonConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0 && size.y == 0) {
        size = ImGui::CalcTextSize(label);
        size.x += ImGui::GetStyle().FramePadding.x * 2;
        size.y += ImGui::GetStyle().FramePadding.y * 2;
    }
    
    if (config.customColor != 0) ImGui::PushStyleColor(ImGuiCol_Button, config.customColor);
    if (config.customHoverColor != 0) ImGui::PushStyleColor(ImGuiCol_ButtonHovered, config.customHoverColor);
    if (config.customActiveColor != 0) ImGui::PushStyleColor(ImGuiCol_ButtonActive, config.customActiveColor);
    if (config.textColor != 0) ImGui::PushStyleColor(ImGuiCol_Text, config.textColor);
    if (config.rounding >= 0) ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, config.rounding);
    
    bool disabled = config.disabled;
    if (disabled) ImGui::BeginDisabled();
    
    bool clicked = ImGui::Button(label, size);
    
    if (disabled) ImGui::EndDisabled();
    if (config.rounding >= 0) ImGui::PopStyleVar();
    if (config.textColor != 0) ImGui::PopStyleColor();
    if (config.customActiveColor != 0) ImGui::PopStyleColor();
    if (config.customHoverColor != 0) ImGui::PopStyleColor();
    if (config.customColor != 0) ImGui::PopStyleColor();
    
    ImGui::PopID();
    return clicked;
}

bool ViceWidgets::SmallButton(const char* label, const ButtonConfig& config) {
    ImGui::PushID(label);
    
    if (config.customColor != 0) ImGui::PushStyleColor(ImGuiCol_Button, config.customColor);
    if (config.customHoverColor != 0) ImGui::PushStyleColor(ImGuiCol_ButtonHovered, config.customHoverColor);
    if (config.customActiveColor != 0) ImGui::PushStyleColor(ImGuiCol_ButtonActive, config.customActiveColor);
    if (config.textColor != 0) ImGui::PushStyleColor(ImGuiCol_Text, config.textColor);
    
    bool disabled = config.disabled;
    if (disabled) ImGui::BeginDisabled();
    
    bool clicked = ImGui::SmallButton(label);
    
    if (disabled) ImGui::EndDisabled();
    if (config.textColor != 0) ImGui::PopStyleColor();
    if (config.customActiveColor != 0) ImGui::PopStyleColor();
    if (config.customHoverColor != 0) ImGui::PopStyleColor();
    if (config.customColor != 0) ImGui::PopStyleColor();
    
    ImGui::PopID();
    return clicked;
}

bool ViceWidgets::InvisibleButton(const char* id, const ImVec2& size, const ButtonConfig& config) {
    return ImGui::InvisibleButton(id, size);
}

bool ViceWidgets::ArrowButton(const char* id, ImGuiDir dir, const ButtonConfig& config) {
    return ImGui::ArrowButton(id, dir);
}

bool ViceWidgets::ToggleButton(const char* label, bool& value, const ButtonConfig& config) {
    ButtonConfig btnConfig = config;
    if (value) {
        if (btnConfig.customColor == 0) btnConfig.customColor = ViceTheme::Instance().GetColors().PrimaryPink;
        if (btnConfig.customHoverColor == 0) btnConfig.customHoverColor = ViceTheme::Instance().GetColors().PinkSoft;
        if (btnConfig.customActiveColor == 0) btnConfig.customActiveColor = ViceTheme::Instance().GetColors().PrimaryPink;
    }
    
    bool clicked = Button(label, btnConfig);
    if (clicked) value = !value;
    return clicked;
}

// ========== Checkbox ==========

bool ViceWidgets::Checkbox(const char* label, bool& value, const CheckboxConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    ImGui::SetNextItemWidth(size.x);
    
    if (config.animated) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        float t = value ? 1.0f : 0.0f;
        
        ImU32 bgColor = config.bgColor != 0 ? config.bgColor : ViceTheme::Instance().GetColors().Checkbox_Bg;
        ImU32 checkColor = config.checkColor != 0 ? config.checkColor : ViceTheme::Instance().GetColors().Checkbox_Check;
        
        bool clicked = ImGui::InvisibleButton("##check", size);
        if (clicked) value = !value;
        
        float rounding = config.rounding;
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, rounding);
        
        if (value) {
            ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
            float checkSize = std::min(size.x, size.y) * 0.6f;
            drawList->AddLine(
                ImVec2(center.x - checkSize * 0.4f, center.y),
                ImVec2(center.x - checkSize * 0.1f, center.y + checkSize * 0.3f),
                checkColor, 2.0f
            );
            drawList->AddLine(
                ImVec2(center.x - checkSize * 0.1f, center.y + checkSize * 0.3f),
                ImVec2(center.x + checkSize * 0.4f, center.y - checkSize * 0.3f),
                checkColor, 2.0f
            );
        }
        
        ImGui::SameLine();
        ImGui::Text("%s", label);
        
        ImGui::PopID();
        return clicked;
    } else {
        bool changed = ImGui::Checkbox(label, &value);
        ImGui::PopID();
        return changed;
    }
}

bool ViceWidgets::CheckboxFlags(const char* label, int& flags, int flagValue, const CheckboxConfig& config) {
    bool value = (flags & flagValue) != 0;
    bool changed = Checkbox(label, value, config);
    if (changed) {
        if (value) flags |= flagValue;
        else flags &= ~flagValue;
    }
    return changed;
}

// ========== Input Text ==========

bool ViceWidgets::InputText(const char* label, std::string& value, const InputTextConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    char buffer[1024];
    strncpy_s(buffer, value.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    bool changed = ImGui::InputText(label, buffer, sizeof(buffer), config.flags);
    
    if (changed) {
        value = buffer;
    }
    
    if (!config.hint.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", config.hint.c_str());
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::InputTextMultiline(const char* label, std::string& value, const ImVec2& size, const InputTextConfig& config) {
    ImGui::PushID(label);
    
    char buffer[8192];
    strncpy_s(buffer, value.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    bool changed = ImGui::InputTextMultiline(label, buffer, sizeof(buffer), size, config.flags);
    
    if (changed) {
        value = buffer;
    }
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::InputInt(const char* label, int& value, const InputTextConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    bool changed = ImGui::InputInt(label, &value, 1, 100, config.flags);
    
    ImGui::PopID();
    return changed;
}

bool ViceWidgets::InputFloat(const char* label, float& value, const InputTextConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(size.x);
    
    bool changed = ImGui::InputFloat(label, &value, 0.1f, 1.0f, "%.3f", config.flags);
    
    ImGui::PopID();
    return changed;
}

// ========== Tooltip ==========

void ViceWidgets::Tooltip(const char* text) {
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", text);
    }
}

void ViceWidgets::TooltipBegin() {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
    }
}

void ViceWidgets::TooltipEnd() {
    ImGui::EndTooltip();
}

void ViceWidgets::TooltipEx(const char* title, const char* description, const char* shortcut) {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", title);
        if (description && *description) {
            ImGui::TextDisabled("%s", description);
        }
        if (shortcut && *shortcut) {
            ImGui::Separator();
            ImGui::TextDisabled("Shortcut: %s", shortcut);
        }
        ImGui::EndTooltip();
    }
}

// ========== Separator ==========

void ViceWidgets::Separator(const char* label) {
    if (label && *label) {
        ImGui::SeparatorText(label);
    } else {
        ImGui::Separator();
    }
}

void ViceWidgets::SeparatorText(const char* label) {
    ImGui::SeparatorText(label);
}

void ViceWidgets::SeparatorEx(ImGuiSeparatorFlags flags) {
    ImGui::SeparatorEx(flags);
}

// ========== Group / Panel ==========

void ViceWidgets::BeginPanel(const char* id, const PanelConfig& config) {
    ImGui::PushID(id);
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    if (size.y == 0) size.y = ImGui::GetContentRegionAvail().y;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 bgColor = config.bgColor != 0 ? config.bgColor : ViceTheme::Instance().GetColors().BG_Card;
    ImU32 borderColor = config.borderColor != 0 ? config.borderColor : ViceTheme::Instance().GetColors().Border_Dim;
    float rounding = config.rounding >= 0 ? config.rounding : ViceTheme::Instance().GetStyle().FrameRounding;
    
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, rounding);
    if (config.bordered) {
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, rounding);
    }
    
    ImGui::SetCursorScreenPos(ImVec2(pos.x + config.padding, pos.y + config.padding));
    ImGui::BeginGroup();
}

void ViceWidgets::EndPanel() {
    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Dummy(ImVec2(0, 8));
}

void ViceWidgets::BeginGroupPanel(const char* label, const ImVec2& size) {
    ImGui::BeginGroup();
    ImGui::BeginChild(label, size, true);
}

void ViceWidgets::EndGroupPanel() {
    ImGui::EndChild();
    ImGui::EndGroup();
}

// ========== Progress Bar ==========

void ViceWidgets::ProgressBar(float fraction, const ProgressConfig& config) {
    ImGui::PushID("##progress");
    
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    
    ImVec2 size = config.size;
    if (size.x == 0) size.x = ImGui::GetContentRegionAvail().x;
    if (size.y == 0) size.y = 20.0f;
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    ImU32 bgColor = config.bgColor != 0 ? config.bgColor : ViceTheme::Instance().GetColors().Progress_Bg;
    ImU32 fillColor = config.fillColor != 0 ? config.fillColor : ViceTheme::Instance().GetColors().Progress_Fill;
    
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, config.rounding);
    
    float fillWidth = size.x * fraction;
    if (fillWidth > 0) {
        drawList->AddRectFilled(pos, ImVec2(pos.x + fillWidth, pos.y + size.y), fillColor, config.rounding);
    }
    
    if (config.showText) {
        char text[64];
        sprintf_s(text, config.format.c_str(), fraction * 100.0f);
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos = ImVec2(pos.x + (size.x - textSize.x) * 0.5f, pos.y + (size.y - textSize.y) * 0.5f);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), text);
    }
    
    ImGui::Dummy(size);
    ImGui::PopID();
}

void ViceWidgets::ProgressBarEx(float fraction, const char* overlay, const ProgressConfig& config) {
    ProgressConfig cfg = config;
    ProgressBar(fraction, cfg);
    
    if (overlay && *overlay) {
        ImGui::SameLine();
        ImGui::Text("%s", overlay);
    }
}

// ========== Spinner ==========

void ViceWidgets::Spinner(const char* id, float radius, int thickness, ImU32 color, float speed) {
    ImGui::PushID(id);
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
    
    float time = ImGui::GetTime() * speed;
    int segments = 12;
    
    for (int i = 0; i < segments; ++i) {
        float angle = time + i * (2.0f * 3.14159f / segments);
        float alpha = 1.0f - static_cast<float>(i) / segments * 0.8f;
        
        ImVec2 p1 = ImVec2(center.x + cosf(angle) * (radius - thickness * 0.5f),
                          center.y + sinf(angle) * (radius - thickness * 0.5f));
        ImVec2 p2 = ImVec2(center.x + cosf(angle) * (radius + thickness * 0.5f),
                          center.y + sinf(angle) * (radius + thickness * 0.5f));
        
        ImU32 c = (color & 0x00FFFFFF) | (static_cast<uint8_t>(255 * alpha) << 24);
        drawList->AddLine(p1, p2, c, static_cast<float>(thickness));
    }
    
    ImGui::Dummy(ImVec2(radius * 2, radius * 2));
    ImGui::PopID();
}

void ViceWidgets::SpinnerEx(const char* id, float radius, int thickness, ImU32 color, int segments, float speed) {
    Spinner(id, radius, thickness, color, speed);
}

// ========== Notification ==========

void ViceWidgets::PushNotification(const Notification& notification) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_notifications.push_back(notification);
    if (m_notifications.size() > 10) {
        m_notifications.erase(m_notifications.begin());
    }
}

void ViceWidgets::PushNotification(const std::string& title, const std::string& message, NotificationType type, float duration) {
    Notification notif;
    notif.title = title;
    notif.message = message;
    notif.type = type;
    notif.duration = duration;
    notif.elapsed = 0.0f;
    PushNotification(notif);
}

void ViceWidgets::RenderNotifications() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    float dt = ImGui::GetIO().DeltaTime;
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    ImVec2 pos = ImVec2(viewportSize.x - 320, 20);
    
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        Notification& notif = *it;
        notif.elapsed += dt;
        
        if (notif.elapsed >= notif.duration) {
            it = m_notifications.erase(it);
            continue;
        }
        
        ImU32 bgColor, borderColor;
        switch (notif.type) {
            case NotificationType::Success:
                bgColor = IM_COL32(40, 80, 40, 240);
                borderColor = IM_COL32(80, 255, 80, 255);
                break;
            case NotificationType::Warning:
                bgColor = IM_COL32(80, 60, 20, 240);
                borderColor = IM_COL32(255, 215, 0, 255);
                break;
            case NotificationType::Error:
                bgColor = IM_COL32(80, 30, 30, 240);
                borderColor = IM_COL32(255, 80, 80, 255);
                break;
            default:
                bgColor = IM_COL32(30, 40, 80, 240);
                borderColor = IM_COL32(95, 155, 255, 255);
                break;
        }
        
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 textSize = ImGui::CalcTextSize(notif.title.c_str());
        ImVec2 msgSize = ImGui::CalcTextSize(notif.message.c_str());
        float width = std::max({300.0f, textSize.x + 20, msgSize.x + 20});
        float height = 50 + msgSize.y;
        
        ImVec2 nPos = ImVec2(pos.x - width, pos.y);
        ImVec2 nMax = ImVec2(pos.x, pos.y + height);
        
        drawList->AddRectFilled(nPos, nMax, bgColor, 6.0f);
        drawList->AddRect(nPos, nMax, borderColor, 6.0f);
        
        drawList->AddText(ImVec2(nPos.x + 10, nPos.y + 8), IM_COL32(255, 255, 255, 255), notif.title.c_str());
        drawList->AddText(ImVec2(nPos.x + 10, nPos.y + 28), IM_COL32(200, 200, 200, 255), notif.message.c_str());
        
        float progress = 1.0f - notif.elapsed / notif.duration;
        drawList->AddRectFilled(
            ImVec2(nPos.x, nMax.y - 3),
            ImVec2(nPos.x + width * progress, nMax.y),
            borderColor, 0.0f, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight
        );
        
        pos.y += height + 8;
        ++it;
    }
}

void ViceWidgets::ClearNotifications() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_notifications.clear();
}

// ========== Knob ==========

bool ViceWidgets::Knob(const char* label, float& value, const KnobConfig& config) {
    ImGui::PushID(label);
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(pos.x + config.radius, pos.y + config.radius);
    
    bool changed = false;
    bool hovered = false;
    bool held = false;
    
    ImGui::InvisibleButton("##knob", ImVec2(config.radius * 2, config.radius * 2));
    hovered = ImGui::IsItemHovered();
    held = ImGui::IsItemActive();
    
    if (held && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        float angleChange = (delta.x - delta.y) * 0.01f;
        value += angleChange * (config.max - config.min);
        value = std::clamp(value, config.min, config.max);
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        changed = true;
    }
    
    ImU32 bgColor = config.bgColor != 0 ? config.bgColor : ViceTheme::Instance().GetColors().BG_Card;
    ImU32 trackColor = config.trackColor != 0 ? config.trackColor : ViceTheme::Instance().GetColors().Border_Dim;
    ImU32 indicatorColor = config.indicatorColor != 0 ? config.indicatorColor : ViceTheme::Instance().GetColors().PrimaryPink;
    
    drawList->AddCircleFilled(center, config.radius, bgColor, 32);
    drawList->AddCircle(center, config.radius, trackColor, 32, static_cast<float>(thickness));
    
    float t = (value - config.min) / (config.max - config.min);
    float startAngle = -3.14159f * 0.75f;
    float endAngle = startAngle + t * 3.14159f * 1.5f;
    
    const int arcSegments = 32;
    for (int i = 0; i < arcSegments; ++i) {
        float a1 = startAngle + (endAngle - startAngle) * i / arcSegments;
        float a2 = startAngle + (endAngle - startAngle) * (i + 1) / arcSegments;
        
        ImVec2 p1 = ImVec2(center.x + cosf(a1) * (config.radius - thickness),
                          center.y + sinf(a1) * (config.radius - thickness));
        ImVec2 p2 = ImVec2(center.x + cosf(a2) * (config.radius - thickness),
                          center.y + sinf(a2) * (config.radius - thickness));
        
        drawList->AddLine(p1, p2, indicatorColor, static_cast<float>(thickness));
    }
    
    float indicatorAngle = startAngle + t * 3.14159f * 1.5f;
    ImVec2 indicatorPos = ImVec2(center.x + cosf(indicatorAngle) * (config.radius - thickness * 2),
                                 center.y + sinf(indicatorAngle) * (config.radius - thickness * 2));
    drawList->AddCircleFilled(indicatorPos, thickness * 1.5f, indicatorColor);
    
    if (config.showValue) {
        char text[32];
        sprintf_s(text, config.format.c_str(), value);
        ImVec2 textSize = ImGui::CalcTextSize(text);
        drawList->AddText(
            ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f),
            IM_COL32(255, 255, 255, 255), text
        );
    }
    
    if (label && *label) {
        ImGui::Dummy(ImVec2(config.radius * 2, config.radius * 2 + 20));
        ImGui::Text("%s", label);
    } else {
        ImGui::Dummy(ImVec2(config.radius * 2, config.radius * 2));
    }
    
    ImGui::PopID();
    return changed;
}

// ========== Color Preview ==========

void ViceWidgets::ColorPreview(ImU32 color, const ImVec2& size, bool border, float rounding) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), color, rounding);
    
    if (border) {
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                         ViceTheme::Instance().GetColors().Border_Dim, rounding);
    }
    
    ImGui::Dummy(size);
}

void ViceWidgets::ColorPreviewWithAlpha(ImU32 color, const ImVec2& size, bool border, float rounding) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    const int checkerSize = 8;
    for (int x = 0; x < size.x; x += checkerSize) {
        for (int y = 0; y < size.y; y += checkerSize) {
            bool dark = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            ImU32 c = dark ? IM_COL32(180, 180, 180, 255) : IM_COL32(220, 220, 220, 255);
            drawList->AddRectFilled(
                ImVec2(pos.x + x, pos.y + y),
                ImVec2(pos.x + x + checkerSize, pos.y + y + checkerSize),
                c
            );
        }
    }
    
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), color, rounding);
    
    if (border) {
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                         ViceTheme::Instance().GetColors().Border_Dim, rounding);
    }
    
    ImGui::Dummy(size);
}

// ========== Icon Button ==========

bool ViceWidgets::IconButton(const char* icon, const char* tooltip, const ButtonConfig& config) {
    ButtonConfig btnConfig = config;
    if (btnConfig.size.x == 0) btnConfig.size = ImVec2(32, 32);
    
    bool clicked = Button(icon, btnConfig);
    
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    
    return clicked;
}

// ========== Search Bar ==========

bool ViceWidgets::SearchBar(const char* id, std::string& query, const char* placeholder, const ImVec2& size) {
    ImGui::PushID(id);
    
    ImVec2 sz = size;
    if (sz.x == 0) sz.x = ImGui::GetContentRegionAvail().x;
    
    ImGui::SetNextItemWidth(sz.x);
    
    char buffer[256];
    strncpy_s(buffer, query.c_str(), sizeof(buffer) - 1);
    
    bool changed = ImGui::InputTextWithHint("##search", placeholder, buffer, sizeof(buffer));
    
    if (changed) {
        query = buffer;
    }
    
    if (!query.empty()) {
        ImGui::SameLine(0, 4);
        if (ImGui::Button("X", ImVec2(24, 0))) {
            query.clear();
            changed = true;
        }
    }
    
    ImGui::PopID();
    return changed;
}

// ========== Property Grid ==========

void ViceWidgets::BeginPropertyGrid(const char* id, const PropertyConfig& config) {
    ImGui::PushID(id);
    ImGui::Columns(2, "##props", false);
    ImGui::SetColumnWidth(0, config.labelWidth);
    ImGui::SetColumnWidth(1, -1);
}

void ViceWidgets::EndPropertyGrid() {
    ImGui::Columns(1);
    ImGui::PopID();
}

bool ViceWidgets::Property(const char* label, bool& value) {
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    bool changed = ImGui::Checkbox("##prop", &value);
    ImGui::NextColumn();
    return changed;
}

bool ViceWidgets::Property(const char* label, int& value) {
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    bool changed = ImGui::InputInt("##prop", &value);
    ImGui::NextColumn();
    return changed;
}

bool ViceWidgets::Property(const char* label, float& value) {
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    bool changed = ImGui::InputFloat("##prop", &value);
    ImGui::NextColumn();
    return changed;
}

bool ViceWidgets::Property(const char* label, std::string& value) {
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    bool changed = ImGui::InputText("##prop", &value);
    ImGui::NextColumn();
    return changed;
}

bool ViceWidgets::Property(const char* label, ImU32& color) {
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    bool changed = ImGui::ColorEdit4("##prop", reinterpret_cast<float*>(&color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    ImGui::NextColumn();
    return changed;
}

bool ViceWidgets::Property(const char* label, int& value, const std::vector<std::string>& items) {
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    bool changed = ImGui::Combo("##prop", &value, items.data(), static_cast<int>(items.size()));
    ImGui::NextColumn();
    return changed;
}

// ========== Collapsible Section ==========

bool ViceWidgets::CollapsibleSection(const char* label, bool& open, bool defaultOpen) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (defaultOpen && !open) open = true;
    if (open) flags |= ImGuiTreeNodeFlags_DefaultOpen;
    
    bool clicked = ImGui::CollapsingHeader(label, flags);
    if (clicked != open) {
        open = clicked;
        return true;
    }
    return false;
}

// ========== Hotkey Display ==========

void ViceWidgets::HotkeyDisplay(int key, KeybindMode mode, const ImVec2& size) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    std::string keyName = GetKeyName(key);
    std::string modeStr;
    switch (mode) {
        case KeybindMode::Toggle: modeStr = "[T]"; break;
        case KeybindMode::Hold: modeStr = "[H]"; break;
        case KeybindMode::Always: modeStr = "[A]"; break;
        case KeybindMode::DoubleTap: modeStr = "[2x]"; break;
    }
    
    std::string text = keyName + " " + modeStr;
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    
    ImVec2 btnSize = size;
    if (btnSize.x == 0) btnSize.x = textSize.x + 16;
    if (btnSize.y == 0) btnSize.y = textSize.y + 8;
    
    ImU32 bgColor = ViceTheme::Instance().GetColors().BG_Card;
    ImU32 borderColor = ViceTheme::Instance().GetColors().Border_Dim;
    ImU32 textColor = ViceTheme::Instance().GetColors().Text_Main;
    
    drawList->AddRectFilled(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y), bgColor, 4.0f);
    drawList->AddRect(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y), borderColor, 4.0f);
    drawList->AddText(ImVec2(pos.x + 8, pos.y + (btnSize.y - textSize.y) * 0.5f), textColor, text.c_str());
    
    ImGui::Dummy(btnSize);
}

std::string ViceWidgets::GetKeyName(int key) const {
    switch (key) {
        case VK_LBUTTON: return "M1";
        case VK_RBUTTON: return "M2";
        case VK_MBUTTON: return "M3";
        case VK_XBUTTON1: return "M4";
        case VK_XBUTTON2: return "M5";
        case VK_BACK: return "Backspace";
        case VK_TAB: return "Tab";
        case VK_RETURN: return "Enter";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_CAPITAL: return "Caps";
        case VK_ESCAPE: return "Esc";
        case VK_SPACE: return "Space";
        case VK_PRIOR: return "PgUp";
        case VK_NEXT: return "PgDn";
        case VK_END: return "End";
        case VK_HOME: return "Home";
        case VK_LEFT: return "Left";
        case VK_UP: return "Up";
        case VK_RIGHT: return "Right";
        case VK_DOWN: return "Down";
        case VK_INSERT: return "Ins";
        case VK_DELETE: return "Del";
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_F7: return "F7";
        case VK_F8: return "F8";
        case VK_F9: return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        case VK_NUMPAD0: return "Num0";
        case VK_NUMPAD1: return "Num1";
        case VK_NUMPAD2: return "Num2";
        case VK_NUMPAD3: return "Num3";
        case VK_NUMPAD4: return "Num4";
        case VK_NUMPAD5: return "Num5";
        case VK_NUMPAD6: return "Num6";
        case VK_NUMPAD7: return "Num7";
        case VK_NUMPAD8: return "Num8";
        case VK_NUMPAD9: return "Num9";
        case VK_MULTIPLY: return "Num*";
        case VK_ADD: return "Num+";
        case VK_SUBTRACT: return "Num-";
        case VK_DECIMAL: return "Num.";
        case VK_DIVIDE: return "Num/";
        default:
            if (key >= 'A' && key <= 'Z') {
                return std::string(1, static_cast<char>(key));
            }
            if (key >= '0' && key <= '9') {
                return std::string(1, static_cast<char>(key));
            }
            return "";
    }
}

bool ViceWidgets::IsKeyPressed(int key) const {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

ImU32 ViceWidgets::GetKeybindModeColor(KeybindMode mode) const {
    switch (mode) {
        case KeybindMode::Toggle: return IM_COL32(255, 215, 0, 255);
        case KeybindMode::Hold: return IM_COL32(255, 95, 155, 255);
        case KeybindMode::Always: return IM_COL32(80, 255, 120, 255);
        case KeybindMode::DoubleTap: return IM_COL32(255, 140, 180, 255);
    }
    return IM_COL32(255, 255, 255, 255);
}

} // namespace gui