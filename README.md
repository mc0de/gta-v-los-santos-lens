# SPESP — Single-player ESP for GTA V

A [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/) plugin with ESP overlays, auto-aim and a few utilities for GTA V **Story Mode**.

> Story Mode only. Do not use with GTA Online. ScriptHookV refuses to run online, and with BattlEye enabled any modded files risk getting your account flagged.

## Features

### ESP
Targets are sorted into categories, and for each category you choose what to draw. In the menu this is a grid: one row per category, one column per element.

| Category | Who | Colour |
|---|---|---|
| Animals | All animals | Tan; while attacking you, same as hostiles |
| Hostiles | People fighting you or hostile toward you (hate / dislike), including cops chasing you | **Green** if you have a clear line of sight (you can shoot them), **red** if they're behind cover |
| Cops | Cops, SWAT, army that aren't after you | Blue |
| Pedestrians | Everyone else | Light purple if friendly, white if neutral |
| Vehicles | All vehicles except the one you're in | Blue for emergency, yellow otherwise |

| Column | What it draws |
|---|---|
| On | Show this category at all |
| Box | 2D bounding box |
| HP | Small health bar above the box |
| Dist | Distance below the box |
| Info | Peds: COP / SWAT / ARMY and the weapon held. Vehicles: name plus `emergency` and `locked` tags, and a small dot after the name when someone is driving |
| Line | Tracer from you to the target, visible through walls |

Defaults: hostiles and cops get everything except tracers, pedestrians and animals get box and distance, and animals are off.

Other ESP options:
- **Show dead**: also draw dead peds and wrecked vehicles (grey). Off by default.
- **Threat arrows**: red arrows around the screen centre pointing at hostiles you can't see on screen.
- **Distance fade**: far targets are drawn fainter.
- **Vehicle types**: a checklist of vehicle classes (Super, Sports, Muscle, Motorcycles, Helicopters, ...) for finding a certain kind of vehicle quickly. Only checked classes are drawn. All are checked by default. Press Enter on the item to open the list, which also has **Show all** and **Show none**, and Backspace to go back. While some classes are unchecked, the status line shows the filter after the V, e.g. `V:Super` or `V:3 types`.
- The current auto-aim target is always drawn, in magenta, even if its category is off.

### Auto-aim
While you hold aim (right mouse), the camera locks onto a living ped you have clear line of sight to. Peds behind walls, vehicles or objects are skipped. The lock stays on one target until it dies or goes out of sight; to switch, let go of aim and aim again. Auto-aim stays off while you hold a throwable (grenade, sticky bomb, molotov, etc.) so it doesn't spoil the throw arc.

**Who is targeted.** Hostiles (anyone fighting you, including cops chasing you and animals attacking you) are always targets. The toggles add the peaceful ones:
- **Target cops**: cops, SWAT and army that aren't after you. Off by default.
- **Target pedestrians**: off by default.
- **Target animals**: off by default.

**How the target is picked.**
- **Pick**: **Crosshair** (default) locks onto the target closest to where you're aiming, so you choose by pointing at it. **Nearest** locks onto the closest by distance, preferring hostiles over peaceful targets; a peaceful lock switches to a hostile as soon as one is in sight.
- **Range**: only peds within this distance are targeted (250 m by default, separate from the ESP range).
- **Aim zone**: only targets within N degrees of the crosshair are considered ("any" = no limit).

Other options:
- **Aim at**: head or upper chest.
- **Smoothing**: 1 snaps instantly; higher values glide the camera onto the target.

### Finder
- **Collectibles**: orange markers and a tall beam on collectible props (spaceship parts, peyote plants, nuclear waste). The model list lives in the `[Collectibles]` section of `SPESP.ini`. The default model names are best-known values and haven't all been verified; fix or add entries there. Objects only exist near the player, so this finds items in your surroundings, not across the whole map.
- **Pickups**: cyan markers on weapon / health / armour pickups, labelled with what they are. Weapon pickups are named automatically, from the same weapon list the ESP uses. Other pickups are named from the `[Pickups]` section of `SPESP.ini`. Its default model names (health, armour, ammo, parachute) are best-known values and haven't been verified, so fix or add entries there. Anything unrecognised shows as `Pickup`. Letter scraps aren't shown here, because they have their own display.
- **Letter scraps**: pink beams (visible through terrain) on the nearest letter scraps, 3 by default (`Letters shown` in the menu), ignoring the finder range. Only scraps the game has loaded around you can be seen, so this shows the nearest ones in your surroundings, not across the whole map. Collected scraps disappear from the world and so from the display. If the scrap model (`prop_ld_scrap`) doesn't exist in the game you get a notification.

### Misc
- **Damage numbers**: a short list just right of the crosshair, so they stay readable while everything moves: bright yellow `-N` when you damage a ped, red `-N KILL` when you kill one. Newest at the bottom; each lasts 1.5 s, up to 6 at once. Hits count while you're shooting, fighting or driving (and for 3 s after) on peds you've damaged; kills count when the game names you or your vehicle as the killer. Position via `DamageX` / `DamageY` in the ini (default 0.53 / 0.47).
- **Speedometer**: while you're in a vehicle, a compact GTA-style panel in the bottom-right area (one dark background, rows with labels on the left and values on the right, like the game's race timers), from top to bottom: make in cyan and model in white (`BENEFACTOR` / `Panto`); the real-life brand the make parodies in green and the class in yellow (`Mercedes-Benz` / `Compacts`; a few brands are best guesses, e.g. Nagasaki as Kawasaki); the rated top speed (`TOP`, the game's estimate, which includes upgrades) and the fastest you've gone in this vehicle (`BEST`, reset when you change vehicle); and the current speed as a big number in km/h, orange once you reach the rated top speed. On by default; position via `SpeedoX` / `SpeedoY` in the ini, which set the panel's bottom-right corner (default 0.86 / 0.985), so longer names grow to the left. Makes come from a table of ~765 models in `src/vehicle_makes.h`. Vehicles with no make (police, trains, ...) or newer than the table show only model and class.
- **Teleport to waypoint**: set a map marker and press End (or use the menu). In a vehicle, the vehicle comes with you. Over water you land on the surface, not the seabed. If no ground is found you're taken out of any vehicle and dropped from the sky with a parachute.
- **Skip time**: press Insert (or use the menu) to move the game clock forward 6 hours. The amount (1–23 hours) is set by **Hours to skip** in the menu. A notification shows the new time.

## Menu and hotkeys

Press **Delete** to open the menu. It's laid out in two columns (ESP and Finder on the left, Auto-aim and Misc on the right); Up/Down runs down the left column and on into the right one. Arrow keys move / change values (in the ESP grid, Left/Right move between cells), Enter toggles or runs an action, Backspace or Delete closes it. (Home isn't used because it opens the Rockstar Games Launcher overlay.) Settings are saved to `SPESP.ini` when the menu closes.

| Key | Action |
|---|---|
| Delete | Open / close menu |
| F9 | Toggle ESP |
| F10 | Toggle pedestrians (quickly declutter busy streets) |
| F11 | Toggle vehicles |
| Page Up | Toggle auto-aim |
| Page Down | Switch aim point: head / body |
| End | Teleport to waypoint |
| Insert | Skip game time forward (6 hours by default) |

F5–F8 are left alone because the game uses them for character switching.

A small status line near the top-left corner shows what's on, e.g. `ESP HCPV | AIM HEAD`, listing the ESP categories that are on: H = hostiles, C = cops, P = pedestrians, A = animals, V = vehicles (plus `:Type` when a vehicle type filter is set).

## Configuration (`SPESP.ini`)

Created next to `SPESP.asi` with defaults on first run. Edit it while the game runs and use **Reload settings file** in the menu to apply.

- `[Keys]`: key names like `F9`, `PageUp`, `Delete`, `End`, `Insert`, `Numpad5`, single letters/digits, or a hex virtual-key code like `0x7A`. `None` disables a hotkey.
- `[ESP]`, `[Aim]`, `[Finder]`, `[Misc]`: everything in the menu, plus `StatusX` / `StatusY` for the status line position, `SpeedoX` / `SpeedoY` for the speedometer position and `DamageX` / `DamageY` for the damage numbers (0–1 screen fractions). `[ESP] VehicleTypes` is `All`, `None`, or a comma-separated list of the menu's class names (`Super, Sports, Sports Classics`). Values outside the menu's ranges are clamped, and spaces around `=` are ignored.
- `[ESP.Hostiles]`, `[ESP.Cops]`, `[ESP.Pedestrians]`, `[ESP.Animals]`, `[ESP.Vehicles]`: the grid, with keys `Enabled`, `Box`, `Health`, `Distance`, `Labels`, `Tracers`.
- `[Collectibles]`: `model_name=Label` lines.
- `[Pickups]`: `model_name=Label` lines naming non-weapon pickups.

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
3. Launch with **BattlEye disabled** (Rockstar Games Launcher → Settings → your GTA V edition) and load Story Mode.

To uninstall or before playing GTA Online, remove `dinput8.dll` (stops all ASI plugins loading) or all the files above.

## Code layout

| File | Purpose |
|---|---|
| `src/invoke.h` | Native invoker over the SDK's `nativeInit` / `nativePush64` / `nativeCall`, and native hashes (each checked against the SDK's `natives.h`) |
| `src/ui.*` | Drawing helpers (text, rects, sprites, 3D lines, bounding boxes), notifications, keyboard input |
| `src/settings.*` | `Settings` struct and `SPESP.ini` load / save |
| `src/esp.cpp` | Ped categories, ped / vehicle ESP, labels, threat arrows |
| `src/aim.cpp` | Auto-aim target selection, line of sight, smoothing, aim zone |
| `src/world.cpp` | Collectible / pickup finder, damage numbers, speedometer, teleport, skip time |
| `src/vehicle_makes.h` | Generated table: vehicle model hash -> in-game make and real-life brand |
| `src/menu.*` | In-game menu |
| `src/main.cpp` | Plugin entry point, hotkeys, status line |
