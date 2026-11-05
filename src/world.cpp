#include "features.h"
#include "vehicle_makes.h"
#include <algorithm>
#include <cctype>
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

// ---- Letter scraps ----

// Only scraps the game has streamed in around the player exist as objects, so these are the
// nearest of the currently loaded ones.

Hash g_scrapModel = 0;
bool g_scrapModelValid = false;
bool g_scrapWarned = false;
std::vector<Entity> g_scraps;

void InitScrapModel() {
    if (g_scrapModel) return;
    g_scrapModel = Invoke<Hash>(N::GET_HASH_KEY, "prop_ld_scrap");
    g_scrapModelValid = Invoke<int>(N::IS_MODEL_VALID, g_scrapModel) != 0;
}

// One pass over the object and pickup pools for the finder and the letter scraps. The object
// pool can hold thousands of entries, so this runs twice a second rather than every frame.
void Scan() {
    g_found.clear();
    g_scraps.clear();
    InitScrapModel();

    bool wantScraps = g.letters && g_scrapModelValid;
    if (g.collectibles || wantScraps) {
        if (g.collectibles) ResolveModelHashes(g.collectibleModels, g_collectibleHashes);
        static int objects[4096];
        int count = worldGetAllObjects(objects, 4096);
        for (int i = 0; i < count; ++i) {
            Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, objects[i]);
            if (wantScraps && model == g_scrapModel) {
                g_scraps.push_back(objects[i]);
            } else if (g.collectibles) {
                if (const char* label = FindLabel(g_collectibleHashes, model))
                    g_found.push_back({ objects[i], label, kOrange });
            }
        }
    }

    if (g.pickups) {
        ResolveModelHashes(g.pickupModels, g_pickupHashes);
        static int pickups[512];
        int count = worldGetAllPickups(pickups, 512);
        for (int i = 0; i < count; ++i) {
            Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, pickups[i]);
            // Letter scraps are pickups too, but they have their own display.
            if (model == g_scrapModel) continue;
            const char* label = WeaponLabelForModel(model);
            if (!label) label = FindLabel(g_pickupHashes, model);
            g_found.push_back({ pickups[i], label ? label : "Pickup", kCyan });
        }
    }
}

void ScanIfDue() {
    int now = Invoke<int>(N::GET_GAME_TIMER);
    if (now >= g_nextScan || now < g_nextScan - 1000) {
        Scan();
        g_nextScan = now + 500;
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

// Shown as a short list at a fixed spot next to the crosshair (DamageX / DamageY), newest at the
// bottom, so they stay readable however fast targets move.
struct Popup { int amount; bool kill; int start; };

constexpr int kPopupLifetimeMs = 1500;
constexpr size_t kMaxPopups = 6;
// A hit counts as the player's only this long after they last shot, fought or drove.
constexpr int kAttackWindowMs = 3000;
// Fully saturated so the numbers stand out against bright scenery.
constexpr Color kDamageHit  { 255, 235,   0, 255 };
constexpr Color kDamageKill { 255,  25,  25, 255 };
std::vector<Popup> g_popups;

struct PedHp { int hp; bool dead; };
std::unordered_map<Ped, PedHp> g_lastHp;
int g_lastAttack = -100000;

} // namespace

// Also runs the shared scan for LettersTick, which comes after it each frame.
void FinderTick(const Vec3& me) {
    if (!g.collectibles && !g.pickups && !g.letters) {
        g_found.clear();
        g_scraps.clear();
        return;
    }
    ScanIfDue();

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
    if (!g_scrapModelValid) {
        if (!g_scrapWarned) Notify("SPESP: letter scrap model not found");
        g_scrapWarned = true;
        return;
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
    std::unordered_map<Ped, PedHp> hpNow;
    hpNow.reserve(g_pedCount);
    for (int i = 0; i < g_pedCount; ++i) {
        Ped p = g_peds[i];
        if (p == player) continue;
        PedHp cur{ Invoke<int>(N::GET_ENTITY_HEALTH, p) + Invoke<int>(N::GET_PED_ARMOUR, p),
                   Invoke<int>(N::IS_ENTITY_DEAD, p, 0) != 0 };
        hpNow[p] = cur;

        auto it = g_lastHp.find(p);
        if (it == g_lastHp.end() || it->second.dead) continue; // unseen, or already dead
        int lost = it->second.hp - cur.hp;

        if (cur.dead) {
            Entity killer = Invoke<Entity>(N::GET_PED_SOURCE_OF_DEATH, p);
            if (killer && (killer == player || killer == myVehicle))
                g_popups.push_back({ lost > 0 ? lost : 0, true, now });
        } else if (lost > 0 && attacking && Invoke<int>(N::HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY, p, player, 1)) {
            g_popups.push_back({ lost, false, now });
        }
    }
    g_lastHp.swap(hpNow);

    g_popups.erase(std::remove_if(g_popups.begin(), g_popups.end(), [now](const Popup& p) {
        return now < p.start || now - p.start > kPopupLifetimeMs;
    }), g_popups.end());
    if (g_popups.size() > kMaxPopups)
        g_popups.erase(g_popups.begin(), g_popups.end() - kMaxPopups);

    // Newest at the bottom; each one pops in slightly larger, then fades out in its second half.
    constexpr float kLineH = 0.03f;
    char buf[32];
    float y = g.damageY;
    for (size_t i = g_popups.size(); i-- > 0;) {
        const Popup& pop = g_popups[i];
        float t = (now - pop.start) / float(kPopupLifetimeMs);
        float alpha = t < 0.5f ? 1.0f : 1.0f - (t - 0.5f) * 2.0f;
        float grow = t < 0.08f ? 1.0f + (0.08f - t) * 4.0f : 1.0f;
        if (pop.kill && pop.amount > 0) std::snprintf(buf, sizeof buf, "-%d KILL", pop.amount);
        else if (pop.kill)              std::snprintf(buf, sizeof buf, "KILL");
        else                            std::snprintf(buf, sizeof buf, "-%d", pop.amount);
        float scale = (pop.kill ? 0.5f : 0.42f) * grow;
        Text(g.damageX, y, buf, WithAlpha(pop.kill ? kDamageKill : kDamageHit, alpha), scale, false);
        y -= kLineH;
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

namespace {

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
        std::snprintf(s_model, sizeof s_model, "%s", VehicleModelName(model));
        int cls = Invoke<int>(N::GET_VEHICLE_CLASS, veh);
        s_class = cls >= 0 && cls < kVehicleClassCount ? kVehicleClasses[cls] : "";
    }
    if (speed > s_best) s_best = speed;

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
