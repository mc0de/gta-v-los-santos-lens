#pragma once

#include "ui.h"
#include "settings.h"

// Ped currently locked by auto-aim (0 = none); ESP highlights it.
extern Ped g_aimTarget;

bool IsThreat(Ped p, Ped player);
bool IsLawEnforcement(Ped p);

// esp.cpp
void EspTick(Ped player, const Vec3& me);

// aim.cpp
void AimTick(Ped player, const Vec3& me);

// world.cpp
void FinderTick(const Vec3& me);
void DamageNumbersTick(Ped player);
void TeleportToWaypoint();
