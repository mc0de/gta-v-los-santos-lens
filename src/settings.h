#pragma once
// User settings, persisted to SPESP.ini next to the .asi.

#include <windows.h>
#include <string>
#include <vector>

enum class Targets { Peds, Vehicles, Both };
enum class AimBone { Head, Body };
enum class AimPriority { Nearest, HostilesFirst, HostilesOnly };

struct Keys {
    int menu          = VK_DELETE; // Home opens the Rockstar launcher overlay
    int toggleEsp     = VK_F9;
    int cycleTargets  = VK_F10;
    int toggleTracers = VK_F11;
    int toggleAim     = VK_PRIOR; // Page Up
    int aimBone       = VK_NEXT;  // Page Down
    int teleport      = VK_END;
};

struct Collectible {
    std::string model;
    std::string label;
};

struct Settings {
    Keys keys;

    // ESP
    bool esp = true;
    Targets targets = Targets::Both;
    bool boxes = true;
    bool skeletons = false;
    bool healthBars = true;
    bool labels = true;
    bool weaponLabels = true;
    bool tracers = false;
    bool distanceFade = true;
    bool threatArrows = true;
    bool espIgnoreAnimals = true;
    float pedRange = 250.0f;
    float vehRange = 350.0f;

    // Finder
    bool collectibles = false;
    bool pickups = false;
    float finderRange = 300.0f;
    std::vector<Collectible> collectibleModels;

    // Aim
    bool aim = false;
    AimBone aimBone = AimBone::Head;
    AimPriority aimPriority = AimPriority::Nearest;
    bool aimIgnoreCops = false;
    bool aimIgnoreAnimals = true;
    int aimSmoothing = 1;   // 1 = snap, higher = slower glide
    int aimFov = 180;       // max degrees from the crosshair; 180 = anywhere
    bool showFov = false;

    // Misc
    bool damageNumbers = true;
    float statusX = 0.06f;
    float statusY = 0.002f;
};

extern Settings g;

// Reads SPESP.ini from the plugin's folder, creating it with defaults if missing.
void LoadSettings(HMODULE module);
void ReloadSettings();
void SaveSettings();
