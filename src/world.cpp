#include "features.h"
#include "vehicle_makes.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

// ---- Collectible / pickup finder ----

constexpr int kCollectible = -1; // Found::type for collectibles; pickups use PickupType

struct Found {
    Entity entity;
    std::string label;
    int type;
    // Line of sight from the player, cached for groups that don't show through walls.
    int losTime = 0;
    bool losValid = false;
    bool los = false;
};

// Pickup colours, by PickupType. Orange is kept for collectibles.
constexpr Color kPickupColors[kPickupTypeCount] = {
    { 255,  60,  60, 240 }, // weapons: red
    {  80, 255, 120, 240 }, // health: green
    {  80, 160, 255, 240 }, // armour: blue
    { 255, 230,  60, 240 }, // ammo: yellow
    { 255,  80, 220, 240 }, // cash: magenta
    {  60, 230, 255, 240 }, // other (parachutes, snacks, packages, ...): cyan
    { 230, 230, 230, 220 }, // unknown model, labelled by hash: white
};

bool Contains(const std::string& s, const char* word) {
    auto it = std::search(s.begin(), s.end(), word, word + std::strlen(word), [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
    });
    return it != s.end();
}

// Non-weapon pickups are typed by the words in their [Pickups] label, so renamed or added
// entries keep their type as long as the label says what they are.
PickupType PickupTypeOf(const std::string& label) {
    if (Contains(label, "health"))                                return kPickupHealth;
    if (Contains(label, "armour") || Contains(label, "armor"))    return kPickupArmour;
    if (Contains(label, "ammo"))                                  return kPickupAmmo;
    if (Contains(label, "cash") || Contains(label, "money") || Contains(label, "briefcase"))
        return kPickupCash;
    return kPickupOther;
}

using ModelLabels = std::vector<std::pair<Hash, std::string>>;

// Rebuilt on every scan so "Reload settings file" changes are picked up.
ModelLabels g_collectibleHashes;
ModelLabels g_pickupHashes;
std::vector<Found> g_found;
int g_nextScan = 0;

// Ini keys are model names, or a model hash written as 0x... (as unknown pickups are labelled).
void ResolveModelHashes(const std::vector<Collectible>& models, ModelLabels& out) {
    out.clear();
    for (const auto& c : models) {
        const char* m = c.model.c_str();
        Hash h = (m[0] == '0' && (m[1] == 'x' || m[1] == 'X'))
                     ? static_cast<Hash>(std::strtoul(m + 2, nullptr, 16))
                     : Invoke<Hash>(N::GET_HASH_KEY, m);
        out.push_back({ h, c.label });
    }
}

const char* FindLabel(const ModelLabels& list, Hash model) {
    for (const auto& [hash, label] : list)
        if (hash == model) return label.c_str();
    return nullptr;
}

bool AlreadyFound(Entity e) {
    for (const auto& f : g_found)
        if (f.entity == e) return true;
    return false;
}

// One pass over the pickup and object pools. Many collectibles (letter scraps, spaceship parts)
// are pickups rather than plain objects, so collectible models are looked for in both pools.
// The object pool can hold thousands of entries, so this runs twice a second, not every frame.
void Scan() {
    g_found.clear();
    const bool collectibles = g.collectibles.enabled, pickupsOn = g.pickups.enabled;
    if (collectibles) ResolveModelHashes(g.collectibleModels, g_collectibleHashes);
    if (pickupsOn)    ResolveModelHashes(g.pickupModels, g_pickupHashes);

    static int pickups[512];
    int count = worldGetAllPickups(pickups, 512);
    for (int i = 0; i < count; ++i) {
        Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, pickups[i]);
        if (collectibles) {
            if (const char* label = FindLabel(g_collectibleHashes, model)) {
                g_found.push_back({ pickups[i], label, kCollectible });
                continue;
            }
        }
        if (!pickupsOn) continue;
        if (const char* weapon = WeaponLabelForModel(model)) {
            g_found.push_back({ pickups[i], weapon, kPickupWeapons });
        } else if (const char* label = FindLabel(g_pickupHashes, model)) {
            g_found.push_back({ pickups[i], label, PickupTypeOf(label) });
        } else {
            // Unknown model: show its hash so it can be named in the ini's [Pickups] section.
            char buf[32];
            std::snprintf(buf, sizeof buf, "Pickup 0x%08X", static_cast<unsigned>(model));
            g_found.push_back({ pickups[i], buf, kPickupUnknown });
        }
    }

    if (collectibles) {
        static int objects[4096];
        count = worldGetAllObjects(objects, 4096);
        for (int i = 0; i < count; ++i) {
            Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, objects[i]);
            if (const char* label = FindLabel(g_collectibleHashes, model))
                if (!AlreadyFound(objects[i])) g_found.push_back({ objects[i], label, kCollectible });
        }
    }
}

bool InSight(Found& f, Ped player, int now) {
    constexpr int kLosCacheMs = 200;
    constexpr int kLosTraceFlags = 1 | 2 | 16; // world geometry + vehicles + objects
    if (!f.losValid || now - f.losTime >= kLosCacheMs || now < f.losTime) {
        f.los = Invoke<int>(N::HAS_ENTITY_CLEAR_LOS_TO_ENTITY, player, f.entity, kLosTraceFlags) != 0;
        f.losTime = now;
        f.losValid = true;
    }
    return f.los;
}

void ScanIfDue() {
    int now = Invoke<int>(N::GET_GAME_TIMER);
    if (now >= g_nextScan || now < g_nextScan - 1000) {
        Scan();
        g_nextScan = now + 500;
    }
}

// ---- Damage numbers ----

// Each number floats up from the ped's head and fades out.
struct Popup { Ped ped; Vec3 pos; int amount; bool kill; int start; };

constexpr int kPopupLifetimeMs = 1000;
// Hits on one ped this close together (fire, bleeding, automatic fire) add up into one number
// instead of starting a new one every frame.
constexpr int kPopupMergeMs = 200;
// Safety cap on numbers drawn at once (oldest dropped), e.g. when an explosion hits a crowd.
constexpr size_t kMaxPopups = 40;
// A hit counts as the player's only this long after they last shot, fought or drove.
constexpr int kAttackWindowMs = 3000;
// Fully saturated so the numbers stand out against bright scenery.
constexpr Color kDamageHit  { 255, 235,   0, 255 };
constexpr Color kDamageKill { 255,  25,  25, 255 };
std::vector<Popup> g_popups;

struct PedHp { int hp; bool dead; };
std::unordered_map<Ped, PedHp> g_lastHp;
std::unordered_map<Ped, PedHp> g_hpNow; // kept between frames so its buckets are reused
int g_lastAttack = -100000;

} // namespace

void FinderTick(const Vec3& me) {
    if (!g.collectibles.enabled && !g.pickups.enabled) {
        g_found.clear();
        return;
    }
    ScanIfDue();

    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    int now = Invoke<int>(N::GET_GAME_TIMER);
    char buf[96];
    for (auto& f : g_found) {
        const bool collectible = f.type == kCollectible;
        const FinderStyle& style = collectible ? g.collectibles : g.pickups;
        if (!style.enabled || (!collectible && !g.pickupTypes[f.type])) continue;
        if (!Invoke<int>(N::DOES_ENTITY_EXIST, f.entity)) continue;
        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, f.entity, 1);
        float dist = Distance(me, pos);
        if (dist > g.finderRange) continue;

        // A tall beam so items are visible from a distance and through clutter. Through walls,
        // it's drawn in 2D from the same projection as the label: DRAW_LINE is hidden by walls,
        // and OverlayLine (which pulls it in front of the camera) jitters on lines this long.
        Color c = collectible ? kOrange : kPickupColors[f.type];
        Vec3 top = pos;
        top.z += 40.0f;
        if (style.beams) {
            if (style.beamsThroughWalls) ScreenLine(pos, top, WithAlpha(c, 0.7f));
            else                         Line3D(pos, top, WithAlpha(c, 0.6f));
        }

        if (!style.throughWalls && !InSight(f, player, now)) continue;

        // Label at the item if it's on screen, otherwise partway up the beam so hidden ones
        // still get one.
        float sx, sy;
        Vec3 labelAt = pos;
        if (!WorldToScreen(labelAt.x, labelAt.y, labelAt.z, sx, sy)) {
            if (!style.beams) continue;
            labelAt.z += (top.z - pos.z) * 0.4f;
            if (!WorldToScreen(labelAt.x, labelAt.y, labelAt.z, sx, sy)) continue;
        }
        Rect(sx, sy, 0.006f / AspectRatio(), 0.006f, c);
        if (dist >= 1000.0f) std::snprintf(buf, sizeof buf, "%s  %.1fkm", f.label.c_str(), dist / 1000.0f);
        else                 std::snprintf(buf, sizeof buf, "%s  %.0fm", f.label.c_str(), dist);
        Text(sx, sy + 0.006f, buf, c, 0.24f);
    }
}

void DamageNumbersTick(Ped player) {
    if (!g.damageNumbers) {
        g_lastHp.clear();
        g_hpNow.clear();
        g_popups.clear();
        return;
    }

    int now = Invoke<int>(N::GET_GAME_TIMER);
    Vehicle myVehicle = Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_IN, player, 1); // current or last
    bool driving = Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0) && myVehicle &&
                   Invoke<float>(N::GET_ENTITY_SPEED, myVehicle) > 3.0f;
    if (Invoke<int>(N::IS_PED_SHOOTING, player) || Invoke<int>(N::IS_PED_IN_MELEE_COMBAT, player) || driving)
        g_lastAttack = now;
    bool attacking = now >= g_lastAttack && now - g_lastAttack <= kAttackWindowMs;

    // The game's damage records are only read here: clearing them would break mission scripts
    // that count the player's kills through them. HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY stays true
    // once set, so a later hit only counts while the player is attacking; a kill counts when the
    // game names the player (or their vehicle) as the killer.
    g_hpNow.clear();
    for (int i = 0; i < g_pedCount; ++i) {
        Ped p = g_peds[i];
        if (p == player) continue;
        PedHp cur{ Invoke<int>(N::GET_ENTITY_HEALTH, p) + Invoke<int>(N::GET_PED_ARMOUR, p),
                   Invoke<int>(N::IS_ENTITY_DEAD, p, 0) != 0 };
        g_hpNow[p] = cur;

        auto it = g_lastHp.find(p);
        if (it == g_lastHp.end() || it->second.dead) continue; // unseen, or already dead
        int lost = it->second.hp - cur.hp;

        bool kill = false;
        if (cur.dead) {
            Entity killer = Invoke<Entity>(N::GET_PED_SOURCE_OF_DEATH, p);
            if (!killer || (killer != player && killer != myVehicle)) continue;
            kill = true;
        } else if (lost <= 0 || !attacking || !Invoke<int>(N::HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY, p, player, 1)) {
            continue;
        }
        if (!kill) {
            auto prev = std::find_if(g_popups.rbegin(), g_popups.rend(),
                                     [p](const Popup& q) { return q.ped == p; });
            if (prev != g_popups.rend() && !prev->kill && now >= prev->start &&
                now - prev->start < kPopupMergeMs) {
                prev->amount += lost;
                continue;
            }
        }
        Vec3 head = Invoke<Vec3>(N::GET_PED_BONE_COORDS, p, kBoneHead, 0.0f, 0.0f, 0.0f);
        g_popups.push_back({ p, head, lost > 0 ? lost : 0, kill, now });
    }
    g_lastHp.swap(g_hpNow);

    g_popups.erase(std::remove_if(g_popups.begin(), g_popups.end(), [now](const Popup& p) {
        return now < p.start || now - p.start > kPopupLifetimeMs;
    }), g_popups.end());
    if (g_popups.size() > kMaxPopups)
        g_popups.erase(g_popups.begin(), g_popups.end() - kMaxPopups);

    // Numbers float upward; fully opaque for the first half, then fade out.
    char buf[32];
    for (const Popup& pop : g_popups) {
        float t = (now - pop.start) / float(kPopupLifetimeMs);
        float sx, sy;
        if (!WorldToScreen(pop.pos.x, pop.pos.y, pop.pos.z + 0.3f + t * 0.8f, sx, sy)) continue;
        if (pop.kill && pop.amount > 0) std::snprintf(buf, sizeof buf, "-%d KILL", pop.amount);
        else if (pop.kill)              std::snprintf(buf, sizeof buf, "KILL");
        else                            std::snprintf(buf, sizeof buf, "-%d", pop.amount);
        float alpha = t < 0.5f ? 1.0f : 1.0f - (t - 0.5f) * 2.0f;
        Text(sx, sy, buf, WithAlpha(pop.kill ? kDamageKill : kDamageHit, alpha), pop.kill ? 0.5f : 0.42f);
    }
}

void TeleportToWaypoint() {
    // Find the map marker (blip type 4).
    bool found = false;
    Vec3 dest{};
    int it = Invoke<int>(N::GET_BLIP_INFO_ID_ITERATOR);
    for (Blip b = Invoke<Blip>(N::GET_FIRST_BLIP_INFO_ID, it); Invoke<int>(N::DOES_BLIP_EXIST, b);
         b = Invoke<Blip>(N::GET_NEXT_BLIP_INFO_ID, it)) {
        if (Invoke<int>(N::GET_BLIP_INFO_ID_TYPE, b) == 4) {
            dest = Invoke<Vec3>(N::GET_BLIP_INFO_ID_COORD, b);
            found = true;
            break;
        }
    }
    if (!found) {
        Notify("No waypoint set");
        return;
    }

    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    Entity e = Invoke<int>(N::IS_PED_IN_ANY_VEHICLE, player, 0) ? Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_USING, player) : player;

    // The ground under the marker isn't loaded yet; hop through heights so the map streams in,
    // then probe for ground at each.
    static const float kProbeHeights[] = { 100, 150, 50, 0, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800 };
    for (float h : kProbeHeights) {
        Invoke(N::SET_ENTITY_COORDS_NO_OFFSET, e, dest.x, dest.y, h, 0, 0, 1);
        scriptWait(100);
        float groundZ, waterZ;
        if (Invoke<int>(N::GET_GROUND_Z_FOR_3D_COORD, dest.x, dest.y, h, &groundZ, 0)) {
            // Over water the ground probe finds the seabed, so land on the surface instead.
            float z = groundZ;
            if (Invoke<int>(N::GET_WATER_HEIGHT, dest.x, dest.y, h, &waterZ) && waterZ > groundZ)
                z = waterZ;
            Invoke(N::SET_ENTITY_COORDS_NO_OFFSET, e, dest.x, dest.y, z + 1.0f, 0, 0, 1);
            return;
        }
    }

    // No ground found: drop in from the sky with a parachute. Out of the vehicle first, since a
    // parachute can't be opened from a seat.
    if (e != player) {
        Invoke(N::CLEAR_PED_TASKS_IMMEDIATELY, player);
        scriptWait(0);
    }
    Invoke(N::SET_ENTITY_COORDS_NO_OFFSET, player, dest.x, dest.y, 1000.0f, 0, 0, 1);
    Invoke(N::GIVE_DELAYED_WEAPON_TO_PED, player, Invoke<Hash>(N::GET_HASH_KEY, "GADGET_PARACHUTE"), 1, 0);
}

void SkipTime() {
    // ADD_TO_CLOCK_TIME rolls over midnight and advances the day itself.
    int hours = g.skipHours < 1 ? 1 : g.skipHours > 23 ? 23 : g.skipHours;
    Invoke(N::ADD_TO_CLOCK_TIME, hours, 0, 0);
    char msg[32];
    std::snprintf(msg, sizeof msg, "Time: %02d:%02d",
                  Invoke<int>(N::GET_CLOCK_HOURS), Invoke<int>(N::GET_CLOCK_MINUTES));
    Notify(msg);
}

const VehicleMake* FindVehicleMake(Hash model) {
    auto end = std::end(kVehicleMakes);
    auto it = std::lower_bound(std::begin(kVehicleMakes), end, static_cast<uint32_t>(model),
                               [](const VehicleMake& m, uint32_t h) { return m.model < h; });
    return it != end && it->model == static_cast<uint32_t>(model) ? it : nullptr;
}

const char* VehicleMakeName(Hash model) {
    const VehicleMake* m = FindVehicleMake(model);
    return m ? m->make : "";
}

const char* VehicleRealBrand(Hash model) {
    const VehicleMake* m = FindVehicleMake(model);
    return m ? m->real : "";
}

// ---- Best speeds, per vehicle model, kept in SPESP_speeds.ini ----

namespace {

std::unordered_map<Hash, float> g_bestSpeeds; // km/h
bool g_bestDirty = false;
ULONGLONG g_bestSavedAt = 0;

std::string SpeedsPath() {
    std::string p = IniPath();
    size_t dot = p.find_last_of('.');
    return (dot == std::string::npos ? p : p.substr(0, dot)) + "_speeds.ini";
}

// Whole file at once, through a temp file so a crash mid-write can't truncate it.
void WriteBestSpeeds() {
    g_bestDirty = false;
    g_bestSavedAt = GetTickCount64();
    std::vector<std::pair<Hash, float>> rows(g_bestSpeeds.begin(), g_bestSpeeds.end());
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    std::string out = "; SPESP best speeds in km/h, per vehicle model (fastest first). "
                      "Edit while the game is closed.\n[BestSpeed]\n";
    char line[160];
    for (const auto& [model, kmh] : rows) {
        if (!(kmh > 0.0f)) continue; // models only sat in
        std::string make = VehicleMakeName(model), name = VehicleModelName(model);
        std::snprintf(line, sizeof line, "0x%08X=%.1f ; %s%s%s\n", static_cast<unsigned>(model), kmh,
                      make.c_str(), make.empty() ? "" : " ", name.c_str());
        out += line;
    }
    std::string path = SpeedsPath(), tmp = path + ".tmp";
    FILE* f = std::fopen(tmp.c_str(), "wb");
    if (!f) return;
    bool ok = std::fwrite(out.data(), 1, out.size(), f) == out.size();
    ok = std::fclose(f) == 0 && ok;
    if (ok) ok = MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
    if (!ok) DeleteFileA(tmp.c_str());
}

} // namespace

void BestSpeedsLoad() {
    if (g_bestDirty) WriteBestSpeeds(); // don't lose a record set just before a reload
    g_bestSpeeds.clear();
    FILE* f = std::fopen(SpeedsPath().c_str(), "rb");
    if (!f) return;
    char line[256];
    while (std::fgets(line, sizeof line, f)) {
        // "0xHASH=km/h ; name": strtoul / strtof stop at '=' and at the comment.
        char* eq = std::strchr(line, '=');
        if (!eq || line[0] == ';' || line[0] == '[') continue;
        char* end = nullptr;
        unsigned long model = std::strtoul(line, &end, 16);
        if (end == line) continue;
        float kmh = std::strtof(eq + 1, &end);
        if (end == eq + 1 || !(kmh > 0.0f)) continue;
        g_bestSpeeds[static_cast<Hash>(model)] = kmh;
    }
    std::fclose(f);
}

void BestSpeedsClear() {
    g_bestSpeeds.clear();
    WriteBestSpeeds();
}

void SpeedometerTick(Ped player) {
    // The vehicle the player was last in, and its model's best speed (km/h).
    static Vehicle s_vehicle = 0;
    static Hash s_modelHash = 0;

    // New records are written at most every 2 s (not every frame while still accelerating),
    // and straight away when the player leaves or changes vehicle.
    auto flush = [] { if (g_bestDirty) WriteBestSpeeds(); };
    if (g_bestDirty && GetTickCount64() - g_bestSavedAt > 2000) flush();

    // Records are kept even while the panel is hidden.
    Vehicle veh = Invoke<BOOL>(N::IS_PED_IN_ANY_VEHICLE, player, 0)
                      ? Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_USING, player) : 0;
    if (!veh || !Invoke<BOOL>(N::DOES_ENTITY_EXIST, veh)) {
        flush();
        s_vehicle = 0;
        return;
    }

    constexpr float kKmh = 3.6f; // m/s -> km/h
    float speed = Invoke<float>(N::GET_ENTITY_SPEED, veh) * kKmh;
    // Name parts only change with the vehicle, so they're looked up once per vehicle.
    static const VehicleMake* s_make = nullptr;
    static char s_model[64] = "";
    static const char* s_class = "";
    if (veh != s_vehicle) {
        flush();
        s_vehicle = veh;
        Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, veh);
        s_modelHash = model;
        s_make = FindVehicleMake(model);
        std::snprintf(s_model, sizeof s_model, "%s", VehicleModelName(model));
        int cls = Invoke<int>(N::GET_VEHICLE_CLASS, veh);
        s_class = cls >= 0 && cls < kVehicleClassCount ? kVehicleClasses[cls] : "";
    }
    float& best = g_bestSpeeds[s_modelHash];
    if (speed > best + 0.05f) {
        if (!g_bestDirty) g_bestSavedAt = GetTickCount64(); // the 2 s count from the first new record
        best = speed;
        g_bestDirty = true;
    }
    if (!g.speedometer) return;
    const float s_best = best;
    float top = Invoke<float>(N::GET_VEHICLE_ESTIMATED_MAX_SPEED, veh) * kKmh;

    // GTA timer-bar style: one dark panel in the bottom-right corner (SpeedoX / SpeedoY):
    //   BENEFACTOR                 Panto     make (cyan) / model
    //   Mercedes-Benz           Compacts     real-life brand (green) / class (yellow)
    //   TOP 180                 BEST 150     stats, light grey
    //   SPEED                  123 KM/H      big number, orange at top speed
    //
    // Laid out by capital-letter height rather than text boxes, so the padding around the panel
    // and the gaps between rows look equal on screen. Sizes are screen-height fractions;
    // horizontal ones are divided by the aspect ratio to match in pixels.
    constexpr float kLabelScale = 0.23f, kValueScale = 0.25f, kSpeedScale = 0.42f;
    // Where font 0's capitals start and end below the y passed to Text, as screen-height
    // fractions: a constant plus a part proportional to the scale. Fitted from in-game
    // screenshots at scales 0.23 and 0.42 (2560x1440).
    auto capTop    = [](float scale) { return 0.0009f + 0.01453f * scale; };
    auto capBottom = [](float scale) { return 0.00219f + 0.05116f * scale; };
    auto capH      = [&](float scale) { return capBottom(scale) - capTop(scale); };
    constexpr float kPadY = 0.009f;   // panel edge to text, all four sides
    constexpr float kGapY = 0.009f;   // between rows, capital to capital
    constexpr float kMinW = 0.13f;
    constexpr Color kPanel     {   0,   0,   0, 165 };
    constexpr Color kLabel     { 225, 225, 225, 255 };
    constexpr Color kStatColor { 200, 200, 200, 255 };

    char make[48] = "", speedBuf[16], topBuf[24], bestBuf[24];
    if (s_make) {
        std::snprintf(make, sizeof make, "%s", s_make->make);
        for (char* c = make; *c; ++c) *c = static_cast<char>(toupper(static_cast<unsigned char>(*c)));
    }
    const char* real = s_make ? s_make->real : "";
    std::snprintf(speedBuf, sizeof speedBuf, "%.0f", speed);
    std::snprintf(topBuf, sizeof topBuf, "TOP %.0f", top);
    std::snprintf(bestBuf, sizeof bestBuf, "BEST %.0f", s_best);
    const char* unit = "KM/H";
    const bool infoRow = *real || *s_class;

    const float aspect = AspectRatio();
    const float padX = kPadY / aspect, gapX = 2.0f * kPadY / aspect, unitGap = 0.5f * kPadY / aspect;
    // Small rows are as tall as the larger of their two scales' capitals.
    const float rowCap = (std::max)(capH(kLabelScale), capH(kValueScale));
    const float speedCap = capH(kSpeedScale);

    auto rowW = [&](const char* l, float ls, const char* r, float rs) {
        return (*l ? TextWidth(l, ls) : 0.0f) + (*r ? TextWidth(r, rs) : 0.0f) + gapX;
    };
    const float speedW = TextWidth("SPEED", kLabelScale) + gapX +
                         TextWidth(speedBuf, kSpeedScale) + unitGap + TextWidth(unit, kLabelScale);
    const float w = (std::max)({ kMinW, rowW(make, kLabelScale, s_model, kValueScale) + 2 * padX,
                                 rowW(real, kLabelScale, s_class, kValueScale) + 2 * padX,
                                 rowW(topBuf, kLabelScale, bestBuf, kLabelScale) + 2 * padX,
                                 speedW + 2 * padX });
    const int rows = infoRow ? 3 : 2;
    const float h = 2 * kPadY + rows * (rowCap + kGapY) + speedCap;
    const float right = g.speedoX, bottom = g.speedoY, left = right - w, topY = bottom - h;
    Rect(left + w * 0.5f, topY + h * 0.5f, w, h, kPanel);

    // Draws text with the bottom of its capitals on `base`, so every size shares one baseline.
    auto draw = [&](float x, float base, const char* t, Color c, float scale) {
        Text(x, base - capBottom(scale), t, c, scale, false, 0, false);
    };
    const float lx = left + padX, rx = right - padX;
    auto row = [&](float base, const char* l, Color lc, const char* r, Color rc, float rs) {
        if (*l) draw(lx, base, l, lc, kLabelScale);
        if (*r) draw(rx - TextWidth(r, rs), base, r, rc, rs);
    };

    float base = topY + kPadY + rowCap;
    row(base, make, kCyan, s_model, kWhite, kValueScale);
    if (infoRow) {
        base += kGapY + rowCap;
        row(base, real, kGreen, s_class, kYellow, kValueScale);
    }
    base += kGapY + rowCap;
    row(base, topBuf, kStatColor, bestBuf, kStatColor, kLabelScale);

    // Speed row: label, number and unit share the number's baseline.
    base += kGapY + speedCap;
    Color sc = top > 0.0f && speed >= top ? kOrange : kWhite;
    float unitX = rx - TextWidth(unit, kLabelScale);
    float numX = unitX - unitGap - TextWidth(speedBuf, kSpeedScale);
    draw(lx, base, "SPEED", kLabel, kLabelScale);
    draw(numX, base, speedBuf, sc, kSpeedScale);
    draw(unitX, base, unit, kLabel, kLabelScale);
}
