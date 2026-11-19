#include "menu.h"
#include "features.h"
#include <cstdio>
#include <vector>

namespace {

// StyleRow is one row of the ESP grid (a target category with a checkbox per column);
// GridHeader is the non-selectable column-title row above it. ColumnBreak starts the
// menu's second column; navigation runs down the first column and on into the second.
// Submenu opens another page of items; Backspace returns to the main page.
// Car is one saved car on the Garage page (Enter calls it, X twice deletes it).
enum class ItemType { Header, GridHeader, ColumnBreak, Bool, Int, Float, Choice, Action, StyleRow, Submenu, Car };

enum Page { kMainPage, kVehicleTypesPage, kGaragePage, kPageCount };

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
    Page page = kMainPage;            // Submenu target
    const char* (*value)() = nullptr; // Submenu value text
    int index = -1;                   // Car: garage entry
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
Item Submenu(const char* label, Page page, const char* (*value)()) {
    Item it{ ItemType::Submenu, label }; it.page = page; it.value = value; return it;
}

Item CarRow(const SavedCar& car, int index) { Item it{ ItemType::Car, car.name.c_str() }; it.index = index; return it; }

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
void DoSaveCar() { GarageSaveCurrent(); }

const char* GarageSummary() {
    static char buf[16];
    size_t n = GarageCars().size();
    std::snprintf(buf, sizeof buf, n == 1 ? "1 car" : "%zu cars", n);
    return buf;
}

// Rebuilt every frame while the page is showing, so it always matches the garage list
// (labels point into GarageCars()). Cars past 20 continue in the second column.
std::vector<Item>& GarageItems() {
    static std::vector<Item> items;
    items.clear();
    items.push_back(Header("GARAGE"));
    items.push_back(Action("Save current car", DoSaveCar));
    const auto& cars = GarageCars();
    if (cars.empty()) items.push_back(Header("No saved cars"));
    for (int i = 0; i < static_cast<int>(cars.size()); ++i) {
        if (i == 20) {
            items.push_back(ColumnBreak());
            items.push_back(Header(""));
        }
        items.push_back(CarRow(cars[i], i));
    }
    return items;
}
void ShowAllVehicleTypes() { g.vehTypes.fill(true); }
void ShowNoVehicleTypes()  { g.vehTypes.fill(false); }

// One checkbox per vehicle class, split over the two columns.
std::vector<Item> BuildVehicleTypesPage() {
    std::vector<Item> items = {
        Header("VEHICLE TYPES"),
        Action("Show all", ShowAllVehicleTypes),
        Action("Show none", ShowNoVehicleTypes),
    };
    const int firstColumn = (kVehicleClassCount + 1) / 2;
    for (int i = 0; i < kVehicleClassCount; ++i) {
        if (i == firstColumn) {
            items.push_back(ColumnBreak());
            items.push_back(Header(""));
        }
        items.push_back(Toggle(kVehicleClasses[i], &g.vehTypes[i]));
    }
    return items;
}

std::vector<Item>& PageItems(Page page) {
    static std::vector<Item> vehicleTypes = BuildVehicleTypesPage();
    static std::vector<Item> main = {
        Header("ESP"),
        Toggle("Enabled", &g.esp),
        Toggle("Show dead", &g.showDead),
        Toggle("Distance fade", &g.distanceFade),
        Toggle("Threat arrows", &g.threatArrows),
        FloatRange("Ped range", &g.pedRange, 25, 1000, 25, "%.0f m"),
        FloatRange("Vehicle range", &g.vehRange, 25, 1000, 25, "%.0f m"),
        Submenu("Vehicle types", kVehicleTypesPage, VehicleTypesSummary),
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
        Submenu("Garage", kGaragePage, GarageSummary),
        Action("Reload settings file", DoReload),
    };
    static std::vector<Item> garage;
    if (page == kGaragePage) return garage;
    return page == kVehicleTypesPage ? vehicleTypes : main;
}

bool g_open = false;
Page g_page = kMainPage;
int g_pageSel[kPageCount] = { 1, 1, 1 }; // selected item on each page (index 1: first row after the header)
int g_col = 0; // selected column within grid rows
bool g_pendingTeleport = false;
int g_pendingCall = -1;     // garage entry to call after the input pass
int g_deleteArmed = -1;     // Garage row index waiting for a second X
bool g_garageFromHotkey = false; // Backspace closes instead of going back to the main page

std::vector<Item>& Items() { return PageItems(g_page); }

void RefreshGaragePage() {
    PageItems(kGaragePage) = GarageItems();
}
int& Sel() { return g_pageSel[g_page]; }

void DoTeleport() { g_pendingTeleport = true; }

void DoReload() {
    ReloadSettings();
    Notify("SPESP settings reloaded");
}

bool Selectable(const Item& it) {
    return it.type != ItemType::Header && it.type != ItemType::GridHeader && it.type != ItemType::ColumnBreak;
}

// Puts the selection on a selectable row (pages can shrink, e.g. after a delete).
void ClampSelection() {
    auto& items = Items();
    int n = static_cast<int>(items.size());
    int& sel = Sel();
    if (sel >= n) sel = n - 1;
    if (sel < 0) sel = 0;
    for (int tries = 0; tries < n && !Selectable(items[sel]); ++tries)
        sel = (sel + n - 1) % n;
}

void Move(int dir) {
    g_deleteArmed = -1;
    auto& items = Items();
    int n = static_cast<int>(items.size());
    int& sel = Sel();
    do {
        sel = (sel + dir + n) % n;
    } while (!Selectable(items[sel]));
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
        case ItemType::Submenu:
            if (dir >= 0) {
                g_page = it.page;
                g_garageFromHotkey = false;
                if (g_page == kGaragePage) { // later keys this frame index the new page
                    RefreshGaragePage();
                    ClampSelection();
                }
            }
            break;
        case ItemType::Car:
            if (dir == 0) g_pendingCall = it.index;
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
        case ItemType::Submenu:
            std::snprintf(buf, n, "%s >", it.value ? it.value() : "");
            return;
        case ItemType::Car: {
            if (selected && g_deleteArmed == Sel()) {
                std::snprintf(buf, n, "X again to delete");
                return;
            }
            const auto& cars = GarageCars();
            const char* plate = it.index < static_cast<int>(cars.size()) ? cars[it.index].plate.c_str() : "";
            std::snprintf(buf, n, "%s%s", plate, it.index == GarageCalledIndex() ? "  (out)" : "");
            return;
        }
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
        if (static_cast<int>(i) == Sel()) selIndex = selCount;
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
        DrawItem(it, x, y, static_cast<int>(idx) == Sel());
        y += kRowH;
        if (y > bottom) bottom = y;
    }

    // Help bar under the taller column.
    RowRect(kX, bottom, totalW, kBarBg);
    const char* help =
        g_page == kMainPage ? "Up/Down: move    Left/Right: change    Enter: toggle    Backspace/Delete: close"
        : g_page != kGaragePage ? "Up/Down: move    Enter: toggle    Backspace: back    Delete: close"
        : g_garageFromHotkey ? "Up/Down: move    Enter: call    X: delete    Backspace: close"
                             : "Up/Down: move    Enter: call    X: delete    Backspace: back    Delete: close";
    Text(kX + totalW * 0.5f, bottom + 0.004f, help, kDimText, kSmallScale, true, 0, false);
}

} // namespace

bool MenuIsOpen() { return g_open; }

void MenuToggle() {
    g_open = !g_open;
    g_page = kMainPage;
    g_garageFromHotkey = false;
    g_deleteArmed = -1;
    if (!g_open) SaveSettings();
}

void MenuOpenGarage() {
    if (g_open && g_page == kGaragePage) {
        MenuToggle();
        return;
    }
    g_open = true;
    g_page = kGaragePage;
    g_garageFromHotkey = true;
    g_deleteArmed = -1;
}

void MenuTick() {
    // Actions that call scriptWait (teleport, calling a car) run after the menu's input pass.
    if (g_pendingTeleport) {
        g_pendingTeleport = false;
        TeleportToWaypoint();
    }
    if (g_pendingCall >= 0) {
        int index = g_pendingCall;
        g_pendingCall = -1;
        GarageCall(index);
    }
    if (!g_open) return;

    BlockGameInput();
    if (g_page == kGaragePage) {
        RefreshGaragePage();
        ClampSelection();
    }

    // Items() is looked up per key: opening a submenu switches pages mid-pass.
    if (KeyRepeat(VK_UP))      Move(-1);
    if (KeyRepeat(VK_DOWN))    Move(1);
    if (KeyRepeat(VK_LEFT))    Adjust(Items()[Sel()], -1);
    if (KeyRepeat(VK_RIGHT))   Adjust(Items()[Sel()], 1);
    if (KeyPressed(VK_RETURN)) {
        g_deleteArmed = -1;
        Adjust(Items()[Sel()], 0);
        if (g_page == kGaragePage) { // a save may have grown the list (labels point into it)
            RefreshGaragePage();
            ClampSelection();
        }
        if (g_pendingCall >= 0) { // calling a car closes the menu so you can walk to it
            MenuToggle();
            return;
        }
    }
    if (g_page == kGaragePage && KeyPressed('X')) {
        const Item& it = Items()[Sel()];
        if (it.type == ItemType::Car) {
            if (g_deleteArmed == Sel()) {
                g_deleteArmed = -1;
                GarageDelete(it.index);
                RefreshGaragePage();
                ClampSelection();
            } else {
                g_deleteArmed = Sel();
            }
        }
    }
    if (KeyPressed(VK_BACK)) {
        g_deleteArmed = -1;
        if (g_page == kGaragePage && g_garageFromHotkey) { MenuToggle(); return; }
        if (g_page != kMainPage) g_page = kMainPage;
        else                     { MenuToggle(); return; }
    }

    Draw();
}
