#pragma once

#include "ui.h"
#include "settings.h"

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
