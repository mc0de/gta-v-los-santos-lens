// Single-player ESP for GTA V (ScriptHookV plugin).
// Default keys (configurable in SPESP.ini): Delete menu, F9 ESP, F10 pedestrians, F11 vehicles,
// Page Up auto-aim, Page Down aim point, End teleport to waypoint, Insert skip time, Numpad + garage.
// (F5-F8 are the game's character-switch keys.)

#include "features.h"
#include "menu.h"
#include <cstdio>
#include <cstring>

namespace {

HMODULE g_module = nullptr;

// Small status line near the top-left corner, shown while ESP or auto-aim is on.
// ESP categories that are on are listed by letter: H(ostiles) C(ops) P(edestrians) A(nimals) V(ehicles).
void DrawStatus() {
    if (!g.esp && !g.aim) return;

    char cats[48] = "";
    if (g.esp) {
        // A vehicle type filter is shown after the V, e.g. "V:Super" or "V:3 types".
        const char* types = VehicleTypesSummary();
        bool filtered = g.vehicles.enabled && std::strcmp(types, "All") != 0;
        std::snprintf(cats, sizeof cats, "%s%s%s%s%s%s%s",
                      g.hostiles.enabled ? "H" : "", g.cops.enabled ? "C" : "",
                      g.pedestrians.enabled ? "P" : "", g.animals.enabled ? "A" : "",
                      g.vehicles.enabled ? "V" : "",
                      filtered ? ":" : "", filtered ? types : "");
    }

    char status[64];
    std::snprintf(status, sizeof status, "%s%s%s%s",
                  g.esp ? "ESP " : "",
                  cats,
                  g.esp && g.aim ? " | " : "",
                  !g.aim ? "" : g.aimBone == AimBone::Head ? "AIM HEAD" : "AIM BODY");
    Text(g.statusX, g.statusY, status, kWhite, 0.2f, false);
}

void HandleHotkeys() {
    const Keys& k = g.keys;
    if (KeyPressed(k.menu))              MenuToggle();
    if (KeyPressed(k.toggleEsp))         g.esp = !g.esp;
    if (KeyPressed(k.togglePedestrians)) g.pedestrians.enabled = !g.pedestrians.enabled;
    if (KeyPressed(k.toggleVehicles))    g.vehicles.enabled = !g.vehicles.enabled;
    if (KeyPressed(k.toggleAim))         { g.aim = !g.aim; g_aimTarget = 0; }
    if (KeyPressed(k.aimBone))       g.aimBone = g.aimBone == AimBone::Head ? AimBone::Body : AimBone::Head;
    if (KeyPressed(k.teleport))      TeleportToWaypoint();
    if (KeyPressed(k.skipTime))      SkipTime();
    if (KeyPressed(k.garage))        MenuOpenGarage();
}

void Tick() {
    if (Invoke<int>(N::IS_PAUSE_MENU_ACTIVE)) return;

    HandleHotkeys();
    BeginFrame(); // after hotkeys: teleport waits a few frames

    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    Vec3 me = Invoke<Vec3>(N::GET_ENTITY_COORDS, player, 1);

    if (g.aim) AimTick(player, me);
    else       g_aimTarget = 0;

    if (g.esp) EspTick(player, me);
    FinderTick(me);
    LettersTick(me);
    DamageNumbersTick(player);
    SpeedometerTick(player);
    GarageTick(player);

    MenuTick();
    DrawStatus();
}

void ScriptMain() {
    LoadSettings(g_module);
    for (;;) {
        Tick();
        scriptWait(0);
    }
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            g_module = module;
            scriptRegister(module, ScriptMain);
            break;
        case DLL_PROCESS_DETACH:
            scriptUnregister(module);
            break;
    }
    return TRUE;
}
