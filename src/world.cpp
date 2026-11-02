#include "features.h"
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

// ---- Collectible / pickup finder ----

struct Found { Entity entity; const char* label; Color color; };

// Labels in g_found point into g_modelHashes, so both are rebuilt together on every scan.
// Rebuilding (rather than caching) also picks up "Reload settings file" changes.
std::vector<std::pair<Hash, std::string>> g_modelHashes;
std::vector<Found> g_found;
int g_nextScan = 0;

void ResolveModelHashes() {
    g_modelHashes.clear();
    for (const auto& c : g.collectibleModels)
        g_modelHashes.push_back({ Invoke<Hash>(N::GET_HASH_KEY, c.model.c_str()), c.label });
}

// The object pool can hold thousands of entries, so scan it twice a second rather than every frame.
void Scan() {
    g_found.clear();

    if (g.collectibles) {
        ResolveModelHashes();
        static int objects[4096];
        int count = worldGetAllObjects(objects, 4096);
        for (int i = 0; i < count; ++i) {
            Hash model = Invoke<Hash>(N::GET_ENTITY_MODEL, objects[i]);
            for (const auto& [hash, label] : g_modelHashes) {
                if (hash == model) {
                    g_found.push_back({ objects[i], label.c_str(), kCyan });
                    break;
                }
            }
        }
    }

    if (g.pickups) {
        static int pickups[512];
        int count = worldGetAllPickups(pickups, 512);
        for (int i = 0; i < count; ++i)
            g_found.push_back({ pickups[i], "Pickup", kOrange });
    }
}

// ---- Damage numbers ----

struct Popup { Vec3 pos; int amount; bool kill; int start; };

constexpr int kPopupLifetimeMs = 1000;
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
            Text(sx, sy, buf, WithAlpha(pop.kill ? kRed : kYellow, 1.0f - t), pop.kill ? 0.42f : 0.36f);
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
