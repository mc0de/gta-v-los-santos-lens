#include "features.h"
#include "vehicle_makes.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

// ---- Collectible / pickup finder ----

struct Found { Entity entity; const char* label; Color color; };

using ModelLabels = std::vector<std::pair<Hash, std::string>>;

// Labels in g_found point into these lists, so they're all rebuilt together on every scan.
// Rebuilding (rather than caching) also picks up "Reload settings file" changes.
ModelLabels g_collectibleHashes;
ModelLabels g_pickupHashes;
std::vector<Found> g_found;
int g_nextScan = 0;

void ResolveModelHashes(const std::vector<Collectible>& models, ModelLabels& out) {
    out.clear();
    for (const auto& c : models)
        out.push_back({ Invoke<Hash>(N::GET_HASH_KEY, c.model.c_str()), c.label });
}

const char* FindLabel(const ModelLabels& list, Hash model) {
    for (const auto& [hash, label] : list)
        if (hash == model) return label.c_str();
    return nullptr;
}

// The object pool can hold thousands of entries, so scan it twice a second rather than every frame.
void Scan() {
    g_found.clear();

    if (g.collectibles) {
        ResolveModelHashes(g.collectibleModels, g_collectibleHashes);
        static int objects[4096];
        int count = worldGetAllObjects(objects, 4096);
        for (int i = 0; i < count; ++i) {
            Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, objects[i]);
            if (const char* label = FindLabel(g_collectibleHashes, model))
                g_found.push_back({ objects[i], label, kOrange });
        }
    }

    if (g.pickups) {
        ResolveModelHashes(g.pickupModels, g_pickupHashes);
        // Letter scraps are pickups too, but they have their own display.
        Hash scrap = Invoke<Hash>(N::GET_HASH_KEY, "prop_ld_scrap");
        static int pickups[512];
        int count = worldGetAllPickups(pickups, 512);
        for (int i = 0; i < count; ++i) {
            Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, pickups[i]);
            if (model == scrap) continue;
            const char* label = WeaponLabelForModel(model);
            if (!label) label = FindLabel(g_pickupHashes, model);
            g_found.push_back({ pickups[i], label ? label : "Pickup", kCyan });
        }
    }
}

// ---- Letter scraps ----

// Only scraps the game has streamed in around the player exist as objects, so these are the
// nearest of the currently loaded ones.

Hash g_scrapModel = 0;
bool g_scrapModelValid = false;
bool g_scrapInit = false;
std::vector<Entity> g_scraps;
int g_nextScrapScan = 0;

void InitScrapModel() {
    if (g_scrapInit) return;
    g_scrapInit = true;
    g_scrapModel = Invoke<Hash>(N::GET_HASH_KEY, "prop_ld_scrap");
    g_scrapModelValid = Invoke<int>(N::IS_MODEL_VALID, g_scrapModel) != 0;
    if (!g_scrapModelValid)
        Notify("SPESP: letter scrap model not found");
}

// The object pool can hold thousands of entries, so scan it twice a second rather than every frame.
void ScanScraps() {
    static int objects[4096];
    int count = worldGetAllObjects(objects, 4096);
    g_scraps.clear();
    for (int i = 0; i < count; ++i) {
        if (Invoke<Hash>(N::GET_ENTITY_MODEL, objects[i]) == g_scrapModel)
            g_scraps.push_back(objects[i]);
    }
}

void DrawScrap(const Vec3& base, float d) {
    Vec3 top = base;
    top.z += 150.0f;
    // Drawn over terrain so the beam stays visible from a distance.
    OverlayLine(base, top, WithAlpha(kPink, 0.8f));

    char buf[48];
    if (d >= 1000.0f) std::snprintf(buf, sizeof buf, "Letter scrap  %.1fkm", d / 1000.0f);
    else              std::snprintf(buf, sizeof buf, "Letter scrap  %.0fm", d);

    // Label at the beam's base if visible, otherwise partway up so distant ones still get one.
    float sx, sy;
    Vec3 labelAt = base;
    if (!WorldToScreen(base.x, base.y, base.z, sx, sy)) {
        labelAt.z += 60.0f;
        if (!WorldToScreen(labelAt.x, labelAt.y, labelAt.z, sx, sy)) return;
    }
    Rect(sx, sy, 0.006f / AspectRatio(), 0.006f, kPink);
    Text(sx, sy + 0.006f, buf, kPink, 0.24f);
}

// ---- Damage numbers ----

struct Popup { Vec3 pos; int amount; bool kill; int start; };

constexpr int kPopupLifetimeMs = 1000;
// Fully saturated so the numbers stand out against bright scenery.
constexpr Color kDamageHit  { 255, 235,   0, 255 };
constexpr Color kDamageKill { 255,  25,  25, 255 };
std::vector<Popup> g_popups;
std::unordered_map<Ped, int> g_lastHp;

} // namespace

void FinderTick(const Vec3& me) {
    if (!g.collectibles && !g.pickups) {
        g_found.clear();
        return;
    }

    int now = Invoke<int>(N::GET_GAME_TIMER);
    if (now >= g_nextScan) {
        Scan();
        g_nextScan = now + 500;
    }

    char buf[96];
    for (const auto& f : g_found) {
        if (!Invoke<int>(N::DOES_ENTITY_EXIST, f.entity)) continue;
        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, f.entity, 1);
        float dist = Distance(me, pos);
        if (dist > g.finderRange) continue;

        // A tall beam so items are visible from a distance and through clutter.
        Vec3 top = pos;
        top.z += 40.0f;
        Line3D(pos, top, WithAlpha(f.color, 0.6f));

        float sx, sy;
        if (!WorldToScreen(pos.x, pos.y, pos.z, sx, sy)) continue;
        Rect(sx, sy, 0.006f / AspectRatio(), 0.006f, f.color);
        std::snprintf(buf, sizeof buf, "%s  %.0fm", f.label, dist);
        Text(sx, sy + 0.006f, buf, f.color, 0.24f);
    }
}

void LettersTick(const Vec3& me) {
    if (!g.letters) return;
    InitScrapModel();
    if (!g_scrapModelValid) return;

    int now = Invoke<int>(N::GET_GAME_TIMER);
    if (now >= g_nextScrapScan) {
        g_nextScrapScan = now + 500;
        ScanScraps();
    }

    // Nearest loaded scraps, ignoring the finder range.
    std::vector<std::pair<float, Vec3>> nearby;
    for (Entity e : g_scraps) {
        if (!Invoke<int>(N::DOES_ENTITY_EXIST, e)) continue;
        Vec3 pos = Invoke<Vec3>(N::GET_ENTITY_COORDS, e, 1);
        nearby.push_back({ Distance(me, pos), pos });
    }
    size_t n = (std::min)(nearby.size(), static_cast<size_t>((std::max)(g.letterCount, 0)));
    std::partial_sort(nearby.begin(), nearby.begin() + n, nearby.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
    for (size_t i = 0; i < n; ++i)
        DrawScrap(nearby[i].second, nearby[i].first);
}

void DamageNumbersTick(Ped player) {
    if (!g.damageNumbers) {
        g_lastHp.clear();
        g_popups.clear();
        return;
    }

    static int peds[512];
    int count = worldGetAllPeds(peds, 512);
    int now = Invoke<int>(N::GET_GAME_TIMER);

    std::unordered_map<Ped, int> hpNow;
    hpNow.reserve(count);
    for (int i = 0; i < count; ++i) {
        Ped p = peds[i];
        if (p == player) continue;
        int hp = Invoke<int>(N::GET_ENTITY_HEALTH, p) + Invoke<int>(N::GET_PED_ARMOUR, p);
        hpNow[p] = hp;

        // Health drop since last frame, on a ped the player has damaged. The game's damage
        // record is read-only here: clearing it would break mission scripts that count the
        // player's kills through it.
        auto it = g_lastHp.find(p);
        if (it == g_lastHp.end() || hp >= it->second) continue;
        if (!Invoke<int>(N::HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY, p, player, 1)) continue;

        Vec3 head = Invoke<Vec3>(N::GET_PED_BONE_COORDS, p, 31086, 0.0f, 0.0f, 0.0f);
        g_popups.push_back({ head, it->second - hp, Invoke<int>(N::IS_ENTITY_DEAD, p, 0) != 0, now });
    }
    g_lastHp.swap(hpNow);

    // Numbers float upward and fade out.
    char buf[32];
    for (size_t i = 0; i < g_popups.size();) {
        const Popup& pop = g_popups[i];
        int age = now - pop.start;
        if (age > kPopupLifetimeMs) {
            g_popups.erase(g_popups.begin() + i);
            continue;
        }
        float t = age / float(kPopupLifetimeMs);
        float sx, sy;
        if (WorldToScreen(pop.pos.x, pop.pos.y, pop.pos.z + 0.3f + t * 0.8f, sx, sy)) {
            std::snprintf(buf, sizeof buf, pop.kill ? "-%d KILL" : "-%d", pop.amount);
            // Fully opaque for the first half, then fade out.
            float alpha = t < 0.5f ? 1.0f : 1.0f - (t - 0.5f) * 2.0f;
            Text(sx, sy, buf, WithAlpha(pop.kill ? kDamageKill : kDamageHit, alpha), pop.kill ? 0.5f : 0.42f);
        }
        ++i;
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
        float groundZ;
        if (Invoke<int>(N::GET_GROUND_Z_FOR_3D_COORD, dest.x, dest.y, h, &groundZ, 0)) {
            Invoke(N::SET_ENTITY_COORDS_NO_OFFSET, e, dest.x, dest.y, groundZ + 1.0f, 0, 0, 1);
            return;
        }
    }

    // No ground found (e.g. open water): drop in from the sky with a parachute.
    Invoke(N::SET_ENTITY_COORDS_NO_OFFSET, e, dest.x, dest.y, 1000.0f, 0, 0, 1);
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

namespace {

// GET_VEHICLE_CLASS ids 0-22.
const char* const kVehicleClasses[] = {
    "Compacts", "Sedans", "SUVs", "Coupes", "Muscle", "Sports Classics", "Sports", "Super", "Motorcycles",
    "Off-road", "Industrial", "Utility", "Vans", "Cycles", "Boats", "Helicopters", "Planes", "Service",
    "Emergency", "Military", "Commercial", "Trains", "Open Wheel",
};

const VehicleMake* FindVehicleMake(Hash model) {
    auto end = std::end(kVehicleMakes);
    auto it = std::lower_bound(std::begin(kVehicleMakes), end, static_cast<uint32_t>(model),
                               [](const VehicleMake& m, uint32_t h) { return m.model < h; });
    return it != end && it->model == static_cast<uint32_t>(model) ? it : nullptr;
}

} // namespace

void SpeedometerTick(Ped player) {
    // Best speed reached, kept per vehicle and reset when you get into a different one.
    static Vehicle s_vehicle = 0;
    static float s_best = 0.0f;

    if (!g.speedometer || !Invoke<BOOL>(N::IS_PED_IN_ANY_VEHICLE, player, 0)) return;
    Vehicle veh = Invoke<Vehicle>(N::GET_VEHICLE_PED_IS_USING, player);
    if (!veh || !Invoke<BOOL>(N::DOES_ENTITY_EXIST, veh)) return;

    constexpr float kKmh = 3.6f; // m/s -> km/h
    float speed = Invoke<float>(N::GET_ENTITY_SPEED, veh) * kKmh;
    float top = Invoke<float>(N::GET_VEHICLE_ESTIMATED_MAX_SPEED, veh) * kKmh;
    // Name parts only change with the vehicle, so they're looked up once per vehicle.
    static const VehicleMake* s_make = nullptr;
    static char s_model[64] = "";
    static const char* s_class = "";
    if (veh != s_vehicle) {
        s_vehicle = veh;
        s_best = 0.0f;
        Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, veh);
        s_make = FindVehicleMake(model);
        const char* code = Invoke<const char*>(N::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL, model);
        const char* name = code ? Invoke<const char*>(N::GET_FILENAME_FOR_AUDIO_CONVERSATION, code) : nullptr;
        if (!name || !*name || !std::strcmp(name, "NULL")) name = code ? code : "";
        std::snprintf(s_model, sizeof s_model, "%s", name);
        int cls = Invoke<int>(N::GET_VEHICLE_CLASS, veh);
        s_class = cls >= 0 && cls < static_cast<int>(std::size(kVehicleClasses)) ? kVehicleClasses[cls] : "";
    }
    if (speed > s_best) s_best = speed;

    // Layout, all rows left-aligned except the stats:
    //   Benefactor Panto, Compacts      make cyan, model white, class yellow
    //   Mercedes-Benz                   real-life brand, green
    //   123 km/h              top 180   speed big; stats small, right-aligned
    //                        best 150
    // SpeedoX/Y is the box's bottom-right corner, so longer names grow to the left.
    constexpr float kNameScale = 0.24f, kRealScale = 0.22f, kSpeedScale = 0.38f, kStatScale = 0.21f;
    constexpr float kRow = 0.017f, kStatRow = 0.0145f, kSpeedH = 0.029f;
    constexpr float kPadX = 0.005f, kPadY = 0.003f, kSpeedGap = 0.012f;
    constexpr Color kStatColor { 215, 215, 215, 240 };

    char name[160], speedBuf[24], topBuf[24], bestBuf[24];
    const char* make = s_make ? s_make->make : "";
    const char* real = s_make ? s_make->real : "";
    std::snprintf(name, sizeof name, "%s%s%s%s%s", make, *make ? " " : "", s_model, *s_class ? ", " : "", s_class);
    std::snprintf(speedBuf, sizeof speedBuf, "%.0f km/h", speed);
    std::snprintf(topBuf, sizeof topBuf, "top %.0f", top);
    std::snprintf(bestBuf, sizeof bestBuf, "best %.0f", s_best);

    const float nameW = TextWidth(name, kNameScale);
    const float realW = *real ? TextWidth(real, kRealScale) : 0.0f;
    const float statW = (std::max)(TextWidth(topBuf, kStatScale), TextWidth(bestBuf, kStatScale));
    const float speedW = TextWidth(speedBuf, kSpeedScale);
    const float boxW = (std::max)({ nameW, realW, speedW + kSpeedGap + statW }) + 2 * kPadX;

    const float right = g.speedoX, bottom = g.speedoY;
    const float left = right - boxW;
    const float speedY = bottom - kPadY - kSpeedH;
    const float realY = speedY - (*real ? kRow : 0.0f);
    const float nameY = realY - kRow;
    const float boxTop = nameY - kPadY;
    Rect(left + boxW / 2, (boxTop + bottom) / 2, boxW, bottom - boxTop, { 0, 0, 0, 120 });

    // The name is one string with normal spacing. Each coloured part is drawn at the offset where
    // it starts in the full string: full width minus the width of the text from that part on.
    const float tx = left + kPadX;
    char modelPart[80];
    std::snprintf(modelPart, sizeof modelPart, "%s%s", s_model, *s_class ? "," : "");
    const char* fromModel = name + std::strlen(make) + (*make ? 1 : 0);
    if (*make) Text(tx, nameY, make, kCyan, kNameScale, false);
    Text(tx + nameW - TextWidth(fromModel, kNameScale), nameY, modelPart, kWhite, kNameScale, false);
    if (*s_class) Text(tx + nameW - TextWidth(s_class, kNameScale), nameY, s_class, kYellow, kNameScale, false);
    if (*real) Text(tx, realY, real, kGreen, kRealScale, false);

    Text(tx, speedY, speedBuf, top > 0.0f && speed >= top ? kOrange : kWhite, kSpeedScale, false);
    const float sx = right - kPadX;
    Text(sx - TextWidth(topBuf, kStatScale), speedY + 0.001f, topBuf, kStatColor, kStatScale, false);
    Text(sx - TextWidth(bestBuf, kStatScale), speedY + 0.001f + kStatRow, bestBuf, kStatColor, kStatScale, false);
}
