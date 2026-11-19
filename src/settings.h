#pragma once
// User settings, persisted to SPESP.ini next to the .asi.

#include <windows.h>
#include <array>
#include <string>
#include <vector>

enum class AimBone { Head, Body };
enum class AimPick { Crosshair, Nearest };

struct Keys {
    int menu             = VK_DELETE; // Home opens the Rockstar launcher overlay
    int toggleEsp        = VK_F9;
    int togglePedestrians = VK_F10;
    int toggleVehicles   = VK_F11;
    int toggleAim        = VK_PRIOR; // Page Up
    int aimBone          = VK_NEXT;  // Page Down
    int teleport         = VK_END;
    int skipTime         = VK_INSERT;
};

// What the ESP draws for one category of target.
struct EspStyle {
    bool enabled;
    bool box;
    bool health;
    bool distance;
    bool labels;   // peds: COP/SWAT/ARMY and weapon held; vehicles: name and tags
    bool tracers;
};

// Vehicle class names by GET_VEHICLE_CLASS id (0-22): the vehicle ESP filter's checklist,
// the speedometer's class line and the names used by VehicleTypes in the ini.
constexpr int kVehicleClassCount = 23;
extern const char* const kVehicleClasses[kVehicleClassCount];

using VehicleClassSet = std::array<bool, kVehicleClassCount>;
inline VehicleClassSet AllVehicleClasses() { VehicleClassSet s; s.fill(true); return s; }

// "All", "None", the one class shown, or "N types".
const char* VehicleTypesSummary();

struct Collectible {
    std::string model;
    std::string label;
};

struct Settings {
    Keys keys;

    // ESP
    bool esp = true;
    bool showDead = false;
    bool distanceFade = true;
    bool threatArrows = true;
    float pedRange = 250.0f;
    float vehRange = 350.0f;
    VehicleClassSet vehTypes = AllVehicleClasses(); // vehicle classes the ESP shows

    // Peds are sorted into one category each, checked in this order:
    // animal, hostile (fighting you / hates you), cop (cop/SWAT/army), pedestrian (everyone else).
    //                         on     box    health distance labels tracers
    EspStyle hostiles    {    true,  true,  true,  true,    true,  false };
    EspStyle animals     {    false, true,  false, true,    false, false };
    EspStyle cops        {    true,  true,  true,  true,    true,  false };
    EspStyle pedestrians {    true,  true,  false, true,    false, false };
    EspStyle vehicles    {    true,  true,  false, true,    true,  false };

    // Finder
    bool collectibles = false;
    bool pickups = false;
    float finderRange = 300.0f;
    std::vector<Collectible> collectibleModels;
    // Names for non-weapon pickups (health, armour, ...); weapon pickups are named automatically.
    std::vector<Collectible> pickupModels;

    // Letter scraps: show the nearest few of those the game has loaded, ignoring finderRange.
    bool letters = false;
    int letterCount = 3;

    // Aim
    bool aim = false;
    AimBone aimBone = AimBone::Head;
    AimPick aimPick = AimPick::Crosshair;
    // Hostiles (anyone fighting you, cops and animals included) are always targets;
    // these add the peaceful members of each category.
    bool aimCops = false;
    bool aimPedestrians = false;
    bool aimAnimals = false;
    float aimRange = 250.0f;
    int aimFov = 180;       // max degrees from the crosshair; 180 = anywhere
    int aimSmoothing = 1;   // 1 = snap, higher = slower glide

    // Misc
    bool damageNumbers = true;
    int skipHours = 6;      // hours the skip-time hotkey moves the clock forward
    bool speedometer = true; // km/h readout while in a vehicle
    float speedoX = 0.86f;  // bottom-right corner of the speedometer box
    float speedoY = 0.985f;
    float statusX = 0.06f;
    float statusY = 0.002f;

    // Garage blip (ini only). Best-known ids, not verified: 225 = car icon, 27 = purple.
    int garageBlipSprite = 225;
    int garageBlipColour = 27;
};

extern Settings g;

// Reads SPESP.ini from the plugin's folder, creating it with defaults if missing.
void LoadSettings(HMODULE module);
void ReloadSettings();
void SaveSettings();
// Full path of SPESP.ini.
const std::string& IniPath();
