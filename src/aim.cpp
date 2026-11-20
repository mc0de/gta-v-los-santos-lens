#include "features.h"
#include <algorithm>
#include <cmath>

Ped g_aimTarget = 0;

namespace {

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
// target, and add that whole error to the value we set last frame, so the crosshair lands on the
// target whatever the offsets are.
//
// Steering only happens while the gameplay camera is the one on screen: a cinematic or scripted
// camera ignores the setter, so the error would never shrink and the accumulator would run away.
void AimAtFromVehicle(Ped target) {
    if (!Invoke<int>(N::IS_GAMEPLAY_CAM_RENDERING)) {
        s_vehAimActive = false;
        return;
    }
    Vec3 cam = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_COORD);
    Vec3 rot = Invoke<Vec3>(N::GET_FINAL_RENDERED_CAM_ROT, 2);
    float heading, pitch;
    Angles(cam, AimPoint(target), heading, pitch);

    if (!s_vehAimActive) {
        s_vehHeading = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_HEADING);
        s_vehPitch = Invoke<float>(N::GET_GAMEPLAY_CAM_RELATIVE_PITCH);
        s_vehAimActive = true;
    }
    s_vehHeading = WrapDegrees(s_vehHeading + WrapDegrees(heading - rot.z));
    s_vehPitch = (std::clamp)(s_vehPitch + (pitch - rot.x), -80.0f, 80.0f);

    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_HEADING, s_vehHeading);
    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_PITCH, s_vehPitch, 1.0f);
}

// Points the gameplay camera (and therefore the crosshair) straight at the target.
void AimAt(Ped player, Ped target) {
    if (Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0)) {
        AimAtFromVehicle(target);
        return;
    }
    s_vehAimActive = false;

    Vec3 cam = Invoke<Vec3>(N::GET_GAMEPLAY_CAM_COORD);
    float heading, pitch;
    Angles(cam, AimPoint(target), heading, pitch);

    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_HEADING, WrapDegrees(heading - Invoke<float>(N::GET_ENTITY_HEADING, player)));
    Invoke(N::SET_GAMEPLAY_CAM_RELATIVE_PITCH, pitch, 1.0f);
}

// Degrees between the directions from the camera to `a` and to `b`.
float AngleBetween(const Vec3& from, const Vec3& a, const Vec3& b) {
    float ax = a.x - from.x, ay = a.y - from.y, az = a.z - from.z;
    float bx = b.x - from.x, by = b.y - from.y, bz = b.z - from.z;
    float la = std::sqrt(ax * ax + ay * ay + az * az), lb = std::sqrt(bx * bx + by * by + bz * bz);
    if (la < 0.001f || lb < 0.001f) return 0.0f;
    float c = (ax * bx + ay * by + az * bz) / (la * lb);
    return std::acos((std::min)((std::max)(c, -1.0f), 1.0f)) * kRadToDeg;
}

// The visible target nearest to the current one on its left (dir = +1) or right (dir = -1),
// or 0 if there's none on that side. Sides are by horizontal angle as seen from the camera.
Ped NeighbourTarget(Ped player, const Vec3& me, const View& view, Ped current, int dir) {
    struct Option { Ped ped; float sep; };
    static Option options[512];
    int n = 0;

    Vec3 curPoint = AimPoint(current);
    float curHeading, pitch;
    Angles(view.pos, curPoint, curHeading, pitch);

    for (int i = 0; i < g_pedCount; ++i) {
        Candidate c;
        if (g_peds[i] == current || !IsAimCandidate(g_peds[i], player, me, view, c)) continue;
        Vec3 point = AimPoint(c.ped);
        float heading;
        Angles(view.pos, point, heading, pitch);
        // GTA headings grow counter-clockwise, so a larger heading is further left.
        if (WrapDegrees(heading - curHeading) * dir <= 0.0f) continue;
        options[n++] = { c.ped, AngleBetween(view.pos, curPoint, point) };
    }
    std::sort(options, options + n, [](const Option& a, const Option& b) { return a.sep < b.sep; });

    for (int i = 0; i < n; ++i) {
        if (HasLineOfSight(player, options[i].ped))
            return options[i].ped;
    }
    return 0;
}

// Game actions on the target-switch keys, turned off while auto-aim is aiming so pressing them
// doesn't also take cover, open the radio wheel or sound the horn. Only the default PC bindings
// of Q and E are known; other keys are left alone.
void BlockSwitchKeyActions(int vk) {
    static const int kQ[] = { 44, 85 };         // INPUT_COVER, INPUT_VEH_RADIO_WHEEL
    static const int kE[] = { 38, 46, 51, 86 }; // INPUT_PICKUP, INPUT_TALK, INPUT_CONTEXT, INPUT_VEH_HORN
    if (vk == 'Q') for (int c : kQ) Invoke(N::DISABLE_CONTROL_ACTION, 0, c, 1);
    if (vk == 'E') for (int c : kE) Invoke(N::DISABLE_CONTROL_ACTION, 0, c, 1);
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
// In Nearest mode a peaceful lock is re-checked a few times a second, so a hostile takes over,
// unless you picked that target yourself with the switch keys.
void AimTick(Ped player, const Vec3& me) {
    static bool s_manual = false; // the current lock was chosen with the switch keys
    if (!Invoke<int>(N::IS_PLAYER_FREE_AIMING, Invoke<Player>(N::PLAYER_ID)) || HoldingThrowable(player)) {
        g_aimTarget = 0;
        s_manual = false;
        s_vehAimActive = false;
        return;
    }
    BlockSwitchKeyActions(g.keys.aimLeft);
    BlockSwitchKeyActions(g.keys.aimRight);
    // Polled every aiming frame (not only with a lock) so a press is never seen late.
    bool left = KeyPressed(g.keys.aimLeft), right = KeyPressed(g.keys.aimRight);

    static int s_recheck = 0;
    View view = ReadView();
    Candidate cur;
    bool valid = g_aimTarget && IsAimCandidate(g_aimTarget, player, me, view, cur) && HasLineOfSight(player, g_aimTarget);
    if (!valid) {
        g_aimTarget = FindAimTarget(player, me, view);
        s_manual = false;
    } else {
        int dir = left == right ? 0 : left ? 1 : -1;
        Ped next = dir ? NeighbourTarget(player, me, view, g_aimTarget, dir) : 0;
        if (next) {
            g_aimTarget = next;
            s_manual = true;
        } else if (g.aimPick == AimPick::Nearest && !cur.hostile && !s_manual && ++s_recheck >= 10) {
            s_recheck = 0;
            Ped best = FindAimTarget(player, me, view);
            if (best && IsThreat(best, player)) g_aimTarget = best;
        }
    }

    if (g_aimTarget)
        AimAt(player, g_aimTarget);
    else
        s_vehAimActive = false;
}
