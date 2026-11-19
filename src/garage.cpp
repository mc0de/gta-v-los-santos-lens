// Garage: save the car you're in (with all its customisation) and call it back later.
// Saved cars live in SPESP_garage.ini next to the plugin. The only world writes here are to the
// car we spawn ourselves.

#include "features.h"
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

// Writes the whole file at once (one WritePrivateProfileString per key would rewrite it ~1000 times).
void WriteGarage() {
    std::string out = "; SPESP garage. Edit only while the game is closed.\n";
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
    }
    if (FILE* f = std::fopen(GaragePath().c_str(), "wb")) {
        std::fwrite(out.data(), 1, out.size(), f);
        std::fclose(f);
    }
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

    bool valid = c.model != 0 && Invoke<int>(N::IS_MODEL_IN_CDIMAGE, c.model) && Invoke<int>(N::IS_MODEL_A_VEHICLE, c.model);
    if (!valid) c.name = "(invalid) " + c.name;
    else if (c.name.empty()) c.name = CarName(c.model);
    return c;
}

} // namespace

const std::vector<SavedCar>& GarageCars() { return g_cars; }
int GarageCalledIndex() { return g_calledIndex; }

void GarageLoad() {
    std::string path = GaragePath();
    g_cars.clear();
    std::vector<char> names(16 * 1024);
    DWORD len = GetPrivateProfileSectionNamesA(names.data(), static_cast<DWORD>(names.size()), path.c_str());
    // Section names come back as "Car1\0Car2\0\0", in file order.
    for (const char* p = names.data(); p < names.data() + len && *p; p += std::strlen(p) + 1) {
        if (_strnicmp(p, "Car", 3) != 0) continue;
        if (g_cars.size() >= kGarageMax) break;
        g_cars.push_back(ReadCar(path, p));
    }
}

bool GarageSaveCurrent() {
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

    SavedCar car = Capture(v);
    char msg[128];
    int called = GarageCalledIndex();
    if (called >= 0 && called < static_cast<int>(g_cars.size()) && IsCalledCar(v)) {
        g_cars[called] = car;
        std::snprintf(msg, sizeof msg, "Updated %s", car.name.c_str());
    } else {
        if (static_cast<int>(g_cars.size()) >= kGarageMax) {
            Notify("Garage is full (40 cars)");
            return false;
        }
        g_cars.push_back(car);
        std::snprintf(msg, sizeof msg, "Saved %s", car.name.c_str());
    }
    WriteGarage();
    Notify(msg);
    return true;
}

void GarageDelete(int index) {
    if (index < 0 || index >= static_cast<int>(g_cars.size())) return;
    g_cars.erase(g_cars.begin() + index);
    OnGarageEntryDeleted(index);
    WriteGarage();
}
