#include "features.h"
#include <algorithm>
#include <cmath>

Ped g_aimTarget = 0;

namespace {

constexpr int kBoneHead = 31086;      // SKEL_Head
constexpr int kBoneBody = 24818;      // SKEL_Spine3 (upper chest)
constexpr float kRadToDeg = 57.2957795f;
constexpr float kDegToRad = 0.0174532925f;
constexpr float kPi = 3.14159265f;

float WrapDegrees(float a) {
    while (a > 180.0f)  a -= 360.0f;
    while (a < -180.0f) a += 360.0f;
    return a;
}

// World heading/pitch (degrees) of the direction from `from` to `to`.
// GTA headings: 0 = north (+Y), increasing counter-clockwise.
void Angles(const Vec3& from, const Vec3& to, float& heading, float& pitch) {
    float dx = to.x - from.x, dy = to.y - from.y, dz = to.z - from.z;
    heading = std::atan2(-dx, dy) * kRadToDeg;
    pitch = std::atan2(dz, std::sqrt(dx * dx + dy * dy)) * kRadToDeg;
}

Vec3 AimPoint(Ped p) {
    int bone = g.aimBone == AimBone::Head ? kBoneHead : kBoneBody;
    return Invoke<Vec3>(N::GET_PED_BONE_COORDS, p, bone, 0.0f, 0.0f, 0.0f);
}

// Angular distance between the crosshair and the target.
float AngleFromCrosshair(const Vec3& target) {
    Vec3 cam = Invoke<Vec3>(N::GET_GAMEPLAY_CAM_COORD);
    Vec3 rot = Invoke<Vec3>(N::GET_GAMEPLAY_CAM_ROT, 2);
    float heading, pitch;
    Angles(cam, target, heading, pitch);
    float dh = WrapDegrees(heading - rot.z), dp = pitch - rot.x;
    return std::sqrt(dh * dh + dp * dp);
}

// Filters that don't need a raycast. `dist` is set for candidates.
bool IsAimCandidate(Ped p, Ped player, const Vec3& me, float& dist) {
    if (p == player || !Invoke<int>(N::DOES_ENTITY_EXIST, p) || Invoke<int>(N::IS_ENTITY_DEAD, p, 0))
        return false;
    if (g.aimIgnoreAnimals && !Invoke<int>(N::IS_PED_HUMAN, p))
        return false;
    if (g.aimIgnoreCops && IsLawEnforcement(p))
        return false;

    Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, p, 1);
    dist = Distance(me, pos);
    if (dist > g.pedRange)
        return false;
    if (g.aimFov < 180 && AngleFromCrosshair(pos) > g.aimFov)
        return false;
    if (g.aimPriority == AimPriority::HostilesOnly && !IsThreat(p, player))
        return false;
    return true;
}

// Best visible ped: hostiles first if requested, then nearest.
// Candidates are LOS-tested in priority order, since the raycast is the expensive part.
Ped FindAimTarget(Ped player, const Vec3& me) {
    struct Candidate { int tier; float dist; Ped ped; };
    static int peds[512];
    static Candidate candidates[512];
    int count = worldGetAllPeds(peds, 512);
    int n = 0;

    for (int i = 0; i < count; ++i) {
        float dist;
        if (!IsAimCandidate(peds[i], player, me, dist)) continue;
        int tier = (g.aimPriority == AimPriority::HostilesFirst && !IsThreat(peds[i], player)) ? 1 : 0;
        candidates[n++] = { tier, dist, peds[i] };
    }
    std::sort(candidates, candidates + n, [](const Candidate& a, const Candidate& b) {
        return a.tier != b.tier ? a.tier < b.tier : a.dist < b.dist;
    });

    for (int i = 0; i < n; ++i) {
        if (HasLineOfSight(player, candidates[i].ped))
            return candidates[i].ped;
    }
    return 0;
}

// Vehicle aim state: the relative angles we last asked for. Cleared whenever auto-aim isn't
// driving the camera, so the next lock starts from where the camera really is.
bool s_vehAimActive = false;
float s_vehHeading = 0.0f, s_vehPitch = 0.0f;

// In a vehicle the relative angles aren't measured from the ped's heading and the horizon: they
// follow the vehicle (its heading, its pitch on slopes) and the vehicle camera adds its own tilt,
// and the pitch getter doesn't match what the setter applies. So steer by feedback instead:
// measure the angle between the rendered camera (what's on screen, so the crosshair) and the
// target, and add that error to the value we set last frame. Repeated each frame this settles
// with the crosshair on the target whatever the offsets are.
void AimAtFromVehicle(Ped target, float t) {
    Vec3 cam = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_COORD);
    Vec3 rot = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_ROT, 2);
    float heading, pitch;
    Angles(cam, AimPoint(target), heading, pitch);

    if (!s_vehAimActive) {
        s_vehHeading = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_HEADING);
        s_vehPitch = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_PITCH);
        s_vehAimActive = true;
    }
    s_vehHeading = WrapDegrees(s_vehHeading + WrapDegrees(heading - rot.z) * t);
    s_vehPitch = (std::min)((std::max)(s_vehPitch + (pitch - rot.x) * t, -80.0f), 80.0f);

    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_HEADING, s_vehHeading);
    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_PITCH, s_vehPitch, 1.0f);
}

// Turns the gameplay camera (and therefore the crosshair) toward the target,
// moving 1/smoothing of the remaining angle each frame.
void AimAt(Ped player, Ped target) {
    float t = 1.0f / (std::max)(g.aimSmoothing, 1);
    if (Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0)) {
        AimAtFromVehicle(target, t);
        return;
    }
    s_vehAimActive = false;

    Vec3 cam = Invoke<Vec3>(N::GET_GAMEPLAY_CAM_COORD);
    float heading, pitch;
    Angles(cam, AimPoint(target), heading, pitch);

    float wantHeading = WrapDegrees(heading - Invoke<float>(N::GET_ENTITY_HEADING, player));
    float wantPitch = pitch;

    float curHeading = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_HEADING);
    float curPitch = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_PITCH);

    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_HEADING, curHeading + WrapDegrees(wantHeading - curHeading) * t);
    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_PITCH, curPitch + (wantPitch - curPitch) * t, 1.0f);
}

// Grenades, sticky bombs, molotovs etc. are thrown in an arc, so locking the camera onto a
// target would only ruin the throw.
bool HoldingThrowable(Ped player) {
    static Hash thrownGroup = 0;
    if (!thrownGroup)
        thrownGroup = Invoke<Hash>(N::GET_HASH_KEY, "GROUP_THROWN");
    Hash weapon = Invoke<Hash>(N::GET_SELECTED_PED_WEAPON, player);
    return Invoke<Hash>(N::GET_WEAPONTYPE_GROUP, weapon) == thrownGroup;
}

// Circle on screen showing the FOV limit.
void DrawFovCircle() {
    if (g.aimFov >= 90) return; // wider than the screen
    float camFov = Invoke<float>(N::GET_GAMEPLAY_CAM_FOV);
    float ry = 0.5f * std::tan(g.aimFov * kDegToRad) / std::tan(camFov * 0.5f * kDegToRad);
    if (ry > 0.75f) return;

    const float aspect = AspectRatio();
    const float rx = ry / aspect;
    const int segments = 64;
    const float dot = 0.0018f;
    for (int i = 0; i < segments; ++i) {
        float a = 2.0f * kPi * i / segments;
        Rect(0.5f + std::cos(a) * rx, 0.5f + std::sin(a) * ry, dot / aspect, dot, { 255, 255, 255, 120 });
    }
}

} // namespace

bool HasLineOfSight(Ped player, Ped p) {
    constexpr int kLosTraceFlags = 17; // world geometry + vehicles + objects
    return Invoke<int>(N::HAS_ENTITY_CLEAR_LOS_TO_ENTITY, player, p, kLosTraceFlags) != 0;
}

// Holds the current target while it stays valid and visible; re-acquires otherwise.
void AimTick(Ped player, const Vec3& me) {
    if (g.showFov)
        DrawFovCircle();

    if (!Invoke<int>(N::IS_PLAYER_FREE_AIMING, Invoke<Player>(N::PLAYER_ID)) || HoldingThrowable(player)) {
        g_aimTarget = 0;
        s_vehAimActive = false;
        return;
    }

    float dist;
    if (!g_aimTarget || !IsAimCandidate(g_aimTarget, player, me, dist) || !HasLineOfSight(player, g_aimTarget))
        g_aimTarget = FindAimTarget(player, me);

    if (g_aimTarget)
        AimAt(player, g_aimTarget);
    else
        s_vehAimActive = false;
}
