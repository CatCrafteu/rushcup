#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include "vice_theme.hpp"

namespace gui {

// Advanced rendering utilities (blur, shadows, animations, rounded rects)
class ViceRender {
public:
    static ViceRender& Instance() {
        static ViceRender instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void NewFrame();
    void EndFrame();
    
    // ========== Blur Effects ==========
    enum class BlurType { None, Background, Frosted, Strong };
    
    struct BlurConfig {
        BlurType type = BlurType::Frosted;
        float intensity = 8.0f;
        int passes = 2;
        ImU32 tintColor = IM_COL32(0, 0, 0, 0);
        float tintStrength = 0.1f;
    };
    
    void PushBlur(const BlurConfig& config = {});
    void PopBlur();
    void DrawBlurRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, 
                      float rounding, const BlurConfig& config);
    void DrawBlurBackground(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding);
    
    // ========== Shadows ==========
    struct ShadowConfig {
        ImVec2 offset = {0, 4};
        float blurRadius = 16.0f;
        ImU32 color = IM_COL32(0, 0, 0, 120);
        float spread = 0.0f;
        bool onlyRect = false;
    };
    
    void DrawShadowRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, 
                        float rounding, const ShadowConfig& config);
    void DrawShadowCircle(ImDrawList* drawList, const ImVec2& center, float radius, const ShadowConfig& config);
    void DrawShadowPolygon(ImDrawList* drawList, const ImVec2* points, int count, const ShadowConfig& config);
    void DrawLayeredShadow(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, int layers = 3);
    
    // ========== Animations ==========
    struct AnimationConfig {
        float duration = 0.2f;
        enum class Easing { Linear, EaseIn, EaseOut, EaseInOut, Bounce, Elastic, Back } easing = Easing::EaseOut;
        float delay = 0.0f;
        bool reverse = false;
    };
    
    // Animated float
    float AnimFloat(const char* id, float target, const AnimationConfig& config = {});
    float AnimFloatClamped(const char* id, float target, float min, float max, const AnimationConfig& config = {});
    
    // Animated color
    ImU32 AnimColor(const char* id, ImU32 target, const AnimationConfig& config = {});
    
    // Animated vec2
    ImVec2 AnimVec2(const char* id, const ImVec2& target, const AnimationConfig& config = {});
    
    // Animated bool (for visibility)
    float AnimVisibility(const char* id, bool visible, const AnimationConfig& config = {});
    
    // Animated progress
    float AnimProgress(const char* id, float target, const AnimationConfig& config = {});
    
    // ========== Easing Functions ==========
    static float EaseLinear(float t) { return t; }
    static float EaseInQuad(float t) { return t * t; }
    static float EaseOutQuad(float t) { return 1 - (1 - t) * (1 - t); }
    static float EaseInOutQuad(float t) { return t < 0.5 ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t); }
    static float EaseInCubic(float t) { return t * t * t; }
    static float EaseOutCubic(float t) { return 1 - pow(1 - t, 3); }
    static float EaseInOutCubic(float t) { return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2; }
    static float EaseInQuart(float t) { return t * t * t * t; }
    static float EaseOutQuart(float t) { return 1 - pow(1 - t, 4); }
    static float EaseInOutQuart(float t) { return t < 0.5 ? 8 * t * t * t * t : 1 - pow(-2 * t + 2, 4) / 2; }
    static float EaseInExpo(float t) { return t == 0 ? 0 : pow(2, 10 * (t - 1)); }
    static float EaseOutExpo(float t) { return t == 1 ? 1 : 1 - pow(2, -10 * t); }
    static float EaseInOutExpo(float t) { 
        return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? pow(2, 20 * t - 10) / 2 : (2 - pow(2, -20 * t + 10)) / 2; 
    }
    static float EaseInCirc(float t) { return 1 - sqrt(1 - t * t); }
    static float EaseOutCirc(float t) { return sqrt(1 - pow(t - 1, 2)); }
    static float EaseInOutCirc(float t) { return t < 0.5 ? (1 - sqrt(1 - 4 * t * t)) / 2 : (sqrt(1 - pow(-2 * t + 2, 2)) + 1) / 2; }
    static float EaseInBack(float t) { const float c1 = 1.70158f; const float c3 = c1 + 1; return c3 * t * t * t - c1 * t * t; }
    static float EaseOutBack(float t) { const float c1 = 1.70158f; const float c3 = c1 + 1; return 1 + c3 * pow(t - 1, 3) + c1 * pow(t - 1, 2); }
    static float EaseInOutBack(float t) { const float c1 = 1.70158f; const float c2 = c1 * 1.525f; return t < 0.5 ? (pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2 : (pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2; }
    static float EaseInElastic(float t) { const float c4 = (2 * 3.14159f) / 3; return t == 0 ? 0 : t == 1 ? 1 : -pow(2, 10 * t - 10) * sin((t * 10 - 10.75f) * c4); }
    static float EaseOutElastic(float t) { const float c4 = (2 * 3.14159f) / 3; return t == 0 ? 0 : t == 1 ? 1 : pow(2, -10 * t) * sin((t * 10 - 0.75f) * c4) + 1; }
    static float EaseInOutElastic(float t) { const float c5 = (2 * 3.14159f) / 4.5f; return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? -(pow(2, 20 * t - 10) * sin((20 * t - 11.125f) * c5)) / 2 : (pow(2, -20 * t + 10) * sin((20 * t - 11.125f) * c5)) / 2 + 1; }
    static float EaseInBounce(float t) { return 1 - EaseOutBounce(1 - t); }
    static float EaseOutBounce(float t) {
        const float n1 = 7.5625f; const float d1 = 2.75f;
        if (t < 1 / d1) return n1 * t * t;
        else if (t < 2 / d1) return n1 * (t - 1.5f / d1) * (t - 1.5f / d1) + 0.75f;
        else if (t < 2.5f / d1) return n1 * (t - 2.25f / d1) * (t - 2.25f / d1) + 0.9375f;
        else return n1 * (t - 2.625f / d1) * (t - 2.625f / d1) + 0.984375f;
    }
    static float EaseInOutBounce(float t) { return t < 0.5 ? EaseInBounce(t * 2) / 2 : EaseOutBounce(t * 2 - 1) / 2 + 0.5f; }
    
    static float ApplyEasing(float t, AnimationConfig::Easing easing) {
        switch (easing) {
            case AnimationConfig::Easing::Linear: return EaseLinear(t);
            case AnimationConfig::Easing::EaseIn: return EaseInCubic(t);
            case AnimationConfig::Easing::EaseOut: return EaseOutCubic(t);
            case AnimationConfig::Easing::EaseInOut: return EaseInOutCubic(t);
            case AnimationConfig::Easing::Bounce: return EaseOutBounce(t);
            case AnimationConfig::Easing::Elastic: return EaseOutElastic(t);
            case AnimationConfig::Easing::Back: return EaseOutBack(t);
        }
        return t;
    }
    
    // ========== Rounded Rectangles ==========
    struct RoundedRectConfig {
        float rounding = 8.0f;
        ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
        float thickness = 1.0f;
        bool filled = true;
        ImU32 fillColor = 0;
        ImU32 borderColor = 0;
        ImVec2 shadowOffset = {0, 0};
        float shadowBlur = 0.0f;
        ImU32 shadowColor = 0;
    };
    
    void DrawRoundedRect(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, const RoundedRectConfig& config);
    void DrawRoundedRectFilled(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 color, ImDrawFlags flags = ImDrawFlags_RoundCornersAll);
    void DrawRoundedRectOutlined(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 color, float thickness = 1.0f, ImDrawFlags flags = ImDrawFlags_RoundCornersAll);
    void DrawRoundedRectGradient(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 col1, ImU32 col2, bool horizontal = true, ImDrawFlags flags = ImDrawFlags_RoundCornersAll);
    void DrawRoundedRectMultiColor(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float rounding, ImU32 colTL, ImU32 colTR, ImU32 colBR, ImU32 colBL, ImDrawFlags flags = ImDrawFlags_RoundCornersAll);
    
    // ========== Custom Shapes ==========
    void DrawRoundedTriangle(ImDrawList* drawList, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float rounding, ImU32 color, bool filled = true, float thickness = 1.0f);
    void DrawRoundedPolygon(ImDrawList* drawList, const ImVec2* points, int count, float rounding, ImU32 color, bool filled = true, float thickness = 1.0f);
    void DrawPie(ImDrawList* drawList, const ImVec2& center, float radius, float startAngle, float endAngle, ImU32 color, bool filled = true, int segments = 32);
    void DrawArc(ImDrawList* drawList, const ImVec2& center, float radius, float startAngle, float endAngle, ImU32 color, float thickness, int segments = 32);
    void DrawRing(ImDrawList* drawList, const ImVec2& center, float innerRadius, float outerRadius, float startAngle, float endAngle, ImU32 color, int segments = 32);
    
    // ========== Text Effects ==========
    struct TextEffectConfig {
        bool shadow = false;
        ImVec2 shadowOffset = {1, 1};
        ImU32 shadowColor = IM_COL32(0, 0, 0, 180);
        bool outline = false;
        float outlineThickness = 1.0f;
        ImU32 outlineColor = IM_COL32(0, 0, 0, 255);
        bool glow = false;
        ImU32 glowColor = IM_COL32(255, 95, 155, 120);
        float glowSize = 4.0f;
        bool gradient = false;
        ImU32 gradientColor1 = 0;
        ImU32 gradientColor2 = 0;
        bool horizontal = true;
    };
    
    void DrawTextEx(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, const char* textEnd = nullptr, float wrapWidth = 0.0f, const TextEffectConfig& config = {});
    void DrawTextWithShadow(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, const ImVec2& shadowOffset = {1, 1}, ImU32 shadowColor = IM_COL32(0, 0, 0, 180));
    void DrawTextWithOutline(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, float thickness = 1.0f, ImU32 outlineColor = IM_COL32(0, 0, 0, 255));
    void DrawTextWithGlow(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, ImU32 glowColor, float glowSize = 4.0f);
    void DrawTextGradient(ImDrawList* drawList, const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color1, ImU32 color2, const char* text, bool horizontal = true);
    
    // ========== Image / Texture ==========
    struct ImageConfig {
        ImVec2 uv0 = {0, 0};
        ImVec2 uv1 = {1, 1};
        ImU32 tintColor = IM_COL32(255, 255, 255, 255);
        ImU32 borderColor = 0;
        float rounding = 0.0f;
        bool rotate = false;
        float rotation = 0.0f;
        ImVec2 rotationCenter = {0.5f, 0.5f};
    };
    
    void DrawImage(ImDrawList* drawList, ImTextureID texture, const ImVec2& pMin, const ImVec2& pMax, const ImageConfig& config = {});
    void DrawImageRounded(ImDrawList* drawList, ImTextureID texture, const ImVec2& pMin, const ImVec2& pMax, float rounding, const ImageConfig& config = {});
    void DrawImageNinePatch(ImDrawList* drawList, ImTextureID texture, const ImVec2& pMin, const ImVec2& pMax, const ImVec2& borders, const ImageConfig& config = {});
    
    // ========== Clipping ==========
    struct ClipRect {
        ImVec2 min;
        ImVec2 max;
    };
    
    void PushClipRect(const ImVec2& min, const ImVec2& max, bool intersect = true);
    void PopClipRect();
    ClipRect GetCurrentClipRect() const;
    
    // ========== Primitive Batching ==========
    void BeginBatch();
    void EndBatch();
    void FlushBatch();
    
    // ========== Screen Space Effects ==========
    void DrawVignette(ImDrawList* drawList, const ImVec2& center, float radius, ImU32 color, float intensity = 1.0f);
    void DrawRadialGradient(ImDrawList* drawList, const ImVec2& center, float radius, ImU32 colorInner, ImU32 colorOuter);
    void DrawScanlines(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float spacing, ImU32 color, float opacity = 0.1f);
    void DrawNoise(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float scale, ImU32 color, float opacity = 0.05f);
    void DrawGrid(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, float cellSize, ImU32 color, float thickness = 1.0f);
    
    // ========== World to Screen Helpers ==========
    bool WorldToScreen(const sdk::Vector3D& world, ImVec2& screen, const sdk::Matrix4x4& viewMatrix, int width, int height);
    bool WorldToScreen(const sdk::Vector3D& world, ImVec2& screen);
    
    // ========== Debug Drawing ==========
    void DrawDebugCrosshair(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color, float thickness = 1.0f);
    void DrawDebugBounds(ImDrawList* drawList, const ImVec2& pMin, const ImVec2& pMax, ImU32 color, float thickness = 1.0f);
    void DrawDebugText(ImDrawList* drawList, const ImVec2& pos, ImU32 color, const char* fmt, ...) const;

private:
    ViceRender() = default;
    
    struct AnimationState {
        float value = 0.0f;
        float target = 0.0f;
        float velocity = 0.0f;
        float lastTime = 0.0f;
        AnimationConfig config;
    };
    
    std::unordered_map<std::string, AnimationState> m_animations;
    std::mutex m_mutex;
    float m_deltaTime = 0.0f;
    float m_time = 0.0f;
    
    void UpdateAnimations();
    float GetAnimationValue(AnimationState& state);
    ImU32 LerpColor(ImU32 a, ImU32 b, float t) const;
};

} // namespace gui