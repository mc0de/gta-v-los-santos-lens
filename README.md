# SPESP â€” Single-player ESP for GTA V

A [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/) plugin with ESP overlays, auto-aim and a few utilities for GTA V **Story Mode**.

> Story Mode only. Do not use with GTA Online. ScriptHookV refuses to run online, and with BattlEye enabled any modded files risk getting your account flagged.

## Features

### ESP
- **Peds**: 2D boxes, optional **skeletons** (visible through walls and models), a small health bar above the head, distance, and labels for weapon held and COP / SWAT / ARMY. Weapon labels can be turned off separately (`Weapon labels` in the menu). Animals can be hidden (on by default). Colour shows disposition:
  | Colour | Meaning |
  |---|---|
  | Red | In combat with you, or hostile (hate / dislike) |
  | Green | Friendly (companion / respect / like) |
  | Blue | Cop, SWAT, army |
  | White | Neutral |
  | Grey | Dead |
  | Magenta | Current auto-aim target |
- **Vehicles**: box with name and distance, plus `emergency`, `occupied` and `locked` tags. Emergency vehicles are blue, others yellow, wrecks grey.
- **Threat arrows**: red arrows around the screen centre pointing at hostiles you can't see on screen.
- **Distance fade**: far targets are drawn fainter.
- **Tracers** (optional): 3D lines from you to each target.

### Auto-aim
While you hold aim (right mouse), the camera locks onto the nearest living ped you have clear line of sight to. Peds behind walls, vehicles or objects are skipped. The lock stays on one target until it dies or goes out of sight.

- **Aim at**: head or upper chest.
- **Priority**: nearest, hostiles first, or hostiles only.
- **Ignore cops / animals**.
- **Smoothing**: 1 snaps instantly; higher values glide the camera onto the target.
- **Aim zone**: only targets within N degrees of the crosshair are considered ("any" = no limit). Optionally drawn as a circle.

### Finder
- **Collectibles**: cyan markers and a tall beam on collectible props (letter scraps, spaceship parts, peyote plants, nuclear waste). The model list lives in the `[Collectibles]` section of `SPESP.ini`. The default model names are best-known values and haven't all been verified; fix or add entries there. Objects only exist near the player, so this finds items in your surroundings, not across the whole map.
- **Pickups**: orange markers on weapon / health / armour pickups.

### Misc
- **Damage numbers**: floating numbers when you damage a ped, red with `KILL` on a kill.
- **Teleport to waypoint**: set a map marker and press End (or use the menu). In a vehicle, the vehicle comes with you. If no ground is found (e.g. open sea) you're dropped from the sky with a parachute.

## Menu and hotkeys

Press **Delete** to open the menu. Arrow keys move / change values, Enter toggles or runs an action, Backspace or Delete closes it. (Home isn't used because it opens the Rockstar Games Launcher overlay.) Settings are saved to `SPESP.ini` when the menu closes.

| Key | Action |
|---|---|
| Delete | Open / close menu |
| F9 | Toggle ESP |
| F10 | Cycle ESP targets: peds â†’ vehicles â†’ both |
| F11 | Toggle tracers |
| Page Up | Toggle auto-aim |
| Page Down | Switch aim point: head / body |
| End | Teleport to waypoint |

F5â€“F8 are left alone because the game uses them for character switching.

A small status line near the top-left corner shows what's on, e.g. `ESP P+V TR | AIM HEAD` (P = peds, V = vehicles, TR = tracers).

## Configuration (`SPESP.ini`)

Created next to `SPESP.asi` with defaults on first run. Edit it while the game runs and use **Reload settings file** in the menu to apply.

- `[Keys]`: key names like `F9`, `PageUp`, `Delete`, `End`, `Insert`, `Numpad5`, single letters/digits, or a hex virtual-key code like `0x7A`. `None` disables a hotkey.
- `[ESP]`, `[Aim]`, `[Finder]`, `[Misc]`: everything in the menu, plus `StatusX` / `StatusY` for the status line position (0â€“1 screen fractions).
- `[Collectibles]`: `model_name=Label` lines.

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

1. Download the ScriptHookV **runtime** (not the SDK) from dev-c.com. Make sure the build supports your game version / edition.
2. Copy into the GTA V folder (next to `GTA5.exe` / `GTA5_Enhanced.exe`):
   - `ScriptHookV.dll`
   - an ASI loader as `dinput8.dll`. The loader bundled with ScriptHookV only works with Legacy (`GTA5.exe`); for Enhanced use one that supports it, such as [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).
   - `SPESP.asi`
3. Launch with **BattlEye disabled** (Rockstar Games Launcher â†’ Settings â†’ your GTA V edition) and load Story Mode.

To uninstall or before playing GTA Online, remove `dinput8.dll` (stops all ASI plugins loading) or all the files above.

## Code layout

| File | Purpose |
|---|---|
| `src/invoke.h` | Native invoker over the SDK's `nativeInit` / `nativePush64` / `nativeCall`, and native hashes (each checked against the SDK's `natives.h`) |
| `src/ui.*` | Drawing helpers (text, rects, sprites, 3D lines, bounding boxes), notifications, keyboard input |
| `src/settings.*` | `Settings` struct and `SPESP.ini` load / save |
| `src/esp.cpp` | Ped / vehicle ESP, skeletons, labels, threat arrows |
| `src/aim.cpp` | Auto-aim target selection, line of sight, smoothing, aim zone |
| `src/world.cpp` | Collectible / pickup finder, damage numbers, teleport |
| `src/menu.*` | In-game menu |
| `src/main.cpp` | Plugin entry point, hotkeys, status line |
