// Single-player ESP for GTA V (ScriptHookV plugin).
// F9 toggle ESP, F10 cycle targets (peds / vehicles / both), F11 toggle tracers.
// (F5-F8 are the game's character-switch keys.)

#include "invoke.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

enum class Targets { Peds, Vehicles, Both };

constexpr int kKeyToggle  = VK_F9;
constexpr int kKeyTargets = VK_F10;
constexpr int kKeyTracers = VK_F11;

struct Settings {
    bool enabled = true;
    bool tracers = false;
    Targets targets = Targets::Both;
    float pedRange = 250.0f;
    float vehRange = 350.0f;
} g;

struct Color { int r, g, b, a; };

constexpr Color kWhite   { 255, 255, 255, 220 };
constexpr Color kRed     { 255,  60,  60, 230 };
constexpr Color kGreen   {  80, 255, 120, 230 };
constexpr Color kBlue    {  80, 160, 255, 230 };
constexpr Color kGrey    { 140, 140, 140, 160 };
constexpr Color kYellow  { 255, 210,  60, 230 };

float Distance(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool WorldToScreen(float x, float y, float z, float& sx, float& sy) {
    return Invoke<int>(N::GET_SCREEN_COORD_FROM_WORLD_COORD, x, y, z, &sx, &sy) != 0;
}

void Rect(float cx, float cy, float w, float h, Color c) {
    Invoke(N::DRAW_RECT, cx, cy, w, h, c.r, c.g, c.b, c.a, 0);
}

void Text(float x, float y, const char* s, Color c, float scale = 0.26f) {
    Invoke(N::SET_TEXT_FONT, 0);
    Invoke(N::SET_TEXT_SCALE, 0.0f, scale);
    Invoke(N::SET_TEXT_COLOUR, c.r, c.g, c.b, 255);
    Invoke(N::SET_TEXT_CENTRE, 1);
    Invoke(N::SET_TEXT_OUTLINE);
    Invoke(N::BEGIN_TEXT_COMMAND_DISPLAY_TEXT, "STRING");
    Invoke(N::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, s);
    Invoke(N::END_TEXT_COMMAND_DISPLAY_TEXT, x, y, 0);
}

struct Box2D { float x0, y0, x1, y1; };

// Projects the entity's model bounding box (8 corners) and returns the enclosing screen rect.
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

void DrawBox(const Box2D& b, Color c) {
    const float ty = 0.0012f;
    const float tx = ty / Invoke<float>(N::GET_ASPECT_RATIO, 0);
    float w = b.x1 - b.x0, h = b.y1 - b.y0;
    float cx = (b.x0 + b.x1) * 0.5f, cy = (b.y0 + b.y1) * 0.5f;
    Rect(cx, b.y0, w, ty, c);
    Rect(cx, b.y1, w, ty, c);
    Rect(b.x0, cy, tx, h, c);
    Rect(b.x1, cy, tx, h, c);
}

void DrawHealthBar(const Box2D& b, float frac) {
    frac = (std::clamp)(frac, 0.0f, 1.0f);
    const float barW = 0.003f;
    float h = b.y1 - b.y0;
    float x = b.x0 - barW * 1.5f;
    Rect(x, (b.y0 + b.y1) * 0.5f, barW, h, { 0, 0, 0, 160 });
    float fillH = h * frac;
    Color c{ static_cast<int>(255 * (1.0f - frac)), static_cast<int>(255 * frac), 40, 220 };
    Rect(x, b.y1 - fillH * 0.5f, barW * 0.6f, fillH, c);
}

void Tracer(const Vec3& from, const Vec3& to, Color c) {
    Invoke(N::DRAW_LINE, from.x, from.y, from.z, to.x, to.y, to.z, c.r, c.g, c.b, c.a);
}

Color PedColor(Ped ped, Ped player) {
    if (Invoke<int>(N::IS_ENTITY_DEAD, ped, 0))
        return kGrey;
    if (Invoke<int>(N::IS_PED_IN_COMBAT, ped, player))
        return kRed;
    switch (Invoke<int>(N::GET_PED_TYPE, ped)) {
        case 6: case 27: case 29: return kBlue; // cop, swat, army
    }
    int rel = Invoke<int>(N::GET_RELATIONSHIP_BETWEEN_PEDS, ped, player);
    if (rel == 5 || rel == 4) return kRed;    // hate / dislike
    if (rel <= 2) return kGreen;              // companion / respect / like
    return kWhite;
}

void DrawPeds(Ped player, const Vec3& me) {
    static int peds[512];
    int count = worldGetAllPeds(peds, 512);
    char buf[64];

    for (int i = 0; i < count; ++i) {
        Ped p = peds[i];
        if (p == player) continue;

        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, p, 1);
        float dist = Distance(me, pos);
        if (dist > g.pedRange) continue;

        Box2D box;
        if (!ScreenBox(p, box)) continue;

        Color c = PedColor(p, player);
        DrawBox(box, c);

        int hp = Invoke<int>(N::GET_ENTITY_HEALTH, p);
        int maxHp = Invoke<int>(N::GET_ENTITY_MAX_HEALTH, p);
        // Peds die at 100 HP, so the usable range is [100, max].
        if (maxHp > 100)
            DrawHealthBar(box, (hp - 100) / float(maxHp - 100));

        std::snprintf(buf, sizeof buf, "%.0fm", dist);
        Text((box.x0 + box.x1) * 0.5f, box.y1 + 0.003f, buf, c);

        if (g.tracers && !Invoke<int>(N::IS_ENTITY_DEAD, p, 0))
            Tracer(me, pos, c);
    }
}

void DrawVehicles(Ped player, const Vec3& me) {
    static int vehs[512];
    int count = worldGetAllVehicles(vehs, 512);
    char buf[96];

    for (int i = 0; i < count; ++i) {
        Vehicle v = vehs[i];
        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, v, 1);
        float dist = Distance(me, pos);
        if (dist < 3.0f || dist > g.vehRange) continue; // skip the car we're sitting in

        Box2D box;
        if (!ScreenBox(v, box)) continue;

        Color c = Invoke<int>(N::IS_ENTITY_DEAD, v, 0) ? kGrey : kYellow;
        DrawBox(box, c);

        const char* label = Invoke<const char*>(N::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL,
                                                Invoke<Hash>(N::GET_ENTITY_MODEL, v));
        const char* name = Invoke<const char*>(N::GET_FILENAME_FOR_AUDIO_CONVERSATION, label);
        std::snprintf(buf, sizeof buf, "%s  %.0fm", name ? name : "?", dist);
        Text((box.x0 + box.x1) * 0.5f, box.y0 - 0.022f, buf, c);

        if (g.tracers)
            Tracer(me, pos, c);
    }
}

bool KeyPressed(int vk) {
    static bool down[256];
    bool now = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = now && !down[vk];
    down[vk] = now;
    return pressed;
}

const char* TargetsName(Targets t) {
    switch (t) {
        case Targets::Peds:     return "peds";
        case Targets::Vehicles: return "vehicles";
        default:                return "peds + vehicles";
    }
}

void Tick() {
    if (KeyPressed(kKeyToggle))  g.enabled = !g.enabled;
    if (KeyPressed(kKeyTargets)) g.targets = static_cast<Targets>((static_cast<int>(g.targets) + 1) % 3);
    if (KeyPressed(kKeyTracers)) g.tracers = !g.tracers;

    if (!g.enabled) return;

    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    Vec3 me = Invoke<Vec3>(N::GET_ENTITY_COORDS, player, 1);

    if (g.targets != Targets::Vehicles) DrawPeds(player, me);
    if (g.targets != Targets::Peds)     DrawVehicles(player, me);

    char status[96];
    std::snprintf(status, sizeof status, "ESP: %s%s", TargetsName(g.targets), g.tracers ? " | tracers" : "");
    Text(0.08f, 0.01f, status, kWhite, 0.3f);
}

void ScriptMain() {
    for (;;) {
        Tick();
        scriptWait(0);
    }
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: scriptRegister(module, ScriptMain); break;
        case DLL_PROCESS_DETACH: scriptUnregister(module); break;
    }
    return TRUE;
}
