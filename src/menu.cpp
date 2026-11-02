#include "menu.h"
#include "features.h"
#include <cstdio>
#include <vector>

namespace {

enum class ItemType { Header, Bool, Int, Float, Choice, Action };

struct Item {
    ItemType type;
    const char* label;
    bool* b = nullptr;
    int* i = nullptr;
    float* f = nullptr;
    float min = 0, max = 0, step = 0;
    const char* fmt = nullptr;        // printf format for Int/Float values
    const char* maxLabel = nullptr;   // shown instead of the number at max (e.g. "any")
    const char* const* choices = nullptr;
    int choiceCount = 0;
    void (*action)() = nullptr;
};

Item Header(const char* label) { return { ItemType::Header, label }; }
Item Toggle(const char* label, bool* v) { Item it{ ItemType::Bool, label }; it.b = v; return it; }
Item Action(const char* label, void (*fn)()) { Item it{ ItemType::Action, label }; it.action = fn; return it; }

Item IntRange(const char* label, int* v, int min, int max, int step, const char* fmt, const char* maxLabel = nullptr) {
    Item it{ ItemType::Int, label };
    it.i = v; it.min = float(min); it.max = float(max); it.step = float(step); it.fmt = fmt; it.maxLabel = maxLabel;
    return it;
}

Item FloatRange(const char* label, float* v, float min, float max, float step, const char* fmt) {
    Item it{ ItemType::Float, label };
    it.f = v; it.min = min; it.max = max; it.step = step; it.fmt = fmt;
    return it;
}

template <typename E, int N>
Item Choice(const char* label, E* v, const char* const (&names)[N]) {
    static_assert(sizeof(E) == sizeof(int), "choice enums must be int-sized");
    Item it{ ItemType::Choice, label };
    it.i = reinterpret_cast<int*>(v); it.choices = names; it.choiceCount = N;
    return it;
}

const char* const kTargetLabels[]   = { "Peds", "Vehicles", "Both" };
const char* const kBoneLabels[]     = { "Head", "Body" };
const char* const kPriorityLabels[] = { "Nearest", "Hostiles first", "Hostiles only" };

void DoTeleport();
void DoReload();

std::vector<Item>& Items() {
    static std::vector<Item> items = {
        Header("ESP"),
        Toggle("Enabled", &g.esp),
        Choice("Show", &g.targets, kTargetLabels),
        Toggle("Boxes", &g.boxes),
        Toggle("Skeletons", &g.skeletons),
        Toggle("Health bars", &g.healthBars),
        Toggle("Labels", &g.labels),
        Toggle("Weapon labels", &g.weaponLabels),
        Toggle("Tracers", &g.tracers),
        Toggle("Distance fade", &g.distanceFade),
        Toggle("Threat arrows", &g.threatArrows),
        Toggle("Ignore animals", &g.espIgnoreAnimals),
        FloatRange("Ped range", &g.pedRange, 25, 1000, 25, "%.0f m"),
        FloatRange("Vehicle range", &g.vehRange, 25, 1000, 25, "%.0f m"),

        Header("Finder"),
        Toggle("Collectibles", &g.collectibles),
        Toggle("Pickups", &g.pickups),
        FloatRange("Range", &g.finderRange, 50, 2000, 50, "%.0f m"),

        Header("Auto-aim"),
        Toggle("Enabled", &g.aim),
        Choice("Aim at", &g.aimBone, kBoneLabels),
        Choice("Priority", &g.aimPriority, kPriorityLabels),
        Toggle("Ignore cops", &g.aimIgnoreCops),
        Toggle("Ignore animals", &g.aimIgnoreAnimals),
        IntRange("Smoothing", &g.aimSmoothing, 1, 20, 1, "%d"),
        IntRange("Aim zone", &g.aimFov, 5, 180, 5, "%d deg", "any"),
        Toggle("Show aim zone", &g.showFov),

        Header("Misc"),
        Toggle("Damage numbers", &g.damageNumbers),
        Action("Teleport to waypoint", DoTeleport),
        Action("Reload settings file", DoReload),
    };
    return items;
}

bool g_open = false;
int g_sel = 1;
bool g_pendingTeleport = false;

void DoTeleport() { g_pendingTeleport = true; }

void DoReload() {
    ReloadSettings();
    Notify("SPESP settings reloaded");
}

void Move(int dir) {
    auto& items = Items();
    int n = static_cast<int>(items.size());
    do {
        g_sel = (g_sel + dir + n) % n;
    } while (items[g_sel].type == ItemType::Header);
}

void Adjust(Item& it, int dir) {
    switch (it.type) {
        case ItemType::Bool:
            *it.b = !*it.b;
            break;
        case ItemType::Int: {
            int v = *it.i + dir * static_cast<int>(it.step);
            *it.i = v < it.min ? static_cast<int>(it.min) : v > it.max ? static_cast<int>(it.max) : v;
            break;
        }
        case ItemType::Float: {
            float v = *it.f + dir * it.step;
            *it.f = v < it.min ? it.min : v > it.max ? it.max : v;
            break;
        }
        case ItemType::Choice:
            *it.i = (*it.i + dir + it.choiceCount) % it.choiceCount;
            break;
        case ItemType::Action:
            if (dir == 0) it.action();
            break;
        default:
            break;
    }
}

void FormatValue(const Item& it, char* buf, size_t n) {
    switch (it.type) {
        case ItemType::Bool:   std::snprintf(buf, n, "%s", *it.b ? "ON" : "off"); break;
        case ItemType::Choice: std::snprintf(buf, n, "< %s >", it.choices[*it.i]); break;
        case ItemType::Int:
            if (it.maxLabel && *it.i >= it.max) std::snprintf(buf, n, "%s", it.maxLabel);
            else std::snprintf(buf, n, it.fmt, *it.i);
            break;
        case ItemType::Float:  std::snprintf(buf, n, it.fmt, *it.f); break;
        default:               buf[0] = '\0'; break;
    }
}

// Stops the arrow keys / Enter / Backspace from also driving the phone and frontend.
void BlockGameInput() {
    static const int kControls[] = {
        27,                          // INPUT_PHONE
        172, 173, 174, 175, 176, 177, // INPUT_CELLPHONE_UP/DOWN/LEFT/RIGHT/SELECT/CANCEL
        199, 200, 201, 202,          // INPUT_FRONTEND_PAUSE / PAUSE_ALTERNATE / ACCEPT / CANCEL
    };
    for (int c : kControls)
        Invoke(N::DISABLE_CONTROL_ACTION, 0, c, 1);
}

void Draw() {
    auto& items = Items();
    const float x = 0.015f, w = 0.19f, rowH = 0.024f;
    float y = 0.12f;
    const float textX = x + 0.006f, valueX = x + 0.125f;

    Rect(x + w * 0.5f, y + rowH * 0.5f, w, rowH * 1.3f, { 20, 20, 20, 230 });
    Text(textX, y - 0.002f, "SPESP", kMagenta, 0.4f, false);
    y += rowH * 1.4f;

    char buf[64];
    for (size_t idx = 0; idx < items.size(); ++idx) {
        const Item& it = items[idx];
        bool selected = static_cast<int>(idx) == g_sel;
        Color bg = selected ? Color{ 255, 60, 255, 140 } : Color{ 0, 0, 0, 170 };
        Rect(x + w * 0.5f, y + rowH * 0.5f, w, rowH, bg);

        if (it.type == ItemType::Header) {
            Text(textX, y + 0.002f, it.label, kYellow, 0.28f, false);
        } else {
            Text(textX + 0.006f, y + 0.002f, it.label, kWhite, 0.26f, false);
            FormatValue(it, buf, sizeof buf);
            Text(valueX, y + 0.002f, buf, it.type == ItemType::Bool && !*it.b ? kGrey : kWhite, 0.26f, false);
        }
        y += rowH;
    }

    Rect(x + w * 0.5f, y + rowH * 0.5f, w, rowH, { 20, 20, 20, 230 });
    Text(textX, y + 0.003f, "Arrows: move/change   Enter: select   Backspace/Delete: close", kGrey, 0.2f, false);
}

} // namespace

bool MenuIsOpen() { return g_open; }

void MenuToggle() {
    g_open = !g_open;
    if (!g_open) SaveSettings();
}

void MenuTick() {
    // Actions that call scriptWait (teleport) run after the menu's input pass.
    if (g_pendingTeleport) {
        g_pendingTeleport = false;
        TeleportToWaypoint();
    }
    if (!g_open) return;

    BlockGameInput();

    auto& items = Items();
    if (KeyRepeat(VK_UP))    Move(-1);
    if (KeyRepeat(VK_DOWN))  Move(1);
    if (KeyRepeat(VK_LEFT))  Adjust(items[g_sel], -1);
    if (KeyRepeat(VK_RIGHT)) Adjust(items[g_sel], 1);
    if (KeyPressed(VK_RETURN)) {
        Item& it = items[g_sel];
        Adjust(it, it.type == ItemType::Action ? 0 : 1);
    }
    if (KeyPressed(VK_BACK)) MenuToggle();

    Draw();
}
