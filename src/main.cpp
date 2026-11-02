// Single-player ESP for GTA V (ScriptHookV plugin).
// Default keys (configurable in SPESP.ini): Delete menu, F9 ESP, F10 ESP targets, F11 tracers,
// Page Up auto-aim, Page Down aim point, End teleport to waypoint.
// (F5-F8 are the game's character-switch keys.)

#include "features.h"
#include "menu.h"
#include <cstdio>

namespace {

HMODULE g_module = nullptr;

const char* TargetsName(Targets t) {
    switch (t) {
        case Targets::Peds:     return "P";
        case Targets::Vehicles: return "V";
        default:                return "P+V";
    }
}

// Small status line near the top-left corner, shown while ESP or auto-aim is on.
void DrawStatus() {
    if (!g.esp && !g.aim) return;

    char status[64];
    std::snprintf(status, sizeof status, "%s%s%s%s%s",
                  g.esp ? "ESP " : "",
                  g.esp ? TargetsName(g.targets) : "",
                  g.esp && g.tracers ? " TR" : "",
                  g.esp && g.aim ? " | " : "",
                  !g.aim ? "" : g.aimBone == AimBone::Head ? "AIM HEAD" : "AIM BODY");
    Text(g.statusX, g.statusY, status, kWhite, 0.2f, false);
}

void HandleHotkeys() {
    const Keys& k = g.keys;
    if (KeyPressed(k.menu))          MenuToggle();
    if (KeyPressed(k.toggleEsp))     g.esp = !g.esp;
    if (KeyPressed(k.cycleTargets))  g.targets = static_cast<Targets>((static_cast<int>(g.targets) + 1) % 3);
    if (KeyPressed(k.toggleTracers)) g.tracers = !g.tracers;
    if (KeyPressed(k.toggleAim))     { g.aim = !g.aim; g_aimTarget = 0; }
    if (KeyPressed(k.aimBone))       g.aimBone = g.aimBone == AimBone::Head ? AimBone::Body : AimBone::Head;
    if (KeyPressed(k.teleport))      TeleportToWaypoint();
}

void Tick() {
    if (Invoke<int>(N::IS_PAUSE_MENU_ACTIVE)) return;

    HandleHotkeys();

    Ped player = Invoke<Ped>(N::PLAYER_PED_ID);
    Vec3 me = Invoke<Vec3>(N::GET_ENTITY_COORDS, player, 1);

    if (g.aim) AimTick(player, me);
    else       g_aimTarget = 0;

    if (g.esp) EspTick(player, me);
    FinderTick(me);
    DamageNumbersTick(player);

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
