#include "ui.h"
#include <algorithm>
#include <cmath>

float Distance(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Color WithAlpha(Color c, float alphaScale) {
    c.a = static_cast<int>(c.a * (std::clamp)(alphaScale, 0.0f, 1.0f));
    return c;
}

bool WorldToScreen(float x, float y, float z, float& sx, float& sy) {
    return Invoke<int>(N::GET_SCREEN_COORD_FROM_WORLD_COORD, x, y, z, &sx, &sy) != 0;
}

float AspectRatio() {
    return Invoke<float>(N::GET_ASPECT_RATIO, 0);
}

void Rect(float cx, float cy, float w, float h, Color c) {
    Invoke(N::DRAW_RECT, cx, cy, w, h, c.r, c.g, c.b, c.a, 0);
}

void Line3D(const Vec3& a, const Vec3& b, Color c) {
    Invoke(N::DRAW_LINE, a.x, a.y, a.z, b.x, b.y, b.z, c.r, c.g, c.b, c.a);
}

// DRAW_LINE is depth-tested, so pull both ends along their camera rays to just in front of the
// camera. The projected 2D line is identical, but nothing in the world can sit in front of it.
void OverlayLine(const Vec3& a, const Vec3& b, Color c) {
    constexpr float kDepth = 0.5f;
    Vec3 cam = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_COORD);
    auto pull = [&](const Vec3& p) {
        float dx = p.x - cam.x, dy = p.y - cam.y, dz = p.z - cam.z;
        float len = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (len < kDepth) return p;
        Vec3 r = cam;
        float s = kDepth / len;
        r.x += dx * s; r.y += dy * s; r.z += dz * s;
        return r;
    };
    Line3D(pull(a), pull(b), c);
}

void Text(float x, float y, const char* s, Color c, float scale, bool centre, int font, bool outline) {
    Invoke(N::SET_TEXT_FONT, font);
    Invoke(N::SET_TEXT_SCALE, 0.0f, scale);
    Invoke(N::SET_TEXT_COLOUR, c.r, c.g, c.b, c.a);
    Invoke(N::SET_TEXT_CENTRE, centre ? 1 : 0);
    if (outline) Invoke(N::SET_TEXT_OUTLINE);
    Invoke(N::BEGIN_TEXT_COMMAND_DISPLAY_TEXT, "STRING");
    Invoke(N::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, s);
    Invoke(N::END_TEXT_COMMAND_DISPLAY_TEXT, x, y, 0);
}

float TextWidth(const char* s, float scale, int font) {
    Invoke(N::SET_TEXT_FONT, font);
    Invoke(N::SET_TEXT_SCALE, 0.0f, scale);
    Invoke(N::BEGIN_TEXT_COMMAND_GET_WIDTH, "STRING");
    Invoke(N::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, s);
    return Invoke<float>(N::END_TEXT_COMMAND_GET_WIDTH, 1);
}

bool Sprite(const char* dict, const char* name, float x, float y, float w, float h, float rotation, Color c) {
    if (!Invoke<int>(N::HAS_STREAMED_TEXTURE_DICT_LOADED, dict)) {
        Invoke(N::REQUEST_STREAMED_TEXTURE_DICT, dict, 0);
        return false;
    }
    Invoke(N::DRAW_SPRITE, dict, name, x, y, w, h, rotation, c.r, c.g, c.b, c.a);
    return true;
}

void Notify(const char* s) {
    Invoke(N::SET_NOTIFICATION_TEXT_ENTRY, "STRING");
    Invoke(N::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, s);
    Invoke<int>(N::DRAW_NOTIFICATION, 0, 1);
}

bool ScreenBox(Entity e, Box2D& out) {
    Vec3 mn{}, mx{};
    Invoke(N::GET_MODEL_DIMENSIONS, Invoke<Hash>(N::GET_ENTITY_MODEL, e), &mn, &mx);

    out = { 1.0f, 1.0f, 0.0f, 0.0f };
    int hits = 0;
    for (int i = 0; i < 8; ++i) {
        float ox = (i & 1) ? mx.x : mn.x;
        float oy = (i & 2) ? mx.y : mn.y;
        float oz = (i & 4) ? mx.z : mn.z;
        Vec3 w = Invoke<Vec3>(N::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS, e, ox, oy, oz);
        float sx, sy;
        if (!WorldToScreen(w.x, w.y, w.z, sx, sy))
            continue;
        out.x0 = (std::min)(out.x0, sx); out.y0 = (std::min)(out.y0, sy);
        out.x1 = (std::max)(out.x1, sx); out.y1 = (std::max)(out.y1, sy);
        ++hits;
    }
    return hits > 0;
}

// Corner brackets rather than a full outline: each corner gets two short strokes, a quarter of
// the box's shorter side (in pixels) long, so wide and tall boxes get equal-looking corners.
void DrawBox(const Box2D& b, Color c) {
    const float aspect = AspectRatio();
    const float ty = 0.0012f;
    const float tx = ty / aspect;
    float w = b.x1 - b.x0, h = b.y1 - b.y0;
    float len = (std::min)(w * aspect, h) * 0.25f; // in screen-height units
    float lx = len / aspect, ly = len;
    // Horizontal strokes run past the corner by half a stroke width so the corners close.
    float hx0 = b.x0 - tx * 0.5f, hx1 = b.x1 + tx * 0.5f;
    for (float y : { b.y0, b.y1 }) {
        Rect(hx0 + lx * 0.5f, y, lx, ty, c);
        Rect(hx1 - lx * 0.5f, y, lx, ty, c);
    }
    for (float x : { b.x0, b.x1 }) {
        Rect(x, b.y0 + ly * 0.5f, tx, ly, c);
        Rect(x, b.y1 - ly * 0.5f, tx, ly, c);
    }
}

void DrawHealthBar(const Box2D& b, float frac) {
    frac = (std::clamp)(frac, 0.0f, 1.0f);
    const float h = 0.0035f;
    const float inset = 0.0006f;
    float w = (std::clamp)((b.x1 - b.x0) * 0.8f, 0.012f, 0.028f);
    float left = (b.x0 + b.x1) * 0.5f - w * 0.5f;
    float y = b.y0 - kHealthBarGap;

    Rect(left + w * 0.5f, y, w, h, { 0, 0, 0, 170 });
    float fillW = (w - 2 * inset) * frac;
    Color c{ static_cast<int>(255 * (1.0f - frac)), static_cast<int>(255 * frac), 40, 230 };
    Rect(left + inset + fillW * 0.5f, y, fillW, h - 2 * inset, c);
}

namespace {

bool GameFocused() {
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

bool g_down[256];
ULONGLONG g_nextRepeat[256];

bool Poll(int vk, bool& now) {
    if (vk <= 0 || vk > 255) return false;
    now = GameFocused() && (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = now && !g_down[vk];
    g_down[vk] = now;
    return pressed;
}

} // namespace

bool KeyPressed(int vk) {
    bool now;
    return Poll(vk, now);
}

bool KeyRepeat(int vk) {
    constexpr ULONGLONG kDelay = 350, kRate = 70;
    bool now;
    ULONGLONG t = GetTickCount64();
    if (Poll(vk, now)) {
        g_nextRepeat[vk] = t + kDelay;
        return true;
    }
    if (now && t >= g_nextRepeat[vk]) {
        g_nextRepeat[vk] = t + kRate;
        return true;
    }
    return false;
}
