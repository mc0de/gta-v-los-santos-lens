#include "menu.h"
#include "features.h"
#include <cstdio>
#include <vector>

namespace {

// StyleRow is one row of the ESP grid (a target category with a checkbox per column);
// GridHeader is the non-selectable column-title row above it.
enum class ItemType { Header, GridHeader, Bool, Int, Float, Choice, Action, StyleRow };

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
    EspStyle* style = nullptr;
};

struct GridColumn { const char* title; bool EspStyle::* field; };
const GridColumn kGridColumns[] = {
    { "On",   &EspStyle::enabled },
    { "Box",  &EspStyle::box },
    { "HP",   &EspStyle::health },
    { "Dist", &EspStyle::distance },
    { "Info", &EspStyle::labels },
    { "Line", &EspStyle::tracers },
};
constexpr int kGridCols = static_cast<int>(std::size(kGridColumns));

Item Header(const char* label) { return { ItemType::Header, label }; }
Item GridHeader() { return { ItemType::GridHeader, "" }; }
Item Toggle(const char* label, bool* v) { Item it{ ItemType::Bool, label }; it.b = v; return it; }
Item Action(const char* label, void (*fn)()) { Item it{ ItemType::Action, label }; it.action = fn; return it; }
Item StyleRow(const char* label, EspStyle* s) { Item it{ ItemType::StyleRow, label }; it.style = s; return it; }

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

const char* const kBoneLabels[]     = { "Head", "Body" };
const char* const kPriorityLabels[] = { "Nearest", "Hostiles first", "Hostiles only" };

void DoTeleport();
void DoReload();

std::vector<Item>& Items() {
    static std::vector<Item> items = {
        Header("ESP"),
        Toggle("Enabled", &g.esp),
        Toggle("Show dead", &g.showDead),
        Toggle("Distance fade", &g.distanceFade),
        Toggle("Threat arrows", &g.threatArrows),
        FloatRange("Ped range", &g.pedRange, 25, 1000, 25, "%.0f m"),
        FloatRange("Vehicle range", &g.vehRange, 25, 1000, 25, "%.0f m"),
        GridHeader(),
        StyleRow("Hostiles", &g.hostiles),
        StyleRow("Cops", &g.cops),
        StyleRow("Pedestrians", &g.pedestrians),
        StyleRow("Animals", &g.animals),
        StyleRow("Vehicles", &g.vehicles),

        Header("Finder"),
        Toggle("Collectibles", &g.collectibles),
        Toggle("Pickups", &g.pickups),
        FloatRange("Range", &g.finderRange, 50, 2000, 50, "%.0f m"),
        Toggle("Letter scraps", &g.letters),
        IntRange("Letters shown", &g.letterCount, 1, 10, 1, "%d nearest"),

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
        Action("Skip one hour", SkipHour),
        Action("Reload settings file", DoReload),
    };
    return items;
}

bool g_open = false;
int g_sel = 1;
int g_col = 0; // selected column within grid rows
bool g_pendingTeleport = false;

void DoTeleport() { g_pendingTeleport = true; }

void DoReload() {
    ReloadSettings();
    Notify("SPESP settings reloaded");
}

bool Selectable(const Item& it) {
    return it.type != ItemType::Header && it.type != ItemType::GridHeader;
}

void Move(int dir) {
    auto& items = Items();
    int n = static_cast<int>(items.size());
    do {
        g_sel = (g_sel + dir + n) % n;
    } while (!Selectable(items[g_sel]));
}

// dir: -1 / +1 for Left / Right, 0 for Enter.
void Adjust(Item& it, int dir) {
    switch (it.type) {
        case ItemType::Bool:
            *it.b = !*it.b;
            break;
        case ItemType::Int: {
            int v = *it.i + (dir ? dir : 1) * static_cast<int>(it.step);
            *it.i = v < it.min ? static_cast<int>(it.min) : v > it.max ? static_cast<int>(it.max) : v;
            break;
        }
        case ItemType::Float: {
            float v = *it.f + (dir ? dir : 1) * it.step;
            *it.f = v < it.min ? it.min : v > it.max ? it.max : v;
            break;
        }
        case ItemType::Choice:
            *it.i = (*it.i + (dir ? dir : 1) + it.choiceCount) % it.choiceCount;
            break;
        case ItemType::Action:
            if (dir == 0) it.action();
            break;
        case ItemType::StyleRow:
            // Left/Right move between cells; Enter toggles the selected one.
            if (dir == 0) {
                bool& cell = it.style->*kGridColumns[g_col].field;
                cell = !cell;
            } else {
                g_col = (g_col + dir + kGridCols) % kGridCols;
            }
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

// Menu layout (screen fractions).
constexpr float kX = 0.015f, kW = 0.25f, kRowH = 0.024f;
constexpr float kTextX = kX + 0.006f;
constexpr float kValueX = kX + 0.14f;
constexpr float kGridX = kX + 0.088f;  // centre of the first grid column
constexpr float kGridColW = 0.0285f;

float GridCellX(int col) { return kGridX + col * kGridColW; }

void DrawGridHeader(float y) {
    for (int c = 0; c < kGridCols; ++c)
        Text(GridCellX(c), y + 0.004f, kGridColumns[c].title, kYellow, 0.22f);
}

void DrawStyleRow(const Item& it, float y, bool selected) {
    Text(kTextX + 0.006f, y + 0.002f, it.label, kWhite, 0.26f, false);
    for (int c = 0; c < kGridCols; ++c) {
        float cx = GridCellX(c);
        bool on = it.style->*kGridColumns[c].field;
        if (selected && c == g_col)
            Rect(cx, y + kRowH * 0.5f, kGridColW * 0.9f, kRowH * 0.85f, { 255, 60, 255, 200 });
        Text(cx, y + 0.002f, on ? "ON" : "-", on ? kWhite : kGrey, 0.26f);
    }
}

void Draw() {
    auto& items = Items();
    float y = 0.12f;

    Rect(kX + kW * 0.5f, y + kRowH * 0.5f, kW, kRowH * 1.3f, { 20, 20, 20, 230 });
    Text(kTextX, y - 0.002f, "SPESP", kMagenta, 0.4f, false);
    y += kRowH * 1.4f;

    char buf[64];
    for (size_t idx = 0; idx < items.size(); ++idx) {
        const Item& it = items[idx];
        bool selected = static_cast<int>(idx) == g_sel;

        // Grid rows highlight just the selected cell, so their row background stays subtle.
        Color bg = !selected ? Color{ 0, 0, 0, 170 }
                 : it.type == ItemType::StyleRow ? Color{ 255, 60, 255, 60 }
                 : Color{ 255, 60, 255, 140 };
        Rect(kX + kW * 0.5f, y + kRowH * 0.5f, kW, kRowH, bg);

        switch (it.type) {
            case ItemType::Header:
                Text(kTextX, y + 0.002f, it.label, kYellow, 0.28f, false);
                break;
            case ItemType::GridHeader:
                DrawGridHeader(y);
                break;
            case ItemType::StyleRow:
                DrawStyleRow(it, y, selected);
                break;
            default:
                Text(kTextX + 0.006f, y + 0.002f, it.label, kWhite, 0.26f, false);
                FormatValue(it, buf, sizeof buf);
                Text(kValueX, y + 0.002f, buf, it.type == ItemType::Bool && !*it.b ? kGrey : kWhite, 0.26f, false);
                break;
        }
        y += kRowH;
    }

    Rect(kX + kW * 0.5f, y + kRowH * 0.5f, kW, kRowH, { 20, 20, 20, 230 });
    Text(kTextX, y + 0.003f, "Arrows: move/change   Enter: toggle   Backspace/Delete: close", kGrey, 0.2f, false);
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
    if (KeyRepeat(VK_UP))      Move(-1);
    if (KeyRepeat(VK_DOWN))    Move(1);
    if (KeyRepeat(VK_LEFT))    Adjust(items[g_sel], -1);
    if (KeyRepeat(VK_RIGHT))   Adjust(items[g_sel], 1);
    if (KeyPressed(VK_RETURN)) Adjust(items[g_sel], 0);
    if (KeyPressed(VK_BACK))   MenuToggle();

    Draw();
}
