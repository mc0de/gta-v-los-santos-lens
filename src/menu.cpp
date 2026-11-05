#include "menu.h"
#include "features.h"
#include <cstdio>
#include <vector>

namespace {

// StyleRow is one row of the ESP grid (a target category with a checkbox per column);
// GridHeader is the non-selectable column-title row above it. ColumnBreak starts the
// menu's second column; navigation runs down the first column and on into the second.
enum class ItemType { Header, GridHeader, ColumnBreak, Bool, Int, Float, Choice, Action, StyleRow };

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
Item ColumnBreak() { return { ItemType::ColumnBreak, "" }; }
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
const char* const kPickLabels[]     = { "Crosshair", "Nearest" };

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

        Header("FINDER"),
        Toggle("Collectibles", &g.collectibles),
        Toggle("Pickups", &g.pickups),
        FloatRange("Range", &g.finderRange, 50, 2000, 50, "%.0f m"),
        Toggle("Letter scraps", &g.letters),
        IntRange("Letters shown", &g.letterCount, 1, 10, 1, "%d nearest"),

        ColumnBreak(),
        Header("AUTO-AIM"),
        Toggle("Enabled", &g.aim),
        Choice("Aim at", &g.aimBone, kBoneLabels),
        Choice("Pick", &g.aimPick, kPickLabels),
        Toggle("Target cops", &g.aimCops),
        Toggle("Target pedestrians", &g.aimPedestrians),
        Toggle("Target animals", &g.aimAnimals),
        FloatRange("Range", &g.aimRange, 25, 1000, 25, "%.0f m"),
        IntRange("Aim zone", &g.aimFov, 5, 180, 5, "%d deg", "any"),
        IntRange("Smoothing", &g.aimSmoothing, 1, 20, 1, "%d"),

        Header("MISC"),
        Toggle("Damage numbers", &g.damageNumbers),
        Toggle("Speedometer", &g.speedometer),
        Action("Teleport to waypoint", DoTeleport),
        Action("Skip time", SkipTime),
        IntRange("Hours to skip", &g.skipHours, 1, 23, 1, "%d h"),
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
    return it.type != ItemType::Header && it.type != ItemType::GridHeader && it.type != ItemType::ColumnBreak;
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

// Value text; the selected row gets GTA-style arrows on adjustable values.
void FormatValue(const Item& it, bool selected, char* buf, size_t n) {
    char v[48];
    switch (it.type) {
        case ItemType::Choice: std::snprintf(v, sizeof v, "%s", it.choices[*it.i]); break;
        case ItemType::Int:
            if (it.maxLabel && *it.i >= it.max) std::snprintf(v, sizeof v, "%s", it.maxLabel);
            else std::snprintf(v, sizeof v, it.fmt, *it.i);
            break;
        case ItemType::Float:  std::snprintf(v, sizeof v, it.fmt, *it.f); break;
        default:               buf[0] = '\0'; return;
    }
    if (selected) std::snprintf(buf, n, "< %s >", v);
    else          std::snprintf(buf, n, "%s", v);
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

// Menu layout (screen fractions): a banner across two columns of rows, GTA interaction-menu style.
constexpr float kX = 0.015f, kTop = 0.04f;
constexpr float kColW = 0.235f, kColGap = 0.004f;
constexpr float kBannerH = 0.065f, kRowH = 0.022f;
constexpr float kPad = 0.006f;                // text inset from the row edges
constexpr float kTextScale = 0.24f, kSmallScale = 0.2f;
constexpr float kTextDy = 0.002f;             // text baseline offset inside a row
constexpr float kGridX = 0.085f;              // centre of the first grid column, from the column's left edge
constexpr float kGridColW = 0.0245f;

constexpr Color kBanner   {  30,  95, 175, 240 };
constexpr Color kBarBg    {   0,   0,   0, 235 };
constexpr Color kRowBg    {   0,   0,   0, 165 };
constexpr Color kSelBg    { 240, 240, 240, 235 };
constexpr Color kGridSelBg{ 255, 255, 255,  45 };
constexpr Color kHudBlue  {  93, 182, 229, 255 };
constexpr Color kText     { 245, 245, 245, 255 };
constexpr Color kSelText  {  10,  10,  10, 255 };
constexpr Color kDimText  { 150, 150, 150, 255 };

void RowRect(float x, float y, float w, Color c) { Rect(x + w * 0.5f, y + kRowH * 0.5f, w, kRowH, c); }
void Label(float x, float y, const char* s, Color c, float scale = kTextScale) { Text(x, y + kTextDy, s, c, scale, false, 0, false); }
void LabelRight(float right, float y, const char* s, Color c) { Label(right - TextWidth(s, kTextScale), y, s, c); }

// A GTA-style checkbox: a frame, filled when on.
void Checkbox(float cx, float cy, bool on, Color fg, Color bg) {
    const float h = 0.013f, w = h / AspectRatio();
    const float bh = 0.0018f, bw = bh / AspectRatio();
    Rect(cx, cy, w, h, fg);
    Rect(cx, cy, w - 2 * bw, h - 2 * bh, bg);
    if (on) Rect(cx, cy, w - 4 * bw, h - 4 * bh, fg);
}

float GridCellX(float colX, int col) { return colX + kGridX + col * kGridColW; }

void DrawGridHeader(float x, float y) {
    RowRect(x, y, kColW, kRowBg);
    for (int c = 0; c < kGridCols; ++c)
        Text(GridCellX(x, c), y + 0.004f, kGridColumns[c].title, kDimText, kSmallScale, true, 0, false);
}

void DrawStyleRow(const Item& it, float x, float y, bool selected) {
    RowRect(x, y, kColW, selected ? kGridSelBg : kRowBg);
    Label(x + kPad, y, it.label, kText);
    for (int c = 0; c < kGridCols; ++c) {
        float cx = GridCellX(x, c), cy = y + kRowH * 0.5f;
        bool cell = selected && c == g_col;
        if (cell) Rect(cx, cy, kGridColW * 0.92f, kRowH * 0.9f, kSelBg);
        bool on = it.style->*kGridColumns[c].field;
        Checkbox(cx, cy, on, cell ? kSelText : on ? kText : kDimText, cell ? kSelBg : kRowBg);
    }
}

void DrawItem(const Item& it, float x, float y, bool selected) {
    switch (it.type) {
        case ItemType::Header:
            RowRect(x, y, kColW, kBarBg);
            Label(x + kPad, y, it.label, kHudBlue);
            return;
        case ItemType::GridHeader:
            DrawGridHeader(x, y);
            return;
        case ItemType::StyleRow:
            DrawStyleRow(it, x, y, selected);
            return;
        default:
            break;
    }

    Color bg = selected ? kSelBg : kRowBg;
    Color fg = selected ? kSelText : kText;
    RowRect(x, y, kColW, bg);
    Label(x + kPad, y, it.label, fg);

    float right = x + kColW - kPad;
    if (it.type == ItemType::Bool) {
        float h = 0.013f / AspectRatio();
        Checkbox(right - h * 0.5f, y + kRowH * 0.5f, *it.b, fg, bg);
    } else {
        char buf[64];
        FormatValue(it, selected, buf, sizeof buf);
        if (buf[0]) LabelRight(right, y, buf, fg);
    }
}

void Draw() {
    auto& items = Items();
    const float totalW = 2 * kColW + kColGap;

    // Banner and subtitle bar with the position counter.
    Rect(kX + totalW * 0.5f, kTop + kBannerH * 0.5f, totalW, kBannerH, kBanner);
    Text(kX + totalW * 0.5f, kTop + 0.004f, "SPESP", kText, 0.85f, true, 1, false);

    int selIndex = 0, selCount = 0;
    for (size_t i = 0; i < items.size(); ++i) {
        if (!Selectable(items[i])) continue;
        ++selCount;
        if (static_cast<int>(i) == g_sel) selIndex = selCount;
    }
    float y0 = kTop + kBannerH;
    RowRect(kX, y0, totalW, kBarBg);
    Label(kX + kPad, y0, "STORY MODE OVERLAY", kHudBlue);
    char counter[16];
    std::snprintf(counter, sizeof counter, "%d / %d", selIndex, selCount);
    LabelRight(kX + totalW - kPad, y0, counter, kText);
    y0 += kRowH;

    // Rows, flowing into the second column at the break.
    float x = kX, y = y0, bottom = y0;
    for (size_t idx = 0; idx < items.size(); ++idx) {
        const Item& it = items[idx];
        if (it.type == ItemType::ColumnBreak) {
            x += kColW + kColGap;
            y = y0;
            continue;
        }
        DrawItem(it, x, y, static_cast<int>(idx) == g_sel);
        y += kRowH;
        if (y > bottom) bottom = y;
    }

    // Help bar under the taller column.
    RowRect(kX, bottom, totalW, kBarBg);
    Text(kX + totalW * 0.5f, bottom + 0.004f,
         "Up/Down: move    Left/Right: change    Enter: toggle    Backspace/Delete: close",
         kDimText, kSmallScale, true, 0, false);
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
