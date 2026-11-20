#include "settings.h"
#include "features.h" // GarageLoad
#include <cstdio>
#include <cstdlib>
#include <cstring>

Settings g;

const char* const kVehicleClasses[kVehicleClassCount] = {
    "Compacts", "Sedans", "SUVs", "Coupes", "Muscle", "Sports Classics", "Sports", "Super", "Motorcycles",
    "Off-road", "Industrial", "Utility", "Vans", "Cycles", "Boats", "Helicopters", "Planes", "Service",
    "Emergency", "Military", "Commercial", "Trains", "Open Wheel",
};

const char* VehicleTypesSummary() {
    int count = 0, last = 0;
    for (int i = 0; i < kVehicleClassCount; ++i)
        if (g.vehTypes[i]) { ++count; last = i; }
    if (count == kVehicleClassCount) return "All";
    if (count == 0) return "None";
    if (count == 1) return kVehicleClasses[last];
    static char buf[16];
    std::snprintf(buf, sizeof buf, "%d types", count);
    return buf;
}

namespace {

std::string g_path;

// Defaults for the collectible finder. Model names are best-known values and may need adjusting
// in the [Collectibles] section of the ini.
const Collectible kDefaultCollectibles[] = {
    { "prop_power_cell",          "Spaceship part" },
    { "prop_peyote_highland_01",  "Peyote" },
    { "prop_peyote_highland_02",  "Peyote" },
    { "prop_peyote_lowland_01",   "Peyote" },
    { "prop_peyote_lowland_02",   "Peyote" },
    { "prop_peyote_water_01",     "Peyote" },
    { "prop_peyote_gold_01",      "Golden peyote" },
    { "prop_rad_waste_barrel_01", "Nuclear waste" },
};

// Names for non-weapon pickups, by model. Best-known values, adjustable in the [Pickups] section.
// Weapon pickups don't need entries: they're named from the weapon list.
const Collectible kDefaultPickups[] = {
    { "prop_ld_health_pack",   "Health" },
    { "prop_armour_pickup",    "Armour" },
    { "prop_ld_ammo_pack_01",  "Ammo" },
    { "prop_ld_ammo_pack_02",  "Ammo" },
    { "prop_ld_ammo_pack_03",  "Ammo" },
    { "prop_parachute",        "Parachute" },
};

struct KeyName { const char* name; int vk; };
const KeyName kKeyNames[] = {
    { "None", 0 },
    { "F1", VK_F1 }, { "F2", VK_F2 }, { "F3", VK_F3 }, { "F4", VK_F4 },
    { "F5", VK_F5 }, { "F6", VK_F6 }, { "F7", VK_F7 }, { "F8", VK_F8 },
    { "F9", VK_F9 }, { "F10", VK_F10 }, { "F11", VK_F11 }, { "F12", VK_F12 },
    { "PageUp", VK_PRIOR }, { "PageDown", VK_NEXT }, { "Home", VK_HOME }, { "End", VK_END },
    { "Insert", VK_INSERT }, { "Delete", VK_DELETE },
    { "Numpad0", VK_NUMPAD0 }, { "Numpad1", VK_NUMPAD1 }, { "Numpad2", VK_NUMPAD2 },
    { "Numpad3", VK_NUMPAD3 }, { "Numpad4", VK_NUMPAD4 }, { "Numpad5", VK_NUMPAD5 },
    { "Numpad6", VK_NUMPAD6 }, { "Numpad7", VK_NUMPAD7 }, { "Numpad8", VK_NUMPAD8 },
    { "Numpad9", VK_NUMPAD9 }, { "Multiply", VK_MULTIPLY }, { "Add", VK_ADD },
    { "Subtract", VK_SUBTRACT }, { "Divide", VK_DIVIDE },
};

const char* const kBoneNames[]     = { "head", "body" };
const char* const kPickNames[]     = { "crosshair", "nearest" };

std::string KeyToString(int vk) {
    for (const auto& k : kKeyNames)
        if (k.vk == vk) return k.name;
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
        return std::string(1, static_cast<char>(vk));
    char buf[16];
    std::snprintf(buf, sizeof buf, "0x%02X", vk);
    return buf;
}

int StringToKey(const char* s, int fallback) {
    for (const auto& k : kKeyNames)
        if (_stricmp(k.name, s) == 0) return k.vk;
    if (std::strlen(s) == 1) {
        char c = static_cast<char>(toupper(static_cast<unsigned char>(s[0])));
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) return c;
    }
    char* end = nullptr;
    long v = std::strtol(s, &end, 0);
    return (end != s && v >= 0 && v <= 255) ? static_cast<int>(v) : fallback;
}

std::string Trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t"), e = s.find_last_not_of(" \t");
    return b == std::string::npos ? std::string() : s.substr(b, e - b + 1);
}

template <typename T>
void Clamp(T& v, T lo, T hi) { v = v < lo ? lo : v > hi ? hi : v; }

// Hand-edited values are kept within the ranges the menu allows.
void ClampSettings() {
    Clamp(g.pedRange, 25.0f, 1000.0f);
    Clamp(g.vehRange, 25.0f, 1000.0f);
    Clamp(g.finderRange, 50.0f, 2000.0f);
    Clamp(g.letterCount, 1, 10);
    Clamp(g.aimRange, 25.0f, 1000.0f);
    Clamp(g.aimFov, 5, 180);
    Clamp(g.skipHours, 1, 23);
    Clamp(g.speedoX, 0.0f, 1.0f);
    Clamp(g.speedoY, 0.0f, 1.0f);
    Clamp(g.statusX, 0.0f, 1.0f);
    Clamp(g.statusY, 0.0f, 1.0f);
    Clamp(g.garageBlipSprite, 0, 1000);
    Clamp(g.garageBlipColour, 0, 85);
}

std::string Read(const char* sec, const char* key) {
    char buf[256];
    GetPrivateProfileStringA(sec, key, "", buf, sizeof buf, g_path.c_str());
    return Trim(buf);
}

void ReadBool(const char* sec, const char* key, bool& v) {
    std::string s = Read(sec, key);
    if (!s.empty()) v = s == "1" || _stricmp(s.c_str(), "true") == 0 || _stricmp(s.c_str(), "on") == 0;
}

void ReadInt(const char* sec, const char* key, int& v) {
    std::string s = Read(sec, key);
    if (!s.empty()) v = std::atoi(s.c_str());
}

void ReadFloat(const char* sec, const char* key, float& v) {
    std::string s = Read(sec, key);
    if (!s.empty()) v = static_cast<float>(std::atof(s.c_str()));
}

void ReadKey(const char* key, int& v) {
    std::string s = Read("Keys", key);
    if (!s.empty()) v = StringToKey(s.c_str(), v);
}

template <typename E, size_t N>
void ReadEnum(const char* sec, const char* key, E& v, const char* const (&names)[N]) {
    std::string s = Read(sec, key);
    for (size_t i = 0; i < N; ++i)
        if (_stricmp(s.c_str(), names[i]) == 0) v = static_cast<E>(i);
}

void Write(const char* sec, const char* key, const std::string& v) {
    WritePrivateProfileStringA(sec, key, v.c_str(), g_path.c_str());
}

void WriteBool(const char* sec, const char* key, bool v) { Write(sec, key, v ? "true" : "false"); }
void WriteInt(const char* sec, const char* key, int v) { Write(sec, key, std::to_string(v)); }

void WriteFloat(const char* sec, const char* key, float v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%g", v);
    Write(sec, key, buf);
}

void WriteKey(const char* key, int vk) { Write("Keys", key, KeyToString(vk)); }

template <typename E, size_t N>
void WriteEnum(const char* sec, const char* key, E v, const char* const (&names)[N]) {
    Write(sec, key, names[static_cast<size_t>(v)]);
}

// "All", "None", or a comma-separated list of class names ("Super, Sports"). Unknown names are skipped.
void ReadVehicleTypes(const char* sec, const char* key, VehicleClassSet& v) {
    std::string s = Read(sec, key);
    if (s.empty() || _stricmp(s.c_str(), "all") == 0) return;
    v.fill(false);
    if (_stricmp(s.c_str(), "none") == 0) return;
    for (size_t start = 0; start <= s.size();) {
        size_t comma = s.find(',', start);
        if (comma == std::string::npos) comma = s.size();
        std::string name = Trim(s.substr(start, comma - start));
        for (int i = 0; i < kVehicleClassCount; ++i)
            if (_stricmp(name.c_str(), kVehicleClasses[i]) == 0) v[i] = true;
        start = comma + 1;
    }
}

void WriteVehicleTypes(const char* sec, const char* key, const VehicleClassSet& v) {
    std::string s;
    int count = 0;
    for (int i = 0; i < kVehicleClassCount; ++i) {
        if (!v[i]) continue;
        if (count++) s += ", ";
        s += kVehicleClasses[i];
    }
    Write(sec, key, count == kVehicleClassCount ? "All" : count == 0 ? "None" : s);
}

void ReadStyle(const char* sec, EspStyle& s) {
    ReadBool(sec, "Enabled", s.enabled);
    ReadBool(sec, "Box", s.box);
    ReadBool(sec, "Health", s.health);
    ReadBool(sec, "Distance", s.distance);
    ReadBool(sec, "Labels", s.labels);
    ReadBool(sec, "Tracers", s.tracers);
}

void WriteStyle(const char* sec, const EspStyle& s) {
    WriteBool(sec, "Enabled", s.enabled);
    WriteBool(sec, "Box", s.box);
    WriteBool(sec, "Health", s.health);
    WriteBool(sec, "Distance", s.distance);
    WriteBool(sec, "Labels", s.labels);
    WriteBool(sec, "Tracers", s.tracers);
}

// Keys from earlier versions, removed on save so the ini doesn't keep dead settings around.
void DeleteObsoleteKeys() {
    static const char* const kKeys[] = { "CycleTargets", "ToggleTracers", "SkipHour" };
    static const char* const kEsp[] = {
        "Targets", "Boxes", "Skeletons", "HealthBars", "Labels", "WeaponLabels",
        "Tracers", "IgnoreAnimals", "HostilesOnly",
    };
    for (const char* k : kKeys) WritePrivateProfileStringA("Keys", k, nullptr, g_path.c_str());
    static const char* const kAim[] = { "Priority", "IgnoreCops", "IgnoreAnimals", "ShowFov", "Smoothing" };
    for (const char* k : kEsp)  WritePrivateProfileStringA("ESP", k, nullptr, g_path.c_str());
    for (const char* k : kAim)  WritePrivateProfileStringA("Aim", k, nullptr, g_path.c_str());
    static const char* const kMisc[] = { "DamageX", "DamageY" };
    for (const char* k : kMisc) WritePrivateProfileStringA("Misc", k, nullptr, g_path.c_str());
}

// Ini files from before the Pick / Target* settings: carry over Priority, IgnoreCops and
// IgnoreAnimals, and keep picking by distance as those versions did.
void MigrateOldAimKeys() {
    std::string priority = Read("Aim", "Priority");
    if (priority.empty() || !Read("Aim", "Pick").empty()) return;

    bool ignoreCops = false, ignoreAnimals = true; // old defaults
    ReadBool("Aim", "IgnoreCops", ignoreCops);
    ReadBool("Aim", "IgnoreAnimals", ignoreAnimals);
    bool hostilesOnly = _stricmp(priority.c_str(), "hostiles_only") == 0;

    g.aimPick = AimPick::Nearest;
    g.aimPedestrians = !hostilesOnly;
    g.aimCops = !hostilesOnly && !ignoreCops;
    g.aimAnimals = !hostilesOnly && !ignoreAnimals;
    g.aimRange = g.pedRange; // aim used the ESP range before it had its own
}

// Reads a "model=Label" section, falling back to the defaults if it's missing or empty.
template <size_t Count>
void ReadModelList(const char* section, std::vector<Collectible>& out, const Collectible (&defaults)[Count]) {
    std::vector<char> buf(32 * 1024);
    DWORD len = GetPrivateProfileSectionA(section, buf.data(), static_cast<DWORD>(buf.size()), g_path.c_str());
    out.clear();

    // Section data is "model=Label\0model=Label\0\0".
    for (const char* p = buf.data(); p < buf.data() + len && *p; p += std::strlen(p) + 1) {
        const char* eq = std::strchr(p, '=');
        if (!eq || *p == ';' || *p == '#') continue;
        std::string model = Trim(std::string(p, eq)), label = Trim(eq + 1);
        if (!model.empty()) out.push_back({ model, label });
    }

    if (out.empty())
        out.assign(std::begin(defaults), std::end(defaults));
}

// Rewrites a "model=Label" section wholesale ("key=value\0...\0\0").
void WriteModelList(const char* section, const std::vector<Collectible>& list) {
    std::string data;
    for (const auto& c : list) {
        data += c.model + "=" + c.label;
        data.push_back('\0');
    }
    data.push_back('\0');
    WritePrivateProfileSectionA(section, data.c_str(), g_path.c_str());
}

} // namespace

void ReloadSettings() {
    Settings d; // start from defaults so removed keys fall back cleanly
    g = d;

    ReadKey("Menu", g.keys.menu);
    ReadKey("ToggleEsp", g.keys.toggleEsp);
    ReadKey("TogglePedestrians", g.keys.togglePedestrians);
    ReadKey("ToggleVehicles", g.keys.toggleVehicles);
    ReadKey("ToggleAim", g.keys.toggleAim);
    ReadKey("AimBone", g.keys.aimBone);
    ReadKey("Teleport", g.keys.teleport);
    ReadKey("SkipHour", g.keys.skipTime); // old name
    ReadKey("SkipTime", g.keys.skipTime);
    ReadKey("Garage", g.keys.garage);
    ReadKey("AimLeft", g.keys.aimLeft);
    ReadKey("AimRight", g.keys.aimRight);

    ReadBool("ESP", "Enabled", g.esp);
    ReadBool("ESP", "ShowDead", g.showDead);
    ReadBool("ESP", "DistanceFade", g.distanceFade);
    ReadBool("ESP", "ThreatArrows", g.threatArrows);
    ReadFloat("ESP", "PedRange", g.pedRange);
    ReadFloat("ESP", "VehicleRange", g.vehRange);
    ReadVehicleTypes("ESP", "VehicleTypes", g.vehTypes);
    ReadStyle("ESP.Hostiles", g.hostiles);
    ReadStyle("ESP.Animals", g.animals);
    ReadStyle("ESP.Cops", g.cops);
    ReadStyle("ESP.Pedestrians", g.pedestrians);
    ReadStyle("ESP.Vehicles", g.vehicles);

    ReadBool("Finder", "Collectibles", g.collectibles);
    ReadBool("Finder", "Pickups", g.pickups);
    ReadFloat("Finder", "Range", g.finderRange);
    ReadBool("Finder", "Letters", g.letters);
    ReadInt("Finder", "LetterCount", g.letterCount);

    ReadBool("Aim", "Enabled", g.aim);
    ReadEnum("Aim", "Bone", g.aimBone, kBoneNames);
    MigrateOldAimKeys();
    ReadEnum("Aim", "Pick", g.aimPick, kPickNames);
    ReadBool("Aim", "TargetCops", g.aimCops);
    ReadBool("Aim", "TargetPedestrians", g.aimPedestrians);
    ReadBool("Aim", "TargetAnimals", g.aimAnimals);
    ReadFloat("Aim", "Range", g.aimRange);
    ReadInt("Aim", "Fov", g.aimFov);

    ReadBool("Misc", "DamageNumbers", g.damageNumbers);
    ReadInt("Misc", "SkipHours", g.skipHours);
    ReadBool("Misc", "Speedometer", g.speedometer);
    ReadFloat("Misc", "SpeedoX", g.speedoX);
    ReadFloat("Misc", "SpeedoY", g.speedoY);
    ReadFloat("Misc", "StatusX", g.statusX);
    ReadFloat("Misc", "StatusY", g.statusY);

    ReadInt("Garage", "BlipSprite", g.garageBlipSprite);
    ReadInt("Garage", "BlipColour", g.garageBlipColour);

    ReadModelList("Collectibles", g.collectibleModels, kDefaultCollectibles);
    ReadModelList("Pickups", g.pickupModels, kDefaultPickups);
    ClampSettings();
    GarageLoad();
    BestSpeedsLoad();
}

void SaveSettings() {
    WriteKey("Menu", g.keys.menu);
    WriteKey("ToggleEsp", g.keys.toggleEsp);
    WriteKey("TogglePedestrians", g.keys.togglePedestrians);
    WriteKey("ToggleVehicles", g.keys.toggleVehicles);
    WriteKey("ToggleAim", g.keys.toggleAim);
    WriteKey("AimBone", g.keys.aimBone);
    WriteKey("Teleport", g.keys.teleport);
    WriteKey("SkipTime", g.keys.skipTime);
    WriteKey("Garage", g.keys.garage);
    WriteKey("AimLeft", g.keys.aimLeft);
    WriteKey("AimRight", g.keys.aimRight);

    WriteBool("ESP", "Enabled", g.esp);
    WriteBool("ESP", "ShowDead", g.showDead);
    WriteBool("ESP", "DistanceFade", g.distanceFade);
    WriteBool("ESP", "ThreatArrows", g.threatArrows);
    WriteFloat("ESP", "PedRange", g.pedRange);
    WriteFloat("ESP", "VehicleRange", g.vehRange);
    WriteVehicleTypes("ESP", "VehicleTypes", g.vehTypes);
    WriteStyle("ESP.Hostiles", g.hostiles);
    WriteStyle("ESP.Animals", g.animals);
    WriteStyle("ESP.Cops", g.cops);
    WriteStyle("ESP.Pedestrians", g.pedestrians);
    WriteStyle("ESP.Vehicles", g.vehicles);
    DeleteObsoleteKeys();

    WriteBool("Finder", "Collectibles", g.collectibles);
    WriteBool("Finder", "Pickups", g.pickups);
    WriteFloat("Finder", "Range", g.finderRange);
    WriteBool("Finder", "Letters", g.letters);
    WriteInt("Finder", "LetterCount", g.letterCount);

    WriteBool("Aim", "Enabled", g.aim);
    WriteEnum("Aim", "Bone", g.aimBone, kBoneNames);
    WriteEnum("Aim", "Pick", g.aimPick, kPickNames);
    WriteBool("Aim", "TargetCops", g.aimCops);
    WriteBool("Aim", "TargetPedestrians", g.aimPedestrians);
    WriteBool("Aim", "TargetAnimals", g.aimAnimals);
    WriteFloat("Aim", "Range", g.aimRange);
    WriteInt("Aim", "Fov", g.aimFov);

    WriteBool("Misc", "DamageNumbers", g.damageNumbers);
    WriteInt("Misc", "SkipHours", g.skipHours);
    WriteBool("Misc", "Speedometer", g.speedometer);
    WriteFloat("Misc", "SpeedoX", g.speedoX);
    WriteFloat("Misc", "SpeedoY", g.speedoY);
    WriteFloat("Misc", "StatusX", g.statusX);
    WriteFloat("Misc", "StatusY", g.statusY);

    WriteInt("Garage", "BlipSprite", g.garageBlipSprite);
    WriteInt("Garage", "BlipColour", g.garageBlipColour);

    WriteModelList("Collectibles", g.collectibleModels);
    WriteModelList("Pickups", g.pickupModels);
}

const std::string& IniPath() { return g_path; }

void LoadSettings(HMODULE module) {
    char path[MAX_PATH];
    GetModuleFileNameA(module, path, MAX_PATH);
    g_path = path;
    size_t dot = g_path.find_last_of('.');
    g_path = (dot == std::string::npos ? g_path : g_path.substr(0, dot)) + ".ini";

    bool exists = GetFileAttributesA(g_path.c_str()) != INVALID_FILE_ATTRIBUTES;
    ReloadSettings();
    if (!exists)
        SaveSettings();
}
