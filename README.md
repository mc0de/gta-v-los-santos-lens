# SPESP — Single-player ESP for GTA V

A [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/) plugin that draws ESP overlays for peds and vehicles in GTA V **Story Mode**.

> Story Mode only. Do not use with GTA Online. ScriptHookV refuses to run online, and with BattlEye enabled any modded files risk getting your account flagged.

## Features

- **Peds**: 2D bounding box, health bar, distance. Box colour shows disposition:
  | Colour | Meaning |
  |---|---|
  | Red | In combat with you, or hostile (hate / dislike) |
  | Green | Friendly (companion / respect / like) |
  | Blue | Cop, SWAT, army |
  | White | Neutral |
  | Grey | Dead |
- **Vehicles**: yellow box with localized model name and distance (grey when destroyed).
- **Tracers** (optional): 3D lines from you to each target.
- **Auto-aim** (optional): while you hold aim (right mouse), the camera locks onto the head or chest (switchable) of the nearest living human you have clear line of sight to. Animals are ignored (`aimIgnoreAnimals` in `Settings`). Peds behind walls, vehicles or objects are skipped. The lock stays on one target until it dies or goes out of sight, then moves to the next nearest. The locked target's ESP box turns magenta.
- Range limits: 250 m for peds, 350 m for vehicles (see `Settings` in `src/main.cpp`).

## Hotkeys

| Key | Action |
|---|---|
| F9 | Toggle ESP |
| F10 | Cycle targets: peds → vehicles → both |
| F11 | Toggle tracers |
| Page Up | Toggle auto-aim |
| Page Down | Switch aim point: head / body |

F5–F8 are left alone because the game uses them for character switching. Change the keys via `kKeyToggle`, `kKeyTargets`, `kKeyTracers`, `kKeyAim`, `kKeyAimBone` in `src/main.cpp`.

A small status line near the top-left corner shows what's on, e.g. `ESP P+V TR | AIM HEAD` (P = peds, V = vehicles, TR = tracers). It's offset right to clear FPS overlays; adjust `kStatusX` / `kStatusY` to move it.

## Building

Requirements:

- Visual Studio 2022 or Build Tools 2022 with the **Desktop development with C++** workload (includes CMake)
- ScriptHookV SDK from [dev-c.com](http://www.dev-c.com/gtav/scripthookv/), extracted into `sdk/` so that these exist:
  ```
  sdk/inc/main.h
  sdk/inc/types.h
  sdk/lib/ScriptHookV.lib
  ```

Build:

```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Output: `build/Release/SPESP.asi`. The CRT is linked statically, so the only runtime dependency is `ScriptHookV.dll`.

## Installing

1. Download the ScriptHookV **runtime** (not the SDK) from dev-c.com. Make sure the build supports your game version / edition (Legacy or Enhanced).
2. Copy into the GTA V folder (next to `GTA5.exe` / `GTA5_Enhanced.exe`):
   - `ScriptHookV.dll`
   - `dinput8.dll` (ASI loader)
   - `SPESP.asi`
3. Launch with **BattlEye disabled** (Rockstar Games Launcher → Settings → your GTA V edition) and load Story Mode.

To uninstall or before playing GTA Online, remove `dinput8.dll` (stops all ASI plugins loading) or all three files.

## How it works

- `src/invoke.h`: a minimal native invoker over the SDK's `nativeInit` / `nativePush64` / `nativeCall` exports. Natives are called by hash, so the code doesn't depend on a particular `natives.h` naming revision.
- `src/main.cpp`: each frame, enumerates peds and vehicles with `worldGetAllPeds` / `worldGetAllVehicles`, projects the 8 corners of each entity's model bounding box to screen space, and draws the enclosing rectangle with `DRAW_RECT` plus text labels.

The plugin only reads game state and draws overlays; it doesn't modify entities or use cheat codes.
