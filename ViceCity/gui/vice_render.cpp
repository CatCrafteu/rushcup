#include "core/pch.hpp"
#include "gui/vice_render.hpp"
#include "gui/vice_theme.hpp"
#include "gui/vice_widgets.hpp"

namespace gui {

void ViceRender::Initialize() {
    LOG_INFO(GUI, "ViceRender initialized");
}

void ViceRender::Shutdown() {
    LOG_INFO(GUI, "ViceRender shutdown");
}

void ViceRender::NewFrame() {
    m_deltaTime = ImGui::GetIO().DeltaTime;
    m_time = ImGui::GetTime();
    UpdateAnimations();
}

void ViceRender::EndFrame() {
    // End of frame cleanup if needed
}

void ViceRender::UpdateAnimations() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto it = m_animations.begin(); it != m_animations.end();) {
        AnimationState& state = it->second;
        float dt = m_deltaTime;
        
        // Update animation
        float target = state.target;
        float current = state.value;
        
        if (std::abs(target - current) < 0.001f) {
            state.value = target;
            state.velocity = 0;
            // Keep animation for a bit then remove
            if (state.config.duration > 0) {
                // Animation complete, remove after one frame
                it = m_animations.erase(it);
                continue;
            }
        } else {
            // Spring animation
            float stiffness = 1.0f / (state.config.duration * state.config.duration);
            float damping = 2.0f * sqrtf(stiffness) * 0.8f;
            
            float force = stiffness * (target - current);
            float dampingForce = -damping * state.velocity;
            float acceleration = force + dampingForce;
            
            state.velocity += acceleration * dt;
            state.value += state.velocity * dt;
            
            // Apply easing
            float progress = std::abs(state.value - current) / std::abs(target - current);
            progress = std::clamp(progress, 0.0f, 1.0f);
            float eased = ApplyEasing(progress, state.config.easing);
            
            if (target > current) {
                state.value = current + (target - current) * eased;
            } else {
                state.value = current - (current - target) * eased;
            }
        }
        
        ++it;
    }
}

float ViceRender::AnimFloat(const char* id, float target, const AnimationConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto& state = m_animations[id];
    state.target = target;
    state.config = config;
    
    if (state.value == 0.0f && state.target != 0.0f) {
        state.value = target; // Instant first frame
    }
    
    return state.value;
}

float ViceRender::AnimFloatClamped(const char* id, float target, float min, float max, const AnimationConfig& config) {
    float value = AnimFloat(id, target, config);
    return std::clamp(value, min, max);
}

ImU32 ViceRender::AnimColor(const char* id, ImU32 target, const AnimationConfig& config) {
    std::string idR = id + std::string("_r");
    std::string idG = id + std::string("_g");
    std::string idB = id + std::string("_b");
    std::string idA = id + std::string("_a");
    
    float r = AnimFloat(idR.c_str(), IM_COL32_R(target) / 255.0f, config);
    float g = AnimFloat(idG.c_str(), IM_COL32_G(target) / 255.0f, config);
    float b = AnimFloat(idB.c_str(), IM_COL32_B(target) / 255.0f, config);
    float a = AnimFloat(idA.c_str(), IM_COL32_A(target) / 255.0f, config);
    
    return IM_COL32(
        static_cast<int>(r * 255),
        static_cast<int>(g * 255),
        static_cast<int>(b * 255),
        static_cast<int>(a * 255)
    );
}

ImVec2 ViceRender::AnimVec2(const char* id, const ImVec2& target, const AnimationConfig& config) {
    std::string idX = id + std::string("_x");
    std::string idY = id + std::string("_y");
    
    float x = AnimFloat(idX.c_str(), target.x, config);
    float y = AnimFloat(idY.c_str(), target.y, config);
    
    return ImVec2(x, y);
}

float ViceRender::AnimVisibility(const char* id, bool visible, const AnimationConfig& config) {
    return AnimFloat(id, visible ? 1.0f : 0.0f, config);
}

float ViceRender::AnimProgress(const char* id, float target, const AnimationConfig& config) {
    return AnimFloatClamped(id, target, 0.0f, 1.0f, config);
}

float ViceRender::GetAnimationValue(AnimationState& state) {
    float dt = m_deltaTime;
    
    float target = state.target;
    float current = state.value;
    
    if (std::abs(target - current) < 0.001f) {
        state.value = target;
        state.velocity = 0;
        return target;
    }
    
    // Spring animation
    float stiffness = 1.0f / (state.config.duration * state.config.duration);
    float damping = 2.0f * sqrtf(stiffness) * 0.8f;
    
    float force = stiffness * (target - current);
    float dampingForce = -damping * state.velocity;
    float acceleration = force + dampingForce;
    
    state.velocity += acceleration * dt;
    state.value += state.velocity * dt;
    
    // Apply easing
    float progress = std::abs(state.value - current) / std::abs(target - current);
    progress = std::clamp(progress, 0.0f, 1.0f);
    float eased = ApplyEasing(progress, state.config.easing);
    
    if (target > current) {
        state.value = current + (target - current) * eased;
    } else {
        state.value = current - (current - target) * eased;
    }
    
    return state.value;
}

float ViceRender::LerpColor(ImU32 a, ImU32 b, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    uint8_t r = static_cast<uint8_t>(IM_COL32_R(a) + (IM_COL32_R(b) - IM_COL32_R(a)) * t);
    uint8_t g = static_cast<uint8_t>(IM_COL32_G(a) + (IM_COL32_G(b) - IM_COL32_G(a)) * t);
    uint8_t bl = static_cast<uint8_t>(IM_COL32_B(a) + (IM_COL32_B(b) - IM_COL32_B(a)) * t);
    uint8_t al = static_cast<uint8_t>(IM_COL32_A(a) + (IM_COL32_A(b) - IM_COL32_A(a)) * t);
    return IM_COL32(r, g, bl, al);
}

// ========== Blur Effects ==========

void ViceRender::PushBlur(const BlurConfig& config) {
    // Blur is implemented per-draw-call in DrawBlurRect/Background
    // This is a placeholder for future global blur stack
}

void ViceRender::PopBlur() {
    // Placeholder
}

void ViceRender::DrawBlurRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, 
                               float rounding, const BlurConfig& config) {
    // Since we can't do real blur in immediate mode easily,
    // we simulate with semi-transparent dark overlay
    ImU32 tintColor = config.tintColor != 0 ? config.tintColor : IM_COL32(0, 0, 0, static_cast<int>(120 * config.tintStrength));
    
    drawList->AddRectFilled(pMin, pMax, tintColor, rounding);
}

void ViceRender::DrawBlurBackground(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding) {
    ImU32 color = IM_COL32(0, 0, 0, 180);
    drawList->AddRectFilled(pMin, pMax, color, rounding);
}

// ========== Shadows ==========

void ViceRender::DrawShadowRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, 
                                 float rounding, const ShadowConfig& config) {
    ImVec2 shadowMin = ImVec2(pMin.x + config.offset.x - config.spread, pMin.y + config.offset.y - config.spread);
    ImVec2 shadowMax = ImVec2(pMax.x + config.offset.x + config.spread, pMax.y + config.offset.y + config.spread);
    
    // Multi-layer shadow for softer look
    for (int i = 0; i < 4; ++i) {
        float t = static_cast<float>(i) / 4.0f;
        float layerBlur = config.blurRadius * (1.0f - t * 0.5f);
        uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(config.color) * (1.0f - t) * 0.5f);
        
        ImU32 layerColor = (config.color & 0x00FFFFFF) | (alpha << 24);
        
        ImVec2 lMin = ImVec2(shadowMin.x - layerBlur, shadowMin.y - layerBlur);
        ImVec2 lMax = ImVec2(shadowMax.x + layerBlur, shadowMax.y + layerBlur);
        
        if (!config.onlyRect) {
            drawList->AddRectFilled(lMin, lMax, layerColor, rounding + layerBlur);
        }
    }
}

void ViceRender::DrawShadowCircle(ImDrawList* drawList, const ImVec2& center, float radius, const ShadowConfig& config) {
    for (int i = 0; i < 4; ++i) {
        float t = static_cast<float>(i) / 4.0f;
        float layerBlur = config.blurRadius * (1.0f - t * 0.5f);
        uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(config.color) * (1.0f - t) * 0.5f);
        
        ImU32 layerColor = (config.color & 0x00FFFFFF) | (alpha << 24);
        
        drawList->AddCircleFilled(
            ImVec2(center.x + config.offset.x, center.y + config.offset.y),
            radius + layerBlur, layerColor, 32
        );
    }
}

void ViceRender::DrawShadowPolygon(ImDrawList* drawList, const ImVec2* points, int count, const ShadowConfig& config) {
    // Simplified: just draw filled polygon with shadow color offset
    std::vector<ImVec2> shadowPoints(count);
    for (int i = 0; i < count; ++i) {
        shadowPoints[i] = ImVec2(points[i].x + config.offset.x, points[i].y + config.offset.y);
    }
    
    ImU32 shadowColor = config.color;
    drawList->AddConvexPolyFilled(shadowPoints.data(), count, shadowColor);
}

void ViceRender::DrawLayeredShadow(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, int layers) {
    ShadowConfig config;
    config.color = IM_COL32(0, 0, 0, 60);
    config.blurRadius = 16.0f;
    config.offset = ImVec2(0, 4);
    
    for (int i = 0; i < layers; ++i) {
        float t = static_cast<float>(i) / layers;
        float blur = config.blurRadius * (1.0f - t * 0.3f);
        uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(config.color) * (1.0f - t) * 0.6f);
        
        ImU32 color = (config.color & 0x00FFFFFF) | (alpha << 24);
        
        ImVec2 lMin = ImVec2(pMin.x - blur, pMin.y - blur + config.offset.y * t);
        ImVec2 lMax = ImVec2(pMax.x + blur, pMax.y + blur + config.offset.y * t);
        
        drawList->AddRectFilled(lMin, lMax, color, rounding + blur);
    }
}

// ========== Rounded Rectangles ==========

void ViceRender::DrawRoundedRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, const RoundedRectConfig& config) {
    ImU32 fillColor = config.fillColor != 0 ? config.fillColor : ViceTheme::Instance().GetColors().BG_Card;
    ImU32 borderColor = config.borderColor != 0 ? config.borderColor : ViceTheme::Instance().GetColors().Border_Dim;
    
    if (config.filled) {
        drawList->AddRectFilled(pMin, pMax, fillColor, config.rounding, config.flags);
    }
    
    if (config.thickness > 0) {
        drawList->AddRect(pMin, pMax, borderColor, config.rounding, config.flags, config.thickness);
    }
    
    // Shadow
    if (config.shadowBlur > 0) {
        DrawShadowRect(drawList, pMin, pMax, config.rounding, {
            config.shadowOffset,
            config.shadowBlur,
            config.shadowColor,
            0.0f
        });
    }
}

void ViceRender::DrawRoundedRectFilled(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 color, ImDrawFlags flags) {
    drawList->AddRectFilled(pMin, pMax, color, rounding, flags);
}

void ViceRender::DrawRoundedRectOutlined(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 color, float thickness, ImDrawFlags flags) {
    drawList->AddRect(pMin, pMax, color, rounding, flags, thickness);
}

void ViceRender::DrawRoundedRectGradient(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 col1, ImU32 col2, bool horizontal, ImDrawFlags flags) {
    // Use multi-color rect for gradient
    if (horizontal) {
        drawList->AddRectFilledMultiColor(pMin, pMax, col1, col2, col2, col1, rounding, flags);
    } else {
        drawList->AddRectFilledMultiColor(pMin, pMax, col1, col1, col2, col2, rounding, flags);
    }
}

void ViceRender::DrawRoundedRectMultiColor(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 colTL, ImU32 colTR, ImU32 colBR, ImU32 colBL, ImDrawFlags flags) {
    drawList->AddRectFilledMultiColor(pMin, pMax, colTL, colTR, colBR, colBL, rounding, flags);
}

// ========== Custom Shapes ==========

void ViceRender::DrawRoundedTriangle(ImDrawList* drawList, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float rounding, ImU32 color, bool filled, float thickness) {
    // Approximate with path
    ImVec2 center = ImVec2((p1.x + p2.x + p3.x) / 3, (p1.y + p2.y + p3.y) / 3);
    
    ImVec2 dir1 = ImVec2(p1.x - center.x, p1.y - center.y);
    ImVec2 dir2 = ImVec2(p2.x - center.x, p2.y - center.y);
    ImVec2 dir3 = ImVec2(p3.x - center.x, p3.y - center.y);
    
    float len1 = sqrtf(dir1.x * dir1.x + dir1.y * dir1.y);
    float len2 = sqrtf(dir2.x * dir2.x + dir2.y * dir2.y);
    float len3 = sqrtf(dir3.x * dir3.x + dir3.y * dir3.y);
    
    if (len1 > 0) dir1 = ImVec2(dir1.x / len1 * rounding, dir1.y / len1 * rounding);
    if (len2 > 0) dir2 = ImVec2(dir2.x / len2 * rounding, dir2.y / len2 * rounding);
    if (len3 > 0) dir3 = ImVec2(dir3.x / len3 * rounding, dir3.y / len3 * rounding);
    
    ImVec2 rp1 = ImVec2(p1.x - dir1.x, p1.y - dir1.y);
    ImVec2 rp2 = ImVec2(p2.x - dir2.x, p2.y - dir2.y);
    ImVec2 rp3 = ImVec2(p3.x - dir3.x, p3.y - dir3.y);
    
    if (filled) {
        drawList->AddTriangleFilled(rp1, rp2, rp3, color);
    } else {
        drawList->AddTriangle(rp1, rp2, rp3, color, thickness);
    }
}

void ViceRender::DrawRoundedPolygon(ImDrawList* drawList, const ImVec2* points, int count, float rounding, ImU32 color, bool filled, float thickness) {
    // Simplified: just draw regular polygon
    if (filled) {
        drawList->AddConvexPolyFilled(points, count, color);
    } else {
        for (int i = 0; i < count; ++i) {
            drawList->AddLine(points[i], points[(i + 1) % count], color, thickness);
        }
    }
}

void ViceRender::DrawPie(ImDrawList* drawList, const ImVec2& center, float radius, float startAngle, float endAngle, ImU32 color, bool filled, int segments) {
    if (filled) {
        std::vector<ImVec2> vertices;
        vertices.push_back(center);
        
        for (int i = 0; i <= segments; ++i) {
            float angle = startAngle + (endAngle - startAngle) * i / segments;
            vertices.push_back(ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius));
        }
        
        drawList->AddConvexPolyFilled(vertices.data(), static_cast<int>(vertices.size()), color);
    } else {
        for (int i = 0; i < segments; ++i) {
            float a1 = startAngle + (endAngle - startAngle) * i / segments;
            float a2 = startAngle + (endAngle - startAngle) * (i + 1) / segments;
            
            ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
            ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
            
            drawList->AddLine(p1, p2, color, 1.0f);
        }
    }
}

void ViceRender::DrawArc(ImDrawList* drawList, const ImVec2& center, float radius, float startAngle, float endAngle, ImU32 color, float thickness, int segments) {
    for (int i = 0; i < segments; ++i) {
        float a1 = startAngle + (endAngle - startAngle) * i / segments;
        float a2 = startAngle + (endAngle - startAngle) * (i + 1) / segments;
        
        ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
        ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
        
        drawList->AddLine(p1, p2, color, thickness);
    }
}

void ViceRender::DrawRing(ImDrawList* drawList, const ImVec2& center, float innerRadius, float outerRadius, float startAngle, float endAngle, ImU32 color, int segments) {
    for (int i = 0; i < segments; ++i) {
        float a1 = startAngle + (endAngle - startAngle) * i / segments;
        float a2 = startAngle + (endAngle - startAngle) * (i + 1) / segments;
        
        ImVec2 p1i = ImVec2(center.x + cosf(a1) * innerRadius, center.y + sinf(a1) * innerRadius);
        ImVec2 p1o = ImVec2(center.x + cosf(a1) * outerRadius, center.y + sinf(a1) * outerRadius);
        ImVec2 p2i = ImVec2(center.x + cosf(a2) * innerRadius, center.y + sinf(a2) * innerRadius);
        ImVec2 p2o = ImVec2(center.x + cosf(a2) * outerRadius, center.y + sinf(a2) * outerRadius);
        
        drawList->AddQuadFilled(p1i, p1o, p2o, p2i, color);
    }
}

// ========== Text Effects ==========

void ViceRender::DrawTextEx(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, const char* textEnd, float wrapWidth, const TextEffectConfig& config) {
    if (!font) font = ImGui::GetFont();
    
    // Shadow
    if (config.shadow) {
        drawList->AddText(font, fontSize, ImVec2(pos.x + config.shadowOffset.x, pos.y + config.shadowOffset.y), config.shadowColor, text, textEnd, wrapWidth);
    }
    
    // Outline
    if (config.outline) {
        const float offsets[8][2] = {
            {-1, -1}, {0, -1}, {1, -1},
            {-1, 0},           {1, 0},
            {-1, 1},  {0, 1},  {1, 1}
        };
        
        for (int i = 0; i < 8; ++i) {
            drawList->AddText(font, fontSize, 
                ImVec2(pos.x + offsets[i][0] * config.outlineThickness, pos.y + offsets[i][1] * config.outlineThickness),
                config.outlineColor, text, textEnd, wrapWidth);
        }
    }
    
    // Glow
    if (config.glow) {
        for (int i = 0; i < 4; ++i) {
            float t = (i + 1) * 0.25f;
            float size = config.glowSize * t;
            uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(config.glowColor) * (1.0f - t) * 0.5f);
            ImU32 glowColor = (config.glowColor & 0x00FFFFFF) | (alpha << 24);
            
            drawList->AddText(font, fontSize, 
                ImVec2(pos.x - size, pos.y - size), glowColor, text, textEnd, wrapWidth);
            drawList->AddText(font, fontSize, 
                ImVec2(pos.x + size, pos.y - size), glowColor, text, textEnd, wrapWidth);
            drawList->AddText(font, fontSize, 
                ImVec2(pos.x - size, pos.y + size), glowColor, text, textEnd, wrapWidth);
            drawList->AddText(font, fontSize, 
                ImVec2(pos.x + size, pos.y + size), glowColor, text, textEnd, wrapWidth);
        }
    }
    
    // Gradient text
    if (config.gradient && config.gradientColor1 != 0 && config.gradientColor2 != 0) {
        // Approximate gradient by drawing multiple times with different colors
        int steps = 8;
        for (int i = 0; i < steps; ++i) {
            float t = static_cast<float>(i) / (steps - 1);
            ImU32 c = LerpColor(config.gradientColor1, config.gradientColor2, t);
            
            ImVec2 offset;
            if (config.horizontal) {
                offset = ImVec2(pos.x + (i * wrapWidth / steps), pos.y);
            } else {
                offset = ImVec2(pos.x, pos.y + (i * 20 / steps)); // Approximate line height
            }
            
            drawList->AddText(font, fontSize, offset, c, text, textEnd, wrapWidth / steps);
        }
    } else {
        // Normal text
        drawList->AddText(font, fontSize, pos, color, text, textEnd, wrapWidth);
    }
}

void ViceRender::DrawTextWithShadow(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, const ImVec2& shadowOffset, ImU32 shadowColor) {
    drawList->AddText(font, fontSize, ImVec2(pos.x + shadowOffset.x, pos.y + shadowOffset.y), shadowColor, text);
    drawList->AddText(font, fontSize, pos, color, text);
}

void ViceRender::DrawTextWithOutline(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, float thickness, ImU32 outlineColor) {
    const float offsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1, 0},           {1, 0},
        {-1, 1},  {0, 1},  {1, 1}
    };
    
    for (int i = 0; i < 8; ++i) {
        drawList->AddText(font, fontSize, 
            ImVec2(pos.x + offsets[i][0] * thickness, pos.y + offsets[i][1] * thickness),
            outlineColor, text);
    }
    
    drawList->AddText(font, fontSize, pos, color, text);
}

void ViceRender::DrawTextWithGlow(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, ImU32 glowColor, float glowSize) {
    for (int i = 0; i < 4; ++i) {
        float t = (i + 1) * 0.25f;
        float size = glowSize * t;
        uint8_t alpha = static_cast<uint8_t>(IM_COL32_A(glowColor) * (1.0f - t) * 0.5f);
        ImU32 c = (glowColor & 0x00FFFFFF) | (alpha << 24);
        
        drawList->AddText(font, fontSize, ImVec2(pos.x - size, pos.y - size), c, text);
        drawList->AddText(font, fontSize, ImVec2(pos.x + size, pos.y - size), c, text);
        drawList->AddText(font, fontSize, ImVec2(pos.x - size, pos.y + size), c, text);
        drawList->AddText(font, fontSize, ImVec2(pos.x + size, pos.y + size), c, text);
    }
    
    drawList->AddText(font, fontSize, pos, color, text);
}

void ViceRender::DrawTextGradient(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color1, ImU32 color2, const char* text, bool horizontal) {
    // Approximate by measuring text width and drawing character by character
    if (!horizontal) {
        drawList->AddText(font, fontSize, pos, color1, text);
        return;
    }
    
    const char* p = text;
    float x = pos.x;
    
    while (*p) {
        // Get character width
        char c[2] = {*p, 0};
        ImVec2 charSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, c);
        
        float t = (x - pos.x) / std::max(1.0f, charSize.x * strlen(text));
        t = std::clamp(t, 0.0f, 1.0f);
        ImU32 c = LerpColor(color1, color2, t);
        
        drawList->AddText(font, fontSize, ImVec2(x, pos.y), c, c);
        x += charSize.x;
        ++p;
    }
}

// ========== Image / Texture ==========

void ViceRender::DrawImage(ImDrawList* drawList, ImTextureID texture, const ImVec2& pMin, const ImVec2& pMax, const ImageConfig& config) {
    drawList->AddImage(texture, pMin, pMax, config.uv0, config.uv1, config.tintColor);
}

void ViceRender::DrawImageRounded(ImDrawList* drawList, ImTextureID texture, const ImVec2& pMin, const ImVec2& pMax, float rounding, const ImageConfig& config) {
    drawList->AddImageRounded(texture, pMin, pMax, config.uv0, config.uv1, config.tintColor, rounding);
}

void ViceRender::DrawImageNinePatch(ImDrawList* drawList, ImTextureID texture, const ImVec2& pMin, const ImVec2& pMax, const ImVec2& borders, const ImageConfig& config) {
    // Nine-patch not directly supported in ImGui, simplified
    DrawImage(drawList, texture, pMin, pMax, config);
}

// ========== Clipping ==========

void ViceRender::PushClipRect(const ImVec2& min, const ImVec2& max, bool intersect) {
    ImGui::GetWindowDrawList()->PushClipRect(min, max, intersect);
}

void ViceRender::PopClipRect() {
    ImGui::GetWindowDrawList()->PopClipRect();
}

ViceRender::ClipRect ViceRender::GetCurrentClipRect() const {
    const ImVec4& clip = ImGui::GetWindowDrawList()->GetClipRect();
    return {ImVec2(clip.x, clip.y), ImVec2(clip.z, clip.w)};
}

// ========== Primitive Batching ==========

void ViceRender::BeginBatch() {
    // ImGui handles batching automatically
}

void ViceRender::EndBatch() {
    // ImGui handles batching automatically
}

void ViceRender::FlushBatch() {
    // ImGui handles batching automatically
}

// ========== Screen Space Effects ==========

void ViceRender::DrawVignette(ImDrawList* drawList, const ImVec2& center, float radius, ImU32 color, float intensity) {
    int segments = 32;
    for (int i = 0; i < segments; ++i) {
        float t1 = static_cast<float>(i) / segments;
        float t2 = static_cast<float>(i + 1) / segments;
        
        float r1 = radius * t1;
        float r2 = radius * t2;
        
        uint8_t alpha1 = static_cast<uint8_t>(IM_COL32_A(color) * intensity * (1.0f - t1));
        uint8_t alpha2 = static_cast<uint8_t>(IM_COL32_A(color) * intensity * (1.0f - t2));
        
        ImU32 c1 = (color & 0x00FFFFFF) | (alpha1 << 24);
        ImU32 c2 = (color & 0x00FFFFFF) | (alpha2 << 24);
        
        drawList->AddCircle(center, r1, c1, 64, 1.0f);
    }
}

void ViceRender::DrawRadialGradient(ImDrawList* drawList, const ImVec2& center, float radius, ImU32 colorInner, ImU32 colorOuter) {
    int segments = 32;
    for (int i = 0; i < segments; ++i) {
        float t1 = static_cast<float>(i) / segments;
        float t2 = static_cast<float>(i + 1) / segments;
        
        float r1 = radius * t1;
        float r2 = radius * t2;
        
        ImU32 c1 = LerpColor(colorInner, colorOuter, t1);
        ImU32 c2 = LerpColor(colorInner, colorOuter, t2);
        
        drawList->AddCircleFilled(center, r2, c2, 64);
    }
}

void ViceRender::DrawScanlines(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float spacing, ImU32 color, float opacity) {
    ImU32 c = (color & 0x00FFFFFF) | (static_cast<uint8_t>(255 * opacity) << 24);
    
    for (float y = pMin.y; y < pMax.y; y += spacing) {
        drawList->AddLine(ImVec2(pMin.x, y), ImVec2(pMax.x, y), c);
    }
}

void ViceRender::DrawNoise(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float scale, ImU32 color, float opacity) {
    // Simplified noise - random pixels
    std::mt19937 rng(static_cast<uint32_t>(ImGui::GetTime() * 1000));
    std::uniform_int_distribution<int> distX(static_cast<int>(pMin.x), static_cast<int>(pMax.x));
    std::uniform_int_distribution<int> distY(static_cast<int>(pMin.y), static_cast<int>(pMax.y));
    std::uniform_int_distribution<int> distAlpha(0, static_cast<int>(255 * opacity));
    
    ImU32 baseColor = color & 0x00FFFFFF;
    
    int numPixels = static_cast<int>((pMax.x - pMin.x) * (pMax.y - pMin.y) * 0.01f);
    for (int i = 0; i < numPixels; ++i) {
        ImVec2 p = ImVec2(static_cast<float>(distX(rng)), static_cast<float>(distY(rng)));
        uint8_t alpha = static_cast<uint8_t>(distAlpha(rng));
        ImU32 c = baseColor | (alpha << 24);
        drawList->AddRectFilled(p, ImVec2(p.x + 1, p.y + 1), c);
    }
}

void ViceRender::DrawGrid(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float cellSize, ImU32 color, float thickness) {
    for (float x = pMin.x; x <= pMax.x; x += cellSize) {
        drawList->AddLine(ImVec2(x, pMin.y), ImVec2(x, pMax.y), color, thickness);
    }
    for (float y = pMin.y; y <= pMax.y; y += cellSize) {
        drawList->AddLine(ImVec2(pMin.x, y), ImVec2(pMax.x, y), color, thickness);
    }
}

// ========== World to Screen ==========

bool ViceRender::WorldToScreen(const sdk::Vector3D& world, ImVec2& screen, const sdk::Matrix4x4& viewMatrix, int width, int height) {
    // Transform world to clip space
    float w = viewMatrix.m[3][0] * world.x + viewMatrix.m[3][1] * world.y + viewMatrix.m[3][2] * world.z + viewMatrix.m[3][3];
    
    if (w < 0.01f) return false;
    
    float x = viewMatrix.m[0][0] * world.x + viewMatrix.m[0][1] * world.y + viewMatrix.m[0][2] * world.z + viewMatrix.m[0][3];
    float y = viewMatrix.m[1][0] * world.x + viewMatrix.m[1][1] * world.y + viewMatrix.m[1][2] * world.z + viewMatrix.m[1][3];
    
    float invW = 1.0f / w;
    x *= invW;
    y *= invW;
    
    // NDC to screen
    screen.x = (width * 0.5f) + (x * width * 0.5f);
    screen.y = (height * 0.5f) - (y * height * 0.5f);
    
    return true;
}

bool ViceRender::WorldToScreen(const sdk::Vector3D& world, ImVec2& screen) {
    // Use default view matrix from game
    // This would need to be hooked from the game's view matrix
    return false;
}

// ========== Debug Drawing ==========

void ViceRender::DrawDebugCrosshair(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color, float thickness) {
    drawList->AddLine(
        ImVec2(center.x - size, center.y),
        ImVec2(center.x + size, center.y),
        color, thickness
    );
    drawList->AddLine(
        ImVec2(center.x, center.y - size),
        ImVec2(center.x, center.y + size),
        color, thickness
    );
}

void ViceRender::DrawDebugBounds(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, ImU32 color, float thickness) {
    drawList->AddRect(pMin, pMax, color, 0.0f, ImDrawFlags_RoundCornersNone, thickness);
}

void ViceRender::DrawDebugText(ImDrawList* drawList, const ImVec2& pos, ImU32 color, const char* fmt, ...) const {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    drawList->AddText(pos, color, buffer);
}

} // namespace gui