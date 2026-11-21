#include "menu.h"
#include "features.h"
#include "photo.h"
#include <algorithm>
#include <cstdio>
#include <initializer_list>
#include <string>
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

// Favourites get a star in front of the name; the labels live as long as the garage list.
const char* CarLabel(const SavedCar& car) {
    if (!car.favorite) return car.name.c_str();
    static std::string labels[kGarageMax];
    static int next = 0;
    std::string& s = labels[next++ % kGarageMax];
    s = "* " + car.name;
    return s.c_str();
}
Item CarRow(const SavedCar& car, int index) { Item it{ ItemType::Car, CarLabel(car) }; it.index = index; return it; }

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
void DoClearBestSpeeds();
constexpr int kGarageVisibleRows = 21; // rows shown under the GARAGE header; longer lists scroll
int g_garageScroll = 0;                // first visible row under the header
bool g_pendingSave = false; // saving waits for the photo camera, so it runs after the input pass
bool g_pendingOverwrite = false;
// Saving the called car asks first: overwrite its entry (Yes) or save a new one (No, the default).
int g_overwriteAsk = -1;     // garage entry the prompt is about, or -1 when no prompt is open
bool g_overwriteYes = false;
void DoSaveCar() {
    g_overwriteAsk = GarageOverwriteTarget();
    g_overwriteYes = false;
    if (g_overwriteAsk < 0) {
        g_pendingSave = true;
        g_pendingOverwrite = false;
    }
}

const char* GarageSummary() {
    static char buf[16];
    size_t n = GarageCars().size();
    std::snprintf(buf, sizeof buf, n == 1 ? "1 car" : "%zu cars", n);
    return buf;
}

// Rebuilt every frame while the page is showing, so it always matches the garage list
// (labels point into GarageCars()). It stays in one column; Draw scrolls long lists.
std::vector<Item>& GarageItems() {
    static std::vector<Item> items;
    items.clear();
    items.push_back(Header("GARAGE"));
    const auto& cars = GarageCars();
    // The called car comes first so it can be called again quickly; it isn't repeated below.
    int called = GarageCalledIndex();
    if (called < 0 || called >= static_cast<int>(cars.size())) called = -1;
    if (called >= 0) items.push_back(CarRow(cars[called], called));
    items.push_back(Action(g_overwriteAsk >= 0 ? "Overwrite saved car?" : "Save current car", DoSaveCar));
    if (cars.empty()) items.push_back(Header("No saved cars"));
    for (int i : GarageDisplayOrder()) {
        if (i == called) continue;
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

        Header("MISC"),
        Toggle("Damage numbers", &g.damageNumbers),
        Toggle("Speedometer", &g.speedometer),
        Action("Clear best speeds", DoClearBestSpeeds),
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
int g_pendingPlate = -1;    // garage entry whose plate to edit after the input pass
int g_deleteArmed = -1;     // Garage row index waiting for a second X
bool g_garageFromHotkey = false; // Backspace closes instead of going back to the main page

std::vector<Item>& Items() { return PageItems(g_page); }

void RefreshGaragePage() {
    PageItems(kGaragePage) = GarageItems();
}
int& Sel() { return g_pageSel[g_page]; }

void DoTeleport() { g_pendingTeleport = true; }

// Every car's record is lost, so it takes a second Enter within 3 s.
void DoClearBestSpeeds() {
    static ULONGLONG armedAt = 0;
    ULONGLONG now = GetTickCount64();
    if (armedAt && now - armedAt < 3000) {
        armedAt = 0;
        BestSpeedsClear();
        Notify("Best speeds cleared");
    } else {
        armedAt = now;
        Notify("Press Enter again to clear every car's best speed");
    }
}

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
                    g_pageSel[kGaragePage] = 1; // top row: the called car if there is one, else Save
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
        case ItemType::Action:
            if (it.action != DoSaveCar || g_overwriteAsk < 0) { buf[0] = '\0'; return; }
            std::snprintf(v, sizeof v, "%s", g_overwriteYes ? "Yes" : "No");
            break;
        case ItemType::Car: {
            if (selected && g_deleteArmed == Sel()) {
                std::snprintf(buf, n, "X again to delete");
                return;
            }
            const auto& cars = GarageCars();
            const char* plate = it.index < static_cast<int>(cars.size()) ? cars[it.index].plate.c_str() : "";
            if (it.index < static_cast<int>(cars.size()) && !cars[it.index].valid) {
                std::snprintf(buf, n, "(invalid)");
                return;
            }
            // The called car's row says so instead of its plate (the preview still shows the plate).
            std::snprintf(buf, n, "%s", it.index == GarageCalledIndex() ? "CALLED" : plate);
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
        23, 73, 75,                  // INPUT_ENTER (F), INPUT_VEH_DUCK (X), INPUT_VEH_EXIT (F): garage keys
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
        bool calledTag = it.type == ItemType::Car && it.index == GarageCalledIndex() && !selected;
        if (buf[0]) LabelRight(right, y, buf, calledTag ? kHudBlue : fg);
    }
}

// The garage preview panel, one menu column wide: the car's photo (or a hint), then its details.
// Returns the panel's bottom edge.
float DrawGaragePanel(const SavedCar& car, int index, float x, float top) {
    const float w = kColW, photoH = w * AspectRatio() * 9.0f / 16.0f;
    Rect(x + w * 0.5f, top + photoH * 0.5f, w, photoH, kBarBg);
    if (!DrawPhoto(car.photo, x + w * 0.5f, top + photoH * 0.5f, w, photoH)) {
        Text(x + w * 0.5f, top + photoH * 0.5f - 0.016f, "No photo yet", kDimText, kTextScale, true, 0, false);
        Text(x + w * 0.5f, top + photoH * 0.5f + 0.002f, "Re-save the car to take one", kDimText, kSmallScale, true, 0, false);
    }

    float y = top + photoH;
    auto row = [&](const char* left, Color lc, const char* right, Color rc) {
        RowRect(x, y, w, kRowBg);
        Label(x + kPad, y, left, lc);
        if (right && *right) LabelRight(x + w - kPad, y, right, rc);
        y += kRowH;
    };
    row(car.name.c_str(), kText, nullptr, kText);
    const char* brand = car.valid ? VehicleRealBrand(car.model) : "";
    int cls = car.valid ? Invoke<int>(N::GET_VEHICLE_CLASS_FROM_NAME, car.model) : -1;
    const char* clsName = cls >= 0 && cls < kVehicleClassCount ? kVehicleClasses[cls] : "";
    if (*brand || *clsName) row(brand, kGreen, clsName, kYellow);
    row("Plate", kDimText, car.plate.c_str(), kText);
    if (car.favorite || index == GarageCalledIndex())
        row(car.favorite ? "* Favorite" : "", kYellow, index == GarageCalledIndex() ? "Called" : "", kCyan);
    if (!car.valid) row("Model not found in the game", kRed, nullptr, kText);
    return y;
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

    // The garage list scrolls under its fixed header so the selection stays in view.
    const int n = static_cast<int>(items.size());
    const bool scrolls = g_page == kGaragePage && n - 1 > kGarageVisibleRows;
    if (scrolls) {
        int row = Sel() - 1; // position under the header
        if (row < g_garageScroll) g_garageScroll = row;
        if (row >= g_garageScroll + kGarageVisibleRows) g_garageScroll = row - kGarageVisibleRows + 1;
        g_garageScroll = std::clamp(g_garageScroll, 0, n - 1 - kGarageVisibleRows);
    } else {
        g_garageScroll = 0;
    }

    // Rows, flowing into the second column at the break.
    float x = kX, y = y0, bottom = y0;
    for (int idx = 0; idx < n; ++idx) {
        const Item& it = items[idx];
        if (scrolls && idx > 0 && (idx - 1 < g_garageScroll || idx - 1 >= g_garageScroll + kGarageVisibleRows))
            continue;
        if (it.type == ItemType::ColumnBreak) {
            x += kColW + kColGap;
            y = y0;
            continue;
        }
        DrawItem(it, x, y, idx == Sel());
        y += kRowH;
        if (y > bottom) bottom = y;
    }

    // Scroll cue on the GARAGE header: how many rows are hidden above and below.
    if (scrolls) {
        int above = g_garageScroll, below = n - 1 - kGarageVisibleRows - g_garageScroll;
        char more[32] = "";
        if (above && below) std::snprintf(more, sizeof more, "%d above  %d below", above, below);
        else if (above)     std::snprintf(more, sizeof more, "%d more above", above);
        else if (below)     std::snprintf(more, sizeof more, "%d more below", below);
        LabelRight(kX + kColW - kPad, y0, more, kDimText);
    }

    // Garage preview for the selected car, in the second column.
    // While the overwrite prompt is open it shows the car that would be overwritten.
    if (g_page == kGaragePage && (g_overwriteAsk >= 0 || Items()[Sel()].type == ItemType::Car)) {
        const auto& cars = GarageCars();
        int idx = g_overwriteAsk >= 0 ? g_overwriteAsk : Items()[Sel()].index;
        if (idx >= 0 && idx < static_cast<int>(cars.size())) {
            float panelBottom = DrawGaragePanel(cars[idx], idx, kX + kColW + kColGap, y0);
            if (panelBottom > bottom) bottom = panelBottom;
        }
    }

    // Help bar under the taller column.
    RowRect(kX, bottom, totalW, kBarBg);
    const char* help =
        g_page == kGaragePage && g_overwriteAsk >= 0
            ? "Left/Right: Yes = overwrite, No = save as new    Enter: confirm    Backspace: cancel"
        : g_page == kMainPage ? "Up/Down: move    Left/Right: change    Enter: toggle    Backspace/Delete: close"
        : g_page != kGaragePage ? "Up/Down: move    Enter: toggle    Backspace: back    Delete: close"
        : g_garageFromHotkey ? "Up/Down: move    Enter: call    F: favorite    P: plate    X: delete    Backspace: close"
                             : "Up/Down: move    Enter: call    F: favorite    P: plate    X: delete    Backspace: back";
    Text(kX + totalW * 0.5f, bottom + 0.004f, help, kDimText, kSmallScale, true, 0, false);
}

} // namespace

namespace {

bool AnyKeyHeld(std::initializer_list<int> keys) {
    for (int vk : keys)
        if (GetAsyncKeyState(vk) & 0x8000) return true;
    return false;
}

// The game's on-screen keyboard, prefilled with the current plate.
void EditPlate(int index) {
    const auto& cars = GarageCars();
    if (index < 0 || index >= static_cast<int>(cars.size())) return;
    std::string current = cars[index].plate;
    Invoke(N::DISPLAY_ONSCREEN_KEYBOARD, 0, "FMMC_KEY_TIP8", "", current.c_str(), "", "", "", 8);
    int status = 0;
    while ((status = Invoke<int>(N::UPDATE_ONSCREEN_KEYBOARD)) == 0) {
        BlockGameInput();
        scriptWait(0);
    }
    std::string result;
    if (status == 1) {
        const char* r = Invoke<const char*>(N::GET_ONSCREEN_KEYBOARD_RESULT);
        if (r) result = r;
    }
    // Let go of the key that closed the keyboard, or the menu (or the pause menu) would see it.
    while (AnyKeyHeld({ VK_RETURN, VK_ESCAPE, VK_BACK })) {
        BlockGameInput();
        scriptWait(0);
    }
    if (status != 1) return;
    if (GarageSetPlate(index, result)) {
        char msg[48];
        std::snprintf(msg, sizeof msg, "Plate set to %s", GarageCars()[index].plate.c_str());
        Notify(msg);
    }
}

} // namespace

bool MenuIsOpen() { return g_open; }

void MenuToggle() {
    g_open = !g_open;
    g_page = kMainPage;
    g_garageFromHotkey = false;
    g_deleteArmed = -1;
    g_overwriteAsk = -1;
    if (!g_open) SaveSettings();
}

void MenuOpenGarage() {
    if (g_open && g_page == kGaragePage) {
        MenuToggle();
        return;
    }
    g_open = true;
    g_page = kGaragePage;
    g_pageSel[kGaragePage] = 1; // top row: the called car if there is one, else Save
    g_garageFromHotkey = true;
    g_deleteArmed = -1;
    g_overwriteAsk = -1;
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
    if (g_pendingSave) {
        g_pendingSave = false;
        GarageSaveCurrent(g_pendingOverwrite);
    }
    if (g_pendingPlate >= 0) {
        int index = g_pendingPlate;
        g_pendingPlate = -1;
        EditPlate(index);
    }
    if (!g_open) return;

    BlockGameInput();
    if (g_page != kGaragePage) g_overwriteAsk = -1;
    if (g_page == kGaragePage) {
        RefreshGaragePage();
        ClampSelection();
    }

    // The overwrite prompt takes all keys until it's answered or cancelled.
    if (g_overwriteAsk >= 0) {
        if (KeyRepeat(VK_LEFT) || KeyRepeat(VK_RIGHT)) g_overwriteYes = !g_overwriteYes;
        if (KeyPressed(VK_RETURN)) {
            g_pendingSave = true;
            g_pendingOverwrite = g_overwriteYes;
            g_overwriteAsk = -1;
        } else if (KeyPressed(VK_BACK) || KeyPressed(VK_UP) || KeyPressed(VK_DOWN)) {
            g_overwriteAsk = -1;
        }
        RefreshGaragePage();
        Draw();
        return;
    }

    // Items() is looked up per key: opening a submenu switches pages mid-pass.
    if (KeyRepeat(VK_UP))      Move(-1);
    if (KeyRepeat(VK_DOWN))    Move(1);
    if (KeyRepeat(VK_LEFT))    { g_deleteArmed = -1; Adjust(Items()[Sel()], -1); }
    if (KeyRepeat(VK_RIGHT))   { g_deleteArmed = -1; Adjust(Items()[Sel()], 1); }
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
    if (g_page == kGaragePage && Items()[Sel()].type == ItemType::Car) {
        int index = Items()[Sel()].index;
        if (KeyPressed('F')) {
            g_deleteArmed = -1;
            GarageToggleFavorite(index);
            RefreshGaragePage();
            auto& items = Items(); // keep the selection on the same car
            for (int i = 0; i < static_cast<int>(items.size()); ++i)
                if (items[i].type == ItemType::Car && items[i].index == index) Sel() = i;
        }
        if (KeyPressed('P')) {
            g_deleteArmed = -1;
            g_pendingPlate = index;
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
