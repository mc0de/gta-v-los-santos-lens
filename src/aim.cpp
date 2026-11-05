#include "features.h"
#include <algorithm>
#include <cmath>

Ped g_aimTarget = 0;

namespace {

constexpr int kBoneHead = 31086;      // SKEL_Head
constexpr int kBoneBody = 24818;      // SKEL_Spine3 (upper chest)
constexpr float kRadToDeg = 57.2957795f;
constexpr float kDegToRad = 0.0174532925f;

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

// The camera on screen (so the crosshair), read once per tick.
struct View {
    Vec3 pos;
    float fx, fy, fz; // unit forward vector
};

View ReadView() {
    View v;
    v.pos = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_COORD);
    Vec3 rot = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_ROT, 2);
    float h = rot.z * kDegToRad, p = rot.x * kDegToRad;
    v.fx = -std::sin(h) * std::cos(p);
    v.fy = std::cos(h) * std::cos(p);
    v.fz = std::sin(p);
    return v;
}

// Degrees between the crosshair and `target`.
float AngleFromCrosshair(const View& v, const Vec3& target) {
    float dx = target.x - v.pos.x, dy = target.y - v.pos.y, dz = target.z - v.pos.z;
    float len = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (len < 0.001f) return 0.0f;
    float c = (dx * v.fx + dy * v.fy + dz * v.fz) / len;
    return std::acos((std::min)((std::max)(c, -1.0f), 1.0f)) * kRadToDeg;
}

// Hostiles (anyone fighting you, including cops after you and attacking animals) are always
// targets; peaceful cops, pedestrians and animals only when their toggle is on.
bool IsAllowedTarget(Ped p, Ped player, bool& hostile) {
    hostile = IsThreat(p, player);
    if (hostile) return true;
    if (!Invoke<int>(N::IS_PED_HUMAN, p)) return g.aimAnimals;
    if (IsLawEnforcement(p)) return g.aimCops;
    return g.aimPedestrians;
}

struct Candidate {
    Ped ped;
    bool hostile;
    float dist;
    float angle; // from the crosshair, to the aim point
};

// Filters that don't need a raycast. Fills `c` for candidates.
bool IsAimCandidate(Ped p, Ped player, const Vec3& me, const View& view, Candidate& c) {
    if (p == player || !Invoke<int>(N::DOES_ENTITY_EXIST, p) || Invoke<int>(N::IS_ENTITY_DEAD, p, 0))
        return false;

    c.ped = p;
    c.dist = Distance(me, Invoke<Vec3>(N::GET_ENTITY_COORDS, p, 1));
    if (c.dist > g.aimRange)
        return false;
    if (!IsAllowedTarget(p, player, c.hostile))
        return false;
    c.angle = AngleFromCrosshair(view, AimPoint(p));
    return g.aimFov >= 180 || c.angle <= g.aimFov;
}

// Crosshair: the target closest to where you're aiming.
// Nearest: hostiles before peaceful targets, then by distance.
bool Better(const Candidate& a, const Candidate& b) {
    if (g.aimPick == AimPick::Crosshair) return a.angle < b.angle;
    if (a.hostile != b.hostile) return a.hostile;
    return a.dist < b.dist;
}

// Best visible ped. Candidates are LOS-tested in order, since the raycast is the expensive part.
Ped FindAimTarget(Ped player, const Vec3& me, const View& view) {
    static Candidate candidates[512];
    int n = 0;

    for (int i = 0; i < g_pedCount; ++i) {
        if (IsAimCandidate(g_peds[i], player, me, view, candidates[n])) ++n;
    }
    std::sort(candidates, candidates + n, Better);

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
//
// Two guards keep the accumulator from running away: steering only happens while the gameplay
// camera is the one on screen (a cinematic or scripted camera ignores the setter, so the error
// would never shrink), and each frame's correction is capped.
void AimAtFromVehicle(Ped target, float t) {
    if (!Invoke<int>(N::IS_GAMEPLAY_CAM_RENDERING)) {
        s_vehAimActive = false;
        return;
    }
    constexpr float kMaxStep = 30.0f; // degrees per frame

    Vec3 cam = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_COORD);
    Vec3 rot = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_ROT, 2);
    float heading, pitch;
    Angles(cam, AimPoint(target), heading, pitch);

    if (!s_vehAimActive) {
        s_vehHeading = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_HEADING);
        s_vehPitch = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_PITCH);
        s_vehAimActive = true;
    }
    float dh = (std::clamp)(WrapDegrees(heading - rot.z) * t, -kMaxStep, kMaxStep);
    float dp = (std::clamp)((pitch - rot.x) * t, -kMaxStep, kMaxStep);
    s_vehHeading = WrapDegrees(s_vehHeading + dh);
    s_vehPitch = (std::clamp)(s_vehPitch + dp, -80.0f, 80.0f);

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

} // namespace


// Holds the current target while it stays valid and visible; re-acquires otherwise.
// In Nearest mode a peaceful lock is re-checked a few times a second, so a hostile takes over.
void AimTick(Ped player, const Vec3& me) {
    if (!Invoke<int>(N::IS_PLAYER_FREE_AIMING, Invoke<Player>(N::PLAYER_ID)) || HoldingThrowable(player)) {
        g_aimTarget = 0;
        s_vehAimActive = false;
        return;
    }

    static int s_recheck = 0;
    View view = ReadView();
    Candidate cur;
    bool valid = g_aimTarget && IsAimCandidate(g_aimTarget, player, me, view, cur) && HasLineOfSight(player, g_aimTarget);
    if (!valid) {
        g_aimTarget = FindAimTarget(player, me, view);
    } else if (g.aimPick == AimPick::Nearest && !cur.hostile && ++s_recheck >= 10) {
        s_recheck = 0;
        Ped best = FindAimTarget(player, me, view);
        if (best && IsThreat(best, player)) g_aimTarget = best;
    }

    if (g_aimTarget)
        AimAt(player, g_aimTarget);
    else
        s_vehAimActive = false;
}
