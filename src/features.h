#pragma once

#include "ui.h"
#include "settings.h"
#include <string>
#include <vector>

constexpr int kBoneHead = 31086; // SKEL_Head

// Ped currently locked by auto-aim (0 = none); ESP highlights it.
extern Ped g_aimTarget;

// All peds the game has loaded, fetched once per frame by BeginFrame and shared by every feature.
extern int g_peds[512];
extern int g_pedCount;
// Refreshes the ped list and starts a new frame for the IsThreat / HasLineOfSight caches.
void BeginFrame();

// Both are cached (threat per frame, line of sight for ~100 ms) since ESP and auto-aim ask about
// the same peds.
bool IsThreat(Ped p, Ped player);
// Clear line of sight from the player (walls, vehicles and objects block it).
bool HasLineOfSight(Ped player, Ped p);
bool IsLawEnforcement(Ped p);
// Name of the weapon whose world model this is (e.g. a weapon pickup), or nullptr.
const char* WeaponLabelForModel(Hash model);
// Vehicle model name ("Panto"), falling back to the model code when the game has no text for it.
const char* VehicleModelName(Hash model);

// In-game make and real-life brand of a vehicle model (world.cpp's table), or nullptr.
struct VehicleMake;
const VehicleMake* FindVehicleMake(Hash model);
const char* VehicleMakeName(Hash model); // "" when unknown

// esp.cpp
void EspTick(Ped player, const Vec3& me);

// aim.cpp
void AimTick(Ped player, const Vec3& me);

// world.cpp
void FinderTick(const Vec3& me);
void LettersTick(const Vec3& me);
void DamageNumbersTick(Ped player);
void SpeedometerTick(Ped player);
void TeleportToWaypoint();
void SkipTime();

// garage.cpp: saved cars, stored in SPESP_garage.ini.
// Everything needed to rebuild a car as it was when saved (damage and dirt aren't kept).
struct SavedCar {
    std::string name;            // "Make Model" at save time
    Hash model = 0;
    int mods[50];                // -1 = stock; 17-22 are toggle slots and stay -1
    bool modVariation[2] = {};   // custom tyres on front (23) / rear (24) wheels
    bool turbo = false, tyreSmoke = false, xenon = false;
    int wheelType = -1;          // -1 = leave as spawned
    int primary = -1, secondary = -1, pearl = -1, wheelColour = -1;
    int paint1Type = -1, paint1Colour = -1, paint1Pearl = -1, paint2Type = -1, paint2Colour = -1;
    bool customPrimary = false, customSecondary = false;
    int customPrimaryRgb[3] = {}, customSecondaryRgb[3] = {};
    int colourCombination = -1;
    int extras[15];              // -1 = doesn't exist, 0 = off, 1 = on
    int livery = -1;
    int windowTint = -1;
    bool neon[4] = {};           // left, right, front, back
    int neonRgb[3] = { 255, 255, 255 };
    int smokeRgb[3] = { 255, 255, 255 };
    std::string plate;           // empty = leave as spawned
    int plateStyle = -1;
    bool bulletproofTyres = false;
    bool valid = true;           // false when the game doesn't know the model (not saved to the file)

    SavedCar() { for (int& m : mods) m = -1; for (int& e : extras) e = -1; }
};

constexpr int kGarageMax = 40;
const std::vector<SavedCar>& GarageCars();
void GarageLoad();                // reads SPESP_garage.ini
bool GarageSaveCurrent();         // saves (or updates) the car the player is in; notifies
void GarageDelete(int index);
void GarageCall(int index);       // waits for the model to stream in: call from the deferred path only
void GarageTick(Ped player);
int GarageCalledIndex();          // garage entry of the car that's out, or -1
