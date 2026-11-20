// Garage: save the car you're in (with all its customisation) and call it back later.
// Saved cars live in SPESP_garage.ini next to the plugin. The only world writes here are to the
// car we spawn ourselves.

#include "features.h"
#include "photo.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

std::vector<SavedCar> g_cars;

// Mod slots 17-22 are toggles (turbo, tyre smoke, xenon); the rest hold an index.
constexpr int kModTurbo = 18, kModTyreSmoke = 20, kModXenon = 22;
constexpr int kModFrontWheels = 23, kModRearWheels = 24;
bool IsToggleSlot(int slot) { return slot >= 17 && slot <= 22; }

constexpr int kPlateMax = 8;

std::string GaragePath() {
    std::string p = IniPath();
    size_t dot = p.find_last_of('.');
    return (dot == std::string::npos ? p : p.substr(0, dot)) + "_garage.ini";
}

bool IsRoadVehicle(Hash model) {
    return Invoke<int>(N::IS_THIS_MODEL_A_CAR, model) || Invoke<int>(N::IS_THIS_MODEL_A_BIKE, model) ||
           Invoke<int>(N::IS_THIS_MODEL_A_QUADBIKE, model) || Invoke<int>(N::IS_THIS_MODEL_A_BICYCLE, model);
}

std::string CarName(Hash model) {
    std::string make = VehicleMakeName(model), name = VehicleModelName(model);
    return make.empty() ? name : make + " " + name;
}

SavedCar Capture(Vehicle v) {
    SavedCar c;
    c.model = Invoke<Hash>(N::GET_ENTITY_MODEL, v);
    c.name = CarName(c.model);

    for (int slot = 0; slot < 50; ++slot)
        if (!IsToggleSlot(slot)) c.mods[slot] = Invoke<int>(N::GET_VEHICLE_MOD, v, slot);
    c.modVariation[0] = Invoke<int>(N::GET_VEHICLE_MOD_VARIATION, v, kModFrontWheels) != 0;
    c.modVariation[1] = Invoke<int>(N::GET_VEHICLE_MOD_VARIATION, v, kModRearWheels) != 0;
    c.turbo     = Invoke<int>(N::IS_TOGGLE_MOD_ON, v, kModTurbo) != 0;
    c.tyreSmoke = Invoke<int>(N::IS_TOGGLE_MOD_ON, v, kModTyreSmoke) != 0;
    c.xenon     = Invoke<int>(N::IS_TOGGLE_MOD_ON, v, kModXenon) != 0;
    c.wheelType = Invoke<int>(N::GET_VEHICLE_WHEEL_TYPE, v);

    Invoke(N::GET_VEHICLE_COLOURS, v, &c.primary, &c.secondary);
    Invoke(N::GET_VEHICLE_EXTRA_COLOURS, v, &c.pearl, &c.wheelColour);
    Invoke(N::GET_VEHICLE_MOD_COLOR_1, v, &c.paint1Type, &c.paint1Colour, &c.paint1Pearl);
    Invoke(N::GET_VEHICLE_MOD_COLOR_2, v, &c.paint2Type, &c.paint2Colour);
    c.customPrimary = Invoke<int>(N::GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM, v) != 0;
    c.customSecondary = Invoke<int>(N::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM, v) != 0;
    if (c.customPrimary)
        Invoke(N::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR, v, &c.customPrimaryRgb[0], &c.customPrimaryRgb[1], &c.customPrimaryRgb[2]);
    if (c.customSecondary)
        Invoke(N::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR, v, &c.customSecondaryRgb[0], &c.customSecondaryRgb[1], &c.customSecondaryRgb[2]);
    c.colourCombination = Invoke<int>(N::GET_VEHICLE_COLOUR_COMBINATION, v);

    for (int i = 0; i < 15; ++i)
        c.extras[i] = Invoke<int>(N::DOES_EXTRA_EXIST, v, i) ? (Invoke<int>(N::IS_VEHICLE_EXTRA_TURNED_ON, v, i) ? 1 : 0) : -1;
    c.livery = Invoke<int>(N::GET_VEHICLE_LIVERY, v);
    c.windowTint = Invoke<int>(N::GET_VEHICLE_WINDOW_TINT, v);
    for (int i = 0; i < 4; ++i)
        c.neon[i] = Invoke<int>(N::IS_VEHICLE_NEON_LIGHT_ENABLED, v, i) != 0;
    Invoke(N::GET_VEHICLE_NEON_LIGHTS_COLOUR, v, &c.neonRgb[0], &c.neonRgb[1], &c.neonRgb[2]);
    Invoke(N::GET_VEHICLE_TYRE_SMOKE_COLOR, v, &c.smokeRgb[0], &c.smokeRgb[1], &c.smokeRgb[2]);

    const char* plate = Invoke<const char*>(N::GET_VEHICLE_NUMBER_PLATE_TEXT, v);
    c.plate = plate ? plate : "";
    if (c.plate.size() > kPlateMax) c.plate.resize(kPlateMax);
    c.plateStyle = Invoke<int>(N::GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX, v);
    c.bulletproofTyres = !Invoke<int>(N::GET_VEHICLE_TYRES_CAN_BURST, v);
    return c;
}

// Only ever called on the car GarageCall just created. Order matters: the mod kit before any mod,
// wheel type before wheel mods, and paint types before the colours they'd otherwise reset.
void ApplySavedCar(Vehicle v, const SavedCar& c) {
    Invoke(N::SET_VEHICLE_MOD_KIT, v, 0);
    if (c.wheelType >= 0) Invoke(N::SET_VEHICLE_WHEEL_TYPE, v, c.wheelType);

    if (c.paint1Type >= 0) Invoke(N::SET_VEHICLE_MOD_COLOR_1, v, c.paint1Type, c.paint1Colour, c.paint1Pearl);
    if (c.paint2Type >= 0) Invoke(N::SET_VEHICLE_MOD_COLOR_2, v, c.paint2Type, c.paint2Colour);
    // The combination picks colours of its own, so it only applies when no colours were saved.
    if (c.primary >= 0 && c.secondary >= 0) Invoke(N::SET_VEHICLE_COLOURS, v, c.primary, c.secondary);
    else if (c.colourCombination >= 0)       Invoke(N::SET_VEHICLE_COLOUR_COMBINATION, v, c.colourCombination);
    if (c.pearl >= 0 && c.wheelColour >= 0) Invoke(N::SET_VEHICLE_EXTRA_COLOURS, v, c.pearl, c.wheelColour);
    if (c.customPrimary)
        Invoke(N::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR, v, c.customPrimaryRgb[0], c.customPrimaryRgb[1], c.customPrimaryRgb[2]);
    if (c.customSecondary)
        Invoke(N::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR, v, c.customSecondaryRgb[0], c.customSecondaryRgb[1], c.customSecondaryRgb[2]);

    for (int slot = 0; slot < 50; ++slot) {
        if (IsToggleSlot(slot) || c.mods[slot] < 0) continue;
        bool customTyres = (slot == kModFrontWheels && c.modVariation[0]) || (slot == kModRearWheels && c.modVariation[1]);
        Invoke(N::SET_VEHICLE_MOD, v, slot, c.mods[slot], customTyres ? 1 : 0);
    }
    Invoke(N::TOGGLE_VEHICLE_MOD, v, kModTurbo, c.turbo ? 1 : 0);
    Invoke(N::TOGGLE_VEHICLE_MOD, v, kModTyreSmoke, c.tyreSmoke ? 1 : 0);
    Invoke(N::TOGGLE_VEHICLE_MOD, v, kModXenon, c.xenon ? 1 : 0);

    for (int i = 0; i < 15; ++i)
        if (c.extras[i] >= 0 && Invoke<int>(N::DOES_EXTRA_EXIST, v, i))
            Invoke(N::SET_VEHICLE_EXTRA, v, i, c.extras[i] ? 0 : 1); // 0 = on
    if (c.livery >= 0) Invoke(N::SET_VEHICLE_LIVERY, v, c.livery);
    if (c.windowTint >= 0) Invoke(N::SET_VEHICLE_WINDOW_TINT, v, c.windowTint);
    for (int i = 0; i < 4; ++i)
        Invoke(N::SET_VEHICLE_NEON_LIGHT_ENABLED, v, i, c.neon[i] ? 1 : 0);
    Invoke(N::SET_VEHICLE_NEON_LIGHTS_COLOUR, v, c.neonRgb[0], c.neonRgb[1], c.neonRgb[2]);
    Invoke(N::SET_VEHICLE_TYRE_SMOKE_COLOR, v, c.smokeRgb[0], c.smokeRgb[1], c.smokeRgb[2]);
    if (!c.plate.empty()) Invoke(N::SET_VEHICLE_NUMBER_PLATE_TEXT, v, c.plate.c_str());
    if (c.plateStyle >= 0) Invoke(N::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX, v, c.plateStyle);
    Invoke(N::SET_VEHICLE_TYRES_CAN_BURST, v, c.bulletproofTyres ? 0 : 1);
}

// ---- The called car: only one is out at a time ----

Vehicle g_called = 0;     // 0 = none
int g_calledIndex = -1;   // its garage entry; -1 once that entry is deleted
Blip g_blip = 0;

bool IsCalledCar(Vehicle v) { return g_called != 0 && v == g_called; }

// Keeps g_calledIndex pointing at the same entry after an erase.
void OnGarageEntryDeleted(int index) {
    if (g_calledIndex == index) g_calledIndex = -1;
    else if (g_calledIndex > index) --g_calledIndex;
}

// ---- SPESP_garage.ini ----

std::string IntList(const int* v, int n) {
    std::string s;
    for (int i = 0; i < n; ++i) {
        if (i) s += ',';
        s += std::to_string(v[i]);
    }
    return s;
}

std::string BoolList(const bool* v, int n) {
    int tmp[8];
    for (int i = 0; i < n; ++i) tmp[i] = v[i] ? 1 : 0;
    return IntList(tmp, n);
}

// Parses up to n comma-separated ints into v; entries that are missing keep their value.
void ParseInts(const std::string& s, int* v, int n) {
    const char* p = s.c_str();
    for (int i = 0; i < n && *p; ++i) {
        char* end = nullptr;
        long x = std::strtol(p, &end, 10);
        if (end != p) v[i] = static_cast<int>(x);
        p = end;
        while (*p && *p != ',') ++p;
        if (*p == ',') ++p;
    }
}

void ParseBools(const std::string& s, bool* v, int n) {
    int tmp[8];
    for (int i = 0; i < n; ++i) tmp[i] = v[i] ? 1 : 0;
    ParseInts(s, tmp, n);
    for (int i = 0; i < n; ++i) v[i] = tmp[i] != 0;
}

std::string ReadKey(const std::string& path, const char* sec, const char* key) {
    char buf[512];
    GetPrivateProfileStringA(sec, key, "", buf, sizeof buf, path.c_str());
    std::string s = buf;
    size_t b = s.find_first_not_of(" \t"), e = s.find_last_not_of(" \t");
    return b == std::string::npos ? std::string() : s.substr(b, e - b + 1);
}

// Writes the whole file at once (one WritePrivateProfileString per key would rewrite it ~1000 times),
// to a temp file first so a crash mid-write can't truncate the garage. Returns false on failure.
bool WriteGarage() {
    std::string out = "; SPESP garage. Edit while the game is closed, or reload settings in the menu right after editing.\n";
    char buf[64];
    for (size_t i = 0; i < g_cars.size(); ++i) {
        const SavedCar& c = g_cars[i];
        auto line = [&out](const char* k, const std::string& v) { out += k; out += '='; out += v; out += '\n'; };
        auto num = [&line](const char* k, int v) { line(k, std::to_string(v)); };
        auto flag = [&line](const char* k, bool v) { line(k, v ? "1" : "0"); };

        out += "\n[Car" + std::to_string(i + 1) + "]\n";
        line("Name", c.name);
        std::snprintf(buf, sizeof buf, "0x%08X", static_cast<unsigned>(c.model));
        line("Model", buf);
        line("Mods", IntList(c.mods, 50));
        line("ModVariation", BoolList(c.modVariation, 2));
        flag("Turbo", c.turbo);
        flag("TyreSmoke", c.tyreSmoke);
        flag("Xenon", c.xenon);
        num("WheelType", c.wheelType);
        int colours[4] = { c.primary, c.secondary, c.pearl, c.wheelColour };
        line("Colours", IntList(colours, 4));
        int paint1[3] = { c.paint1Type, c.paint1Colour, c.paint1Pearl }, paint2[2] = { c.paint2Type, c.paint2Colour };
        line("Paint1", IntList(paint1, 3));
        line("Paint2", IntList(paint2, 2));
        line("CustomPrimary", c.customPrimary ? IntList(c.customPrimaryRgb, 3) : "");
        line("CustomSecondary", c.customSecondary ? IntList(c.customSecondaryRgb, 3) : "");
        num("ColourCombination", c.colourCombination);
        line("Extras", IntList(c.extras, 15));
        num("Livery", c.livery);
        num("WindowTint", c.windowTint);
        line("Neon", BoolList(c.neon, 4));
        line("NeonColour", IntList(c.neonRgb, 3));
        line("SmokeColour", IntList(c.smokeRgb, 3));
        line("Plate", c.plate);
        num("PlateStyle", c.plateStyle);
        flag("BulletproofTyres", c.bulletproofTyres);
        line("Photo", c.photo);
        flag("Favorite", c.favorite);
    }
    std::string path = GaragePath(), tmp = path + ".tmp";
    FILE* f = std::fopen(tmp.c_str(), "wb");
    if (!f) return false;
    bool ok = std::fwrite(out.data(), 1, out.size(), f) == out.size();
    ok = std::fclose(f) == 0 && ok;
    if (ok) ok = MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
    if (!ok) DeleteFileA(tmp.c_str());
    return ok;
}

SavedCar ReadCar(const std::string& path, const char* sec) {
    SavedCar c;
    auto get = [&](const char* k) { return ReadKey(path, sec, k); };
    auto num = [&](const char* k, int& v) { std::string s = get(k); if (!s.empty()) v = std::atoi(s.c_str()); };
    auto flag = [&](const char* k, bool& v) { std::string s = get(k); if (!s.empty()) v = s != "0"; };

    c.model = static_cast<Hash>(std::strtoul(get("Model").c_str(), nullptr, 0));
    c.name = get("Name");
    ParseInts(get("Mods"), c.mods, 50);
    for (int slot = 17; slot <= 22; ++slot) c.mods[slot] = -1;
    ParseBools(get("ModVariation"), c.modVariation, 2);
    flag("Turbo", c.turbo);
    flag("TyreSmoke", c.tyreSmoke);
    flag("Xenon", c.xenon);
    num("WheelType", c.wheelType);
    int colours[4] = { c.primary, c.secondary, c.pearl, c.wheelColour };
    ParseInts(get("Colours"), colours, 4);
    c.primary = colours[0]; c.secondary = colours[1]; c.pearl = colours[2]; c.wheelColour = colours[3];
    int paint1[3] = { c.paint1Type, c.paint1Colour, c.paint1Pearl }, paint2[2] = { c.paint2Type, c.paint2Colour };
    ParseInts(get("Paint1"), paint1, 3);
    ParseInts(get("Paint2"), paint2, 2);
    c.paint1Type = paint1[0]; c.paint1Colour = paint1[1]; c.paint1Pearl = paint1[2];
    c.paint2Type = paint2[0]; c.paint2Colour = paint2[1];
    std::string cp = get("CustomPrimary"), cs = get("CustomSecondary");
    c.customPrimary = !cp.empty();
    c.customSecondary = !cs.empty();
    ParseInts(cp, c.customPrimaryRgb, 3);
    ParseInts(cs, c.customSecondaryRgb, 3);
    num("ColourCombination", c.colourCombination);
    ParseInts(get("Extras"), c.extras, 15);
    num("Livery", c.livery);
    num("WindowTint", c.windowTint);
    ParseBools(get("Neon"), c.neon, 4);
    ParseInts(get("NeonColour"), c.neonRgb, 3);
    ParseInts(get("SmokeColour"), c.smokeRgb, 3);
    c.plate = get("Plate");
    if (c.plate.size() > kPlateMax) c.plate.resize(kPlateMax);
    num("PlateStyle", c.plateStyle);
    flag("BulletproofTyres", c.bulletproofTyres);
    c.photo = get("Photo");
    // Only a plain file name inside SPESP_garage\ (deleting the car deletes it).
    if (c.photo.find_first_of("\\/:") != std::string::npos || c.photo.find("..") != std::string::npos)
        c.photo.clear();
    flag("Favorite", c.favorite);

    c.valid = c.model != 0 && Invoke<int>(N::IS_MODEL_IN_CDIMAGE, c.model) && Invoke<int>(N::IS_MODEL_A_VEHICLE, c.model);
    if (c.name.empty()) {
        char buf[32];
        std::snprintf(buf, sizeof buf, "0x%08X", static_cast<unsigned>(c.model));
        c.name = c.valid ? CarName(c.model) : buf;
    }
    return c;
}

} // namespace

const std::vector<SavedCar>& GarageCars() { return g_cars; }
int GarageCalledIndex() { return g_calledIndex; }

void GarageLoad() {
    std::string path = GaragePath();
    g_cars.clear();
    g_calledIndex = -1; // the file may have been edited, so entry numbers can't be trusted
    std::vector<char> names(16 * 1024);
    DWORD len = GetPrivateProfileSectionNamesA(names.data(), static_cast<DWORD>(names.size()), path.c_str());
    // Section names come back as "Car1\0Car2\0\0", in file order.
    for (const char* p = names.data(); p < names.data() + len && *p; p += std::strlen(p) + 1) {
        if (_strnicmp(p, "Car", 3) != 0) continue;
        if (g_cars.size() >= kGarageMax) break;
        g_cars.push_back(ReadCar(path, p));
    }
}

int GarageOverwriteTarget() {
    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    if (g_calledIndex < 0 || g_calledIndex >= static_cast<int>(g_cars.size())) return -1;
    if (!Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0)) return -1;
    return IsCalledCar(Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_IN, player, 0)) ? g_calledIndex : -1;
}

bool GarageSaveCurrent(bool overwrite) {
    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    if (!Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0)) {
        Notify("Get in a car to save it");
        return false;
    }
    Vehicle v = Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_IN, player, 0);
    if (!IsRoadVehicle(Invoke<Hash>(N::GET_ENTITY_MODEL, v))) {
        Notify("Only road vehicles can be saved");
        return false;
    }

    int called = GarageOverwriteTarget();
    bool update = overwrite && called >= 0;
    if (!update && static_cast<int>(g_cars.size()) >= kGarageMax) {
        Notify("Garage is full (40 cars)");
        return false;
    }

    SavedCar car = Capture(v);
    char file[40];
    std::snprintf(file, sizeof file, "car_%llX.png", static_cast<unsigned long long>(GetTickCount64()));
    bool photo = TakeCarPhoto(v, file);
    if (photo) car.photo = file;

    char msg[160];
    std::string staleFile; // old photo, deleted once the garage file no longer names it
    if (update) {
        // A new photo replaces the old one; if it failed, the old one stays.
        const std::string old = g_cars[called].photo;
        if (photo) staleFile = old;
        else       car.photo = old;
        car.favorite = g_cars[called].favorite;
        g_cars[called] = car;
        std::snprintf(msg, sizeof msg, "Updated %s%s", car.name.c_str(), photo ? "" : " (no photo)");
    } else {
        g_cars.push_back(car);
        // Saved as a copy of the called car: the car you're in is now this entry.
        if (called >= 0) g_calledIndex = static_cast<int>(g_cars.size()) - 1;
        std::snprintf(msg, sizeof msg, "Saved %s%s", car.name.c_str(), photo ? "" : " (no photo)");
    }
    if (!WriteGarage()) {
        Notify("Couldn't write SPESP_garage.ini; the car is only kept until the game closes");
        return true;
    }
    if (!staleFile.empty()) DeleteFileA((PhotoDir() + staleFile).c_str());
    Notify(msg);
    return true;
}

void GarageDelete(int index) {
    if (index < 0 || index >= static_cast<int>(g_cars.size())) return;
    const std::string photo = g_cars[index].photo;
    g_cars.erase(g_cars.begin() + index);
    OnGarageEntryDeleted(index);
    if (!WriteGarage()) {
        Notify("Couldn't write SPESP_garage.ini");
        return; // keep the photo: the file on disk still names it
    }
    if (!photo.empty()) DeleteFileA((PhotoDir() + photo).c_str());
}

// ---- Calling a car ----

namespace {

int g_blipShown = -1; // last SET_BLIP_DISPLAY state, so it's only set on change

float Dist2D(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

enum class Spot { Road, Beside, None };

// Dry ground within 3 m of the player's height at this x/y (not off a roof edge, not in a hill).
bool GroundNear(float x, float y, float playerZ, float& z) {
    float groundZ = 0, waterZ = 0;
    if (!Invoke<int>(N::GET_GROUND_Z_FOR_3D_COORD, x, y, playerZ + 50.0f, &groundZ, 0)) return false;
    if (std::fabs(groundZ - playerZ) > 3.0f) return false;
    if (Invoke<int>(N::GET_WATER_HEIGHT, x, y, playerZ + 50.0f, &waterZ) && waterZ > groundZ) return false;
    z = groundZ + 0.5f;
    return true;
}

// A free road spot 30-60 m away, preferably out of view. Falls back to any free road spot
// 10-120 m away, then to dry ground beside the player. None when there's nowhere sensible
// (e.g. you're swimming).
Spot FindSpawnSpot(Ped player, const Vec3& me, Vec3& pos, float& heading) {
    if (Invoke<int>(N::IS_ENTITY_IN_WATER, player)) return Spot::None;

    struct Node { Vec3 pos; float heading; float dist; bool visible; };
    std::vector<Node> nodes;
    for (int n = 1; n <= 60; ++n) {
        Vec3 p{};
        float h = 0;
        int lanes = 0;
        // Node flag 0: only switched-on roads, so not closed areas like the airport or the base.
        if (!Invoke<int>(N::GET_NTH_CLOSEST_VEHICLE_NODE_WITH_HEADING, me.x, me.y, me.z, n, &p, &h, &lanes, 0, 3.0f, 0))
            continue;
        if (Invoke<int>(N::IS_ANY_VEHICLE_NEAR_POINT, p.x, p.y, p.z, 3.0f)) continue; // don't land on traffic
        p.z += 0.5f; // a little above the road, so the car isn't spawned into it
        nodes.push_back({ p, h, Dist2D(p, me), Invoke<int>(N::IS_SPHERE_VISIBLE, p.x, p.y, p.z, 3.0f) != 0 });
    }
    auto pick = [&](float lo, float hi, bool hiddenOnly) {
        for (const Node& nd : nodes)
            if (nd.dist >= lo && nd.dist <= hi && (!hiddenOnly || !nd.visible)) {
                pos = nd.pos;
                heading = nd.heading;
                return true;
            }
        return false;
    };
    if (pick(30, 60, true) || pick(30, 60, false) || pick(10, 120, false)) return Spot::Road;

    // Beside the player: in front, to either side, behind.
    static const float kOffsets[][2] = { { 0, 6 }, { 4, 0 }, { -4, 0 }, { 0, -6 } };
    for (const auto& o : kOffsets) {
        Vec3 p = Invoke<Vec3>(N::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS, player, o[0], o[1], 0.0f);
        float z = 0;
        if (!GroundNear(p.x, p.y, me.z, z) || Invoke<int>(N::IS_ANY_VEHICLE_NEAR_POINT, p.x, p.y, z, 2.5f)) continue;
        pos = p;
        pos.z = z;
        heading = Invoke<float>(N::GET_ENTITY_HEADING, player);
        return Spot::Beside;
    }
    return Spot::None;
}

void RemoveBlip() {
    if (g_blip && Invoke<int>(N::DOES_BLIP_EXIST, g_blip)) Invoke(N::REMOVE_BLIP, &g_blip);
    g_blip = 0;
    g_blipShown = -1;
}

// Hands the called car back to the game, which keeps or despawns it like any other car.
void ReleaseCalled() {
    RemoveBlip();
    if (g_called && Invoke<int>(N::DOES_ENTITY_EXIST, g_called))
        Invoke(N::SET_VEHICLE_AS_NO_LONGER_NEEDED, &g_called);
    g_called = 0;
    g_calledIndex = -1;
}

// The previous car goes away if nobody would notice: it's empty (no player, companion or thief
// inside) and off screen. Otherwise it's handed back to the game.
void RetirePrevious(Ped player) {
    if (g_called && Invoke<int>(N::DOES_ENTITY_EXIST, g_called) &&
        !Invoke<int>(N::IS_PED_IN_VEHICLE, player, g_called, 0) &&
        Invoke<int>(N::IS_VEHICLE_SEAT_FREE, g_called, -1) &&
        Invoke<int>(N::GET_VEHICLE_NUMBER_OF_PASSENGERS, g_called) == 0 &&
        !Invoke<int>(N::IS_ENTITY_ON_SCREEN, g_called)) {
        RemoveBlip();
        Invoke(N::SET_ENTITY_AS_MISSION_ENTITY, g_called, 1, 1);
        Invoke(N::DELETE_VEHICLE, &g_called);
        g_called = 0;
        g_calledIndex = -1;
        return;
    }
    ReleaseCalled();
}

void AddCarBlip(Vehicle v, const std::string& name) {
    g_blip = Invoke<Blip>(N::ADD_BLIP_FOR_ENTITY, v);
    Invoke(N::SET_BLIP_SPRITE, g_blip, g.garageBlipSprite);
    Invoke(N::SET_BLIP_COLOUR, g_blip, g.garageBlipColour);
    Invoke(N::BEGIN_TEXT_COMMAND_SET_BLIP_NAME, "STRING");
    Invoke(N::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, name.c_str());
    Invoke(N::END_TEXT_COMMAND_SET_BLIP_NAME, g_blip);
    g_blipShown = -1;
}

} // namespace

void GarageCall(int index) {
    if (index < 0 || index >= static_cast<int>(g_cars.size())) return;
    const SavedCar car = g_cars[index];
    char msg[128];
    std::snprintf(msg, sizeof msg, "Can't load %s", car.name.c_str());

    if (car.model == 0 || !Invoke<int>(N::IS_MODEL_IN_CDIMAGE, car.model) || !Invoke<int>(N::IS_MODEL_A_VEHICLE, car.model)) {
        Notify(msg);
        return;
    }
    Invoke(N::REQUEST_MODEL, car.model);
    int deadline = Invoke<int>(N::GET_GAME_TIMER) + 5000;
    while (!Invoke<int>(N::HAS_MODEL_LOADED, car.model)) {
        if (Invoke<int>(N::GET_GAME_TIMER) > deadline) {
            Invoke(N::SET_MODEL_AS_NO_LONGER_NEEDED, car.model);
            Notify(msg);
            return;
        }
        scriptWait(0);
    }

    // Everything that can fail happens before the previous car goes away, so a failed call keeps it.
    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    Vec3 me = Invoke<Vec3>(N::GET_ENTITY_COORDS, player, 1);
    Vec3 pos{};
    float heading = 0;
    Spot spot = FindSpawnSpot(player, me, pos, heading);
    if (spot == Spot::None) {
        Invoke(N::SET_MODEL_AS_NO_LONGER_NEEDED, car.model);
        Notify("No room for the car here");
        return;
    }

    // The 8th argument (newer builds read one) is pushed as 0 so it's never stale.
    Vehicle v = Invoke<Vehicle>(N::CREATE_VEHICLE, car.model, pos.x, pos.y, pos.z, heading, 0, 1, 0);
    Invoke(N::SET_MODEL_AS_NO_LONGER_NEEDED, car.model);
    if (!v) {
        Notify(msg);
        return;
    }
    RetirePrevious(player);
    Invoke<int>(N::SET_VEHICLE_ON_GROUND_PROPERLY, v);
    ApplySavedCar(v, car);
    Invoke(N::SET_VEHICLE_DIRT_LEVEL, v, 0.0f);
    Invoke(N::SET_VEHICLE_DOORS_LOCKED, v, 1);
    Invoke(N::SET_VEHICLE_ENGINE_ON, v, 0, 1, 0); // last arg 0: the engine still starts when you get in
    Invoke(N::SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER, v, 1);
    Invoke(N::SET_VEHICLE_NEEDS_TO_BE_HOTWIRED, v, 0);
    Invoke(N::SET_VEHICLE_IS_STOLEN, v, 0);

    g_called = v;
    g_calledIndex = index;
    AddCarBlip(v, car.name);

    std::snprintf(msg, sizeof msg, spot == Spot::Road ? "%s is waiting nearby" : "%s is next to you", car.name.c_str());
    Notify(msg);
}

void GarageTick(Ped player) {
    if (!g_called) return;
    if (!Invoke<int>(N::DOES_ENTITY_EXIST, g_called)) {
        RemoveBlip();
        g_called = 0;
        g_calledIndex = -1;
        return;
    }
    if (Invoke<int>(N::IS_ENTITY_DEAD, g_called) || !Invoke<int>(N::IS_VEHICLE_DRIVEABLE, g_called, 0)) {
        // The garage entry may have been deleted since, so name it from the model.
        std::string name = CarName(Invoke<Hash>(N::GET_ENTITY_MODEL, g_called));
        char msg[128];
        std::snprintf(msg, sizeof msg, "%s was destroyed", name.c_str());
        Notify(msg);
        ReleaseCalled();
        return;
    }
    int shown = Invoke<int>(N::IS_PED_IN_VEHICLE, player, g_called, 0) ? 0 : 2;
    if (g_blip && shown != g_blipShown) {
        Invoke(N::SET_BLIP_DISPLAY, g_blip, shown);
        g_blipShown = shown;
    }
}

// ---- Favourites and plates ----

std::vector<int> GarageDisplayOrder() {
    std::vector<int> order;
    for (int pass = 0; pass < 2; ++pass)
        for (int i = 0; i < static_cast<int>(g_cars.size()); ++i)
            if (g_cars[i].favorite == (pass == 0)) order.push_back(i);
    return order;
}

void GarageToggleFavorite(int index) {
    if (index < 0 || index >= static_cast<int>(g_cars.size())) return;
    g_cars[index].favorite = !g_cars[index].favorite;
    if (!WriteGarage()) Notify("Couldn't write SPESP_garage.ini");
}

bool GarageSetPlate(int index, const std::string& text) {
    if (index < 0 || index >= static_cast<int>(g_cars.size())) return false;
    std::string plate;
    for (char ch : text) {
        if (static_cast<unsigned char>(ch) < 32) continue;
        plate += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    size_t b = plate.find_first_not_of(' '), e = plate.find_last_not_of(' ');
    plate = b == std::string::npos ? std::string() : plate.substr(b, e - b + 1);
    if (plate.size() > kPlateMax) plate.resize(kPlateMax);
    if (plate.empty()) return false;

    g_cars[index].plate = plate;
    if (index == g_calledIndex && g_called && Invoke<int>(N::DOES_ENTITY_EXIST, g_called))
        Invoke(N::SET_VEHICLE_NUMBER_PLATE_TEXT, g_called, plate.c_str());
    if (!WriteGarage()) Notify("Couldn't write SPESP_garage.ini");
    return true;
}
