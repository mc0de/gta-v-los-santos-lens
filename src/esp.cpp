#include "features.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <unordered_map>

namespace {

constexpr float kRadToDeg = 57.2957795f;
constexpr float kDegToRad = 0.0174532925f;
constexpr float kLabelScale = 0.2f;
constexpr float kLabelHeight = 0.016f; // approx. screen height of a label line at kLabelScale

constexpr Color kTan { 210, 180, 130, 220 };

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
// World model of each weapon in kWeapons, for naming weapon pickups.
Hash g_weaponModels[sizeof kWeapons / sizeof kWeapons[0]] = {};

// Natives can only run on the script thread, so weapon hashes are resolved on first use.
void InitWeaponNames() {
    if (g_unarmed) return;
    g_unarmed = Invoke<Hash>(N::GET_HASH_KEY, "WEAPON_UNARMED");
    for (size_t i = 0; i < std::size(kWeapons); ++i) {
        kWeapons[i].hash = Invoke<Hash>(N::GET_HASH_KEY, kWeapons[i].model);
        g_weaponModels[i] = Invoke<Hash>(N::GET_WEAPONTYPE_MODEL, kWeapons[i].hash);
    }
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

enum class PedCategory { Hostile, Animal, Cop, Pedestrian };

// Animals always land in Animals (even when attacking) so that category's switch hides all of them.
PedCategory Classify(Ped p, Ped player, bool human) {
    if (!human)               return PedCategory::Animal;
    if (IsThreat(p, player))  return PedCategory::Hostile;
    if (IsLawEnforcement(p))  return PedCategory::Cop;
    return PedCategory::Pedestrian;
}

const EspStyle& StyleFor(PedCategory c) {
    switch (c) {
        case PedCategory::Hostile: return g.hostiles;
        case PedCategory::Animal:  return g.animals;
        case PedCategory::Cop:     return g.cops;
        default:                   return g.pedestrians;
    }
}

// Hostiles: green when you have a clear shot, red when they're behind cover.
Color ThreatColor(Ped p, Ped player) {
    return HasLineOfSight(player, p) ? kGreen : kRed;
}

Color CategoryColor(PedCategory c, Ped p, Ped player) {
    switch (c) {
        case PedCategory::Hostile: return ThreatColor(p, player);
        case PedCategory::Animal:  return IsThreat(p, player) ? ThreatColor(p, player) : kTan;
        case PedCategory::Cop:     return kBlue;
        default:
            // companion / respect / like
            return Invoke<int>(N::GET_RELATIONSHIP_BETWEEN_PEDS, p, player) <= 2 ? kLilac : kWhite;
    }
}

// Far targets fade out so crowded areas stay readable.
Color Faded(Color c, float dist, float range) {
    if (!g.distanceFade || range <= 0.0f) return c;
    return WithAlpha(c, 1.0f - 0.7f * (std::min)(dist / range, 1.0f));
}

// Draws the parts of a target that its style enables: box, health bar and label above it,
// distance below, tracer from the player. labelDot adds a small square right of the label.
void DrawTarget(const EspStyle& s, const Box2D& box, Color c, const Vec3& me, const Vec3& pos,
                float dist, float healthFrac, const char* label, bool forceBox, bool labelDot = false) {
    if (s.box || forceBox)
        DrawBox(box, c);

    float labelY = box.y0 - kLabelHeight - 0.002f;
    if (s.health && healthFrac >= 0.0f) {
        DrawHealthBar(box, healthFrac);
        labelY -= kHealthBarGap + 0.002f;
    }

    float cx = (box.x0 + box.x1) * 0.5f;
    if (s.labels && label && *label) {
        Text(cx, labelY, label, c, kLabelScale);
        if (labelDot) {
            const float h = 0.007f, w = h / AspectRatio();
            float x = cx + TextWidth(label, kLabelScale) * 0.5f + w * 1.5f;
            Rect(x, labelY + kLabelHeight * 0.55f, w, h, c);
        }
    }

    if (s.distance) {
        char buf[16];
        std::snprintf(buf, sizeof buf, "%.0fm", dist);
        Text(cx, box.y1 + 0.002f, buf, c, kLabelScale);
    }

    if (s.tracers)
        OverlayLine(me, pos, c);
}

void DrawPeds(Ped player, const Vec3& me) {
    char label[64];

    for (int i = 0; i < g_pedCount; ++i) {
        Ped p = g_peds[i];
        if (p == player) continue;

        bool dead = Invoke<int>(N::IS_ENTITY_DEAD, p, 0) != 0;
        if (dead && !g.showDead) continue;

        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, p, 1);
        float dist = Distance(me, pos);
        if (dist > g.pedRange) continue;

        bool human = Invoke<int>(N::IS_PED_HUMAN, p) != 0;
        PedCategory cat = Classify(p, player, human);
        const EspStyle& s = StyleFor(cat);
        // The auto-aim target is always shown, even if its category is off.
        bool isTarget = p == g_aimTarget;
        if (!s.enabled && !isTarget) continue;

        Box2D box;
        if (!ScreenBox(p, box)) continue;

        Color c = Faded(isTarget ? kMagenta : dead ? kGrey : CategoryColor(cat, p, player), dist, g.pedRange);

        float healthFrac = -1.0f;
        if (s.health && !dead) {
            int hp = Invoke<int>(N::GET_ENTITY_HEALTH, p);
            int maxHp = Invoke<int>(N::GET_ENTITY_MAX_HEALTH, p);
            // Peds die at 100 HP, so the usable range is [100, max].
            if (maxHp > 100)
                healthFrac = (hp - 100) / float(maxHp - 100);
        }

        label[0] = '\0';
        if (s.labels && !dead) {
            const char* law = LawLabel(p);
            const char* weapon = WeaponLabel(p);
            std::snprintf(label, sizeof label, "%s%s%s", law ? law : "", law && weapon ? " | " : "", weapon ? weapon : "");
        }

        DrawTarget(s, box, c, me, pos, dist, healthFrac, label, isTarget);
    }
}

void DrawVehicles(Ped player, const Vec3& me) {
    const EspStyle& s = g.vehicles;
    if (!s.enabled) return;

    static int vehs[512];
    int count = worldGetAllVehicles(vehs, 512);
    char label[128];
    Vehicle mine = Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0)
                       ? Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_USING, player) : 0;

    for (int i = 0; i < count; ++i) {
        Vehicle v = vehs[i];
        if (v == mine) continue;
        bool wrecked = Invoke<int>(N::IS_ENTITY_DEAD, v, 0) != 0;
        if (wrecked && !g.showDead) continue;

        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, v, 1);
        float dist = Distance(me, pos);
        if (dist > g.vehRange) continue;

        int cls = Invoke<int>(N::GET_VEHICLE_CLASS, v);
        if (cls >= 0 && cls < kVehicleClassCount && !g.vehTypes[cls]) continue;

        Box2D box;
        if (!ScreenBox(v, box)) continue;

        bool emergency = cls == 18;
        Color c = Faded(wrecked ? kGrey : emergency ? kBlue : kYellow, dist, g.vehRange);

        float healthFrac = -1.0f;
        if (s.health && !wrecked) {
            int maxHp = Invoke<int>(N::GET_ENTITY_MAX_HEALTH, v);
            if (maxHp > 0)
                healthFrac = Invoke<int>(N::GET_ENTITY_HEALTH, v) / float(maxHp);
        }

        label[0] = '\0';
        bool occupied = false;
        if (s.labels) {
            const char* name = VehicleModelName(Invoke<Hash>(N::GET_ENTITY_MODEL, v));
            int len = std::snprintf(label, sizeof label, "%s", *name ? name : "?");
            if (!wrecked && len > 0 && len < static_cast<int>(sizeof label)) {
                Ped driver = Invoke<Ped>(N::GET_PED_IN_VEHICLE_SEAT, v, -1);
                occupied = driver && driver != player;
                int lock = Invoke<int>(N::GET_VEHICLE_DOOR_LOCK_STATUS, v);
                bool locked = lock == 2 || lock == 3 || lock == 4 || lock == 10;
                std::snprintf(label + len, sizeof label - len, "%s%s",
                              emergency ? " | emergency" : "",
                              locked ? " | locked" : "");
            }
        }

        // Occupied vehicles get a dot after the label instead of a text tag.
        DrawTarget(s, box, c, me, pos, dist, healthFrac, label, false, occupied);
    }
}

// Arrows around the screen centre pointing at hostiles that are off-screen.
void DrawThreatArrows(Ped player, const Vec3& me) {
    Vec3 camRot = Invoke<Vec3>(N::GET_GAMEPLAY_CAM_ROT, 2);
    const float ry = 0.32f;
    const float rx = ry / AspectRatio();
    const float size = 0.022f;

    for (int i = 0; i < g_pedCount; ++i) {
        Ped p = g_peds[i];
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

const char* WeaponLabelForModel(Hash model) {
    InitWeaponNames();
    if (model == 0) return nullptr;
    for (size_t i = 0; i < std::size(kWeapons); ++i)
        if (g_weaponModels[i] == model) return kWeapons[i].label;
    return nullptr;
}

const char* VehicleModelName(Hash model) {
    const char* code = Invoke<const char*>(N::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL, model);
    if (!code) return "";
    const char* name = Invoke<const char*>(N::GET_FILENAME_FOR_AUDIO_CONVERSATION, code);
    return name && *name && std::strcmp(name, "NULL") != 0 ? name : code;
}

int g_peds[512];
int g_pedCount = 0;

namespace {

struct PedCache {
    int threatFrame = -1;
    bool threat = false;
    int losTime = 0;
    bool losValid = false;
    bool los = false;
};
std::unordered_map<Ped, PedCache> g_pedCache;
int g_frame = 0;
int g_now = 0;
int g_nextCachePrune = 0;

} // namespace

void BeginFrame() {
    ++g_frame;
    g_now = Invoke<int>(N::GET_GAME_TIMER);
    g_pedCount = worldGetAllPeds(g_peds, 512);
    // Peds come and go; dropping the whole cache now and then keeps it from growing.
    if (g_now >= g_nextCachePrune || g_now < g_nextCachePrune - 10000) {
        g_pedCache.clear();
        g_nextCachePrune = g_now + 5000;
    }
}

bool HasLineOfSight(Ped player, Ped p) {
    constexpr int kLosTraceFlags = 1 | 2 | 16; // world geometry + vehicles + objects
    constexpr int kLosCacheMs = 100;
    PedCache& c = g_pedCache[p];
    if (!c.losValid || g_now - c.losTime >= kLosCacheMs || g_now < c.losTime) {
        c.los = Invoke<int>(N::HAS_ENTITY_CLEAR_LOS_TO_ENTITY, player, p, kLosTraceFlags) != 0;
        c.losTime = g_now;
        c.losValid = true;
    }
    return c.los;
}

bool IsLawEnforcement(Ped p) {
    switch (Invoke<int>(N::GET_PED_TYPE, p)) {
        case 6: case 27: case 29: return true; // cop, swat, army
        default: return false;
    }
}

namespace {

bool ComputeThreat(Ped p, Ped player) {
    if (Invoke<int>(N::IS_PED_IN_COMBAT, p, player)) return true;
    // Wild animals "hate" the player by default even while idle, so for animals only
    // actual combat counts.
    if (!Invoke<int>(N::IS_PED_HUMAN, p)) return false;
    int rel = Invoke<int>(N::GET_RELATIONSHIP_BETWEEN_PEDS, p, player);
    return rel == 4 || rel == 5; // dislike / hate
}

} // namespace

bool IsThreat(Ped p, Ped player) {
    PedCache& c = g_pedCache[p];
    if (c.threatFrame != g_frame) {
        c.threat = ComputeThreat(p, player);
        c.threatFrame = g_frame;
    }
    return c.threat;
}

void EspTick(Ped player, const Vec3& me) {
    InitWeaponNames();
    DrawPeds(player, me);
    DrawVehicles(player, me);
    if (g.threatArrows) DrawThreatArrows(player, me);
}
