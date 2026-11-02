#include "features.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

constexpr float kRadToDeg = 57.2957795f;
constexpr float kDegToRad = 0.0174532925f;
constexpr float kLabelScale = 0.2f;
constexpr float kLabelHeight = 0.016f; // approx. screen height of a label line at kLabelScale

// Bones used for skeleton ESP, and the pairs (indices into kSkeletonBones) to connect.
constexpr int kSkeletonBones[] = {
    31086, // 0  SKEL_Head
    39317, // 1  SKEL_Neck_1
    24818, // 2  SKEL_Spine3
    11816, // 3  SKEL_Pelvis
    45509, // 4  SKEL_L_UpperArm
    61163, // 5  SKEL_L_Forearm
    18905, // 6  SKEL_L_Hand
    40269, // 7  SKEL_R_UpperArm
    28252, // 8  SKEL_R_Forearm
    57005, // 9  SKEL_R_Hand
    58271, // 10 SKEL_L_Thigh
    63931, // 11 SKEL_L_Calf
    14201, // 12 SKEL_L_Foot
    51826, // 13 SKEL_R_Thigh
    36864, // 14 SKEL_R_Calf
    52301, // 15 SKEL_R_Foot
};
constexpr int kSkeletonLinks[][2] = {
    { 0, 1 }, { 1, 2 }, { 2, 3 },                 // head to pelvis
    { 1, 4 }, { 4, 5 }, { 5, 6 },                 // left arm
    { 1, 7 }, { 7, 8 }, { 8, 9 },                 // right arm
    { 3, 10 }, { 10, 11 }, { 11, 12 },            // left leg
    { 3, 13 }, { 13, 14 }, { 14, 15 },            // right leg
};

struct WeaponName { const char* model; const char* label; Hash hash; };
WeaponName kWeapons[] = {
    { "WEAPON_PISTOL", "Pistol" }, { "WEAPON_COMBATPISTOL", "Combat Pistol" },
    { "WEAPON_APPISTOL", "AP Pistol" }, { "WEAPON_PISTOL50", "Pistol .50" },
    { "WEAPON_SNSPISTOL", "SNS Pistol" }, { "WEAPON_HEAVYPISTOL", "Heavy Pistol" },
    { "WEAPON_VINTAGEPISTOL", "Vintage Pistol" }, { "WEAPON_REVOLVER", "Revolver" },
    { "WEAPON_STUNGUN", "Taser" }, { "WEAPON_MICROSMG", "Micro SMG" },
    { "WEAPON_SMG", "SMG" }, { "WEAPON_ASSAULTSMG", "Assault SMG" },
    { "WEAPON_COMBATPDW", "Combat PDW" }, { "WEAPON_MG", "MG" },
    { "WEAPON_COMBATMG", "Combat MG" }, { "WEAPON_GUSENBERG", "Gusenberg" },
    { "WEAPON_ASSAULTRIFLE", "Assault Rifle" }, { "WEAPON_CARBINERIFLE", "Carbine" },
    { "WEAPON_ADVANCEDRIFLE", "Advanced Rifle" }, { "WEAPON_SPECIALCARBINE", "Special Carbine" },
    { "WEAPON_BULLPUPRIFLE", "Bullpup Rifle" }, { "WEAPON_COMPACTRIFLE", "Compact Rifle" },
    { "WEAPON_PUMPSHOTGUN", "Pump Shotgun" }, { "WEAPON_SAWNOFFSHOTGUN", "Sawed-Off" },
    { "WEAPON_ASSAULTSHOTGUN", "Assault Shotgun" }, { "WEAPON_BULLPUPSHOTGUN", "Bullpup Shotgun" },
    { "WEAPON_HEAVYSHOTGUN", "Heavy Shotgun" }, { "WEAPON_SNIPERRIFLE", "Sniper" },
    { "WEAPON_HEAVYSNIPER", "Heavy Sniper" }, { "WEAPON_MARKSMANRIFLE", "Marksman Rifle" },
    { "WEAPON_RPG", "RPG" }, { "WEAPON_GRENADELAUNCHER", "Grenade Launcher" },
    { "WEAPON_MINIGUN", "Minigun" }, { "WEAPON_KNIFE", "Knife" },
    { "WEAPON_BAT", "Bat" }, { "WEAPON_CROWBAR", "Crowbar" },
    { "WEAPON_GOLFCLUB", "Golf Club" }, { "WEAPON_HAMMER", "Hammer" },
    { "WEAPON_NIGHTSTICK", "Nightstick" }, { "WEAPON_BOTTLE", "Bottle" },
    { "WEAPON_MACHETE", "Machete" }, { "WEAPON_SWITCHBLADE", "Switchblade" },
    { "WEAPON_KNUCKLE", "Knuckles" }, { "WEAPON_GRENADE", "Grenade" },
    { "WEAPON_MOLOTOV", "Molotov" }, { "WEAPON_STICKYBOMB", "Sticky Bomb" },
};
Hash g_unarmed = 0;

// Natives can only run on the script thread, so weapon hashes are resolved on first use.
void InitWeaponNames() {
    if (g_unarmed) return;
    g_unarmed = Invoke<Hash>(N::GET_HASH_KEY, "WEAPON_UNARMED");
    for (auto& w : kWeapons)
        w.hash = Invoke<Hash>(N::GET_HASH_KEY, w.model);
}

// Returns nullptr for unarmed peds and for weapon hashes not in kWeapons, which include
// non-weapon "weapons" like phones and animal attacks.
const char* WeaponLabel(Ped p) {
    Hash h = Invoke<Hash>(N::GET_SELECTED_PED_WEAPON, p);
    if (h == 0 || h == g_unarmed) return nullptr;
    for (const auto& w : kWeapons)
        if (w.hash == h) return w.label;
    return nullptr;
}

const char* LawLabel(Ped p) {
    switch (Invoke<int>(N::GET_PED_TYPE, p)) {
        case 6:  return "COP";
        case 27: return "SWAT";
        case 29: return "ARMY";
        default: return nullptr;
    }
}

// Far targets fade out so crowded areas stay readable.
Color Faded(Color c, float dist, float range) {
    if (!g.distanceFade || range <= 0.0f) return c;
    return WithAlpha(c, 1.0f - 0.7f * (std::min)(dist / range, 1.0f));
}

Color PedColor(Ped ped, Ped player) {
    if (Invoke<int>(N::IS_ENTITY_DEAD, ped, 0)) return kGrey;
    if (IsThreat(ped, player)) return kRed;
    if (IsLawEnforcement(ped)) return kBlue;
    int rel = Invoke<int>(N::GET_RELATIONSHIP_BETWEEN_PEDS, ped, player);
    if (rel <= 2) return kGreen; // companion / respect / like
    return kWhite;
}

void DrawSkeleton(Ped p, Color c) {
    Vec3 pts[std::size(kSkeletonBones)];
    for (size_t i = 0; i < std::size(kSkeletonBones); ++i)
        pts[i] = Invoke<Vec3>(N::GET_PED_BONE_COORDS, p, kSkeletonBones[i], 0.0f, 0.0f, 0.0f);
    for (const auto& link : kSkeletonLinks)
        OverlayLine(pts[link[0]], pts[link[1]], c);
}

void DrawPeds(Ped player, const Vec3& me) {
    static int peds[512];
    int count = worldGetAllPeds(peds, 512);
    char buf[96];

    for (int i = 0; i < count; ++i) {
        Ped p = peds[i];
        if (p == player) continue;

        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, p, 1);
        float dist = Distance(me, pos);
        if (dist > g.pedRange) continue;

        bool human = Invoke<int>(N::IS_PED_HUMAN, p) != 0;
        if (g.espIgnoreAnimals && !human) continue;

        Box2D box;
        if (!ScreenBox(p, box)) continue;

        bool dead = Invoke<int>(N::IS_ENTITY_DEAD, p, 0) != 0;
        Color c = Faded((p == g_aimTarget) ? kMagenta : PedColor(p, player), dist, g.pedRange);

        if (g.boxes)
            DrawBox(box, c);
        if (g.skeletons && !dead && human)
            DrawSkeleton(p, c);

        // Above the box: health bar, then the label line above that.
        float labelY = box.y0 - kLabelHeight - 0.002f;
        if (g.healthBars && !dead) {
            int hp = Invoke<int>(N::GET_ENTITY_HEALTH, p);
            int maxHp = Invoke<int>(N::GET_ENTITY_MAX_HEALTH, p);
            // Peds die at 100 HP, so the usable range is [100, max].
            if (maxHp > 100) {
                DrawHealthBar(box, (hp - 100) / float(maxHp - 100));
                labelY -= kHealthBarGap + 0.002f;
            }
        }

        float cx = (box.x0 + box.x1) * 0.5f;
        std::snprintf(buf, sizeof buf, "%.0fm", dist);
        Text(cx, box.y1 + 0.002f, buf, c, kLabelScale);

        if (g.labels && !dead) {
            const char* law = LawLabel(p);
            const char* weapon = g.weaponLabels ? WeaponLabel(p) : nullptr;
            if (law || weapon) {
                std::snprintf(buf, sizeof buf, "%s%s%s", law ? law : "", law && weapon ? " | " : "", weapon ? weapon : "");
                Text(cx, labelY, buf, c, kLabelScale);
            }
        }

        if (g.tracers && !dead)
            OverlayLine(me, pos, c);
    }
}

void DrawVehicles(Ped player, const Vec3& me) {
    static int vehs[512];
    int count = worldGetAllVehicles(vehs, 512);
    char buf[128];

    for (int i = 0; i < count; ++i) {
        Vehicle v = vehs[i];
        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, v, 1);
        float dist = Distance(me, pos);
        if (dist < 3.0f || dist > g.vehRange) continue; // skip the car we're sitting in

        Box2D box;
        if (!ScreenBox(v, box)) continue;

        bool wrecked = Invoke<int>(N::IS_ENTITY_DEAD, v, 0) != 0;
        bool emergency = Invoke<int>(N::GET_VEHICLE_CLASS, v) == 18;
        Color c = Faded(wrecked ? kGrey : emergency ? kBlue : kYellow, dist, g.vehRange);
        if (g.boxes)
            DrawBox(box, c);

        const char* label = Invoke<const char*>(N::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL,
                                                Invoke<Hash>(N::GET_ENTITY_MODEL, v));
        const char* name = Invoke<const char*>(N::GET_FILENAME_FOR_AUDIO_CONVERSATION, label);
        int len = std::snprintf(buf, sizeof buf, "%s  %.0fm", name ? name : "?", dist);

        if (g.labels && !wrecked && len > 0 && len < static_cast<int>(sizeof buf)) {
            Ped driver = Invoke<Ped>(N::GET_PED_IN_VEHICLE_SEAT, v, -1);
            int lock = Invoke<int>(N::GET_VEHICLE_DOOR_LOCK_STATUS, v);
            bool locked = lock == 2 || lock == 3 || lock == 4 || lock == 10;
            std::snprintf(buf + len, sizeof buf - len, "%s%s%s",
                          emergency ? " | emergency" : "",
                          driver && driver != player ? " | occupied" : "",
                          locked ? " | locked" : "");
        }
        Text((box.x0 + box.x1) * 0.5f, box.y0 - kLabelHeight - 0.002f, buf, c, kLabelScale);

        if (g.tracers)
            OverlayLine(me, pos, c);
    }
}

// Arrows around the screen centre pointing at hostiles that are off-screen.
void DrawThreatArrows(Ped player, const Vec3& me) {
    static int peds[512];
    int count = worldGetAllPeds(peds, 512);

    Vec3 camRot = Invoke<Vec3>(N::GET_GAMEPLAY_CAM_ROT, 2);
    const float ry = 0.32f;
    const float rx = ry / AspectRatio();
    const float size = 0.022f;

    for (int i = 0; i < count; ++i) {
        Ped p = peds[i];
        if (p == player || Invoke<int>(N::IS_ENTITY_DEAD, p, 0)) continue;

        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, p, 1);
        float dist = Distance(me, pos);
        if (dist > g.pedRange || !IsThreat(p, player)) continue;

        float sx, sy;
        if (WorldToScreen(pos.x, pos.y, pos.z, sx, sy) && sx > 0.0f && sx < 1.0f && sy > 0.0f && sy < 1.0f)
            continue;

        // Bearing relative to where the camera faces; 0 = ahead, positive = to the left.
        float bearing = std::atan2(-(pos.x - me.x), pos.y - me.y) * kRadToDeg;
        float rel = (bearing - camRot.z) * kDegToRad;
        float dx = -std::sin(rel), dy = -std::cos(rel); // screen space, y down
        float x = 0.5f + dx * rx, y = 0.5f + dy * ry;

        Color c = Faded(kRed, dist, g.pedRange);
        float rotation = std::atan2(dy, dx) * kRadToDeg; // sprite points right at 0
        if (!Sprite("commonmenu", "arrowright", x, y, size / AspectRatio() * 1.2f, size, rotation, c))
            Rect(x, y, 0.008f / AspectRatio(), 0.008f, c);
    }
}

} // namespace

bool IsLawEnforcement(Ped p) {
    switch (Invoke<int>(N::GET_PED_TYPE, p)) {
        case 6: case 27: case 29: return true; // cop, swat, army
        default: return false;
    }
}

bool IsThreat(Ped p, Ped player) {
    if (Invoke<int>(N::IS_PED_IN_COMBAT, p, player)) return true;
    int rel = Invoke<int>(N::GET_RELATIONSHIP_BETWEEN_PEDS, p, player);
    return rel == 4 || rel == 5; // dislike / hate
}

void EspTick(Ped player, const Vec3& me) {
    InitWeaponNames();
    if (g.targets != Targets::Vehicles) DrawPeds(player, me);
    if (g.targets != Targets::Peds)     DrawVehicles(player, me);
    if (g.threatArrows)                 DrawThreatArrows(player, me);
}
