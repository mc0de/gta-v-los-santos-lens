# Los Santos Lens — Single-player overlay for GTA V

A ScriptHookV plugin with ESP overlays, auto-aim and a few utilities for GTA V **Story Mode**.

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
| Box | Corners of a 2D bounding box |
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
While you hold aim (right mouse), the camera locks onto a living ped you have clear line of sight to and points the crosshair straight at it every frame, with no smoothing or delay. Peds behind walls, vehicles or objects are skipped. The lock stays on one target until it dies or goes out of sight. To switch while aiming, press **Q** for the nearest target to the left of the current one or **E** for the nearest one to the right (only targets that pass the filters below and are in sight); a target you pick this way stays locked. While auto-aim is aiming, the game's own actions on Q and E (take cover, radio wheel, pick up, talk, horn) are turned off. Auto-aim stays off while you hold a throwable (grenade, sticky bomb, molotov, etc.) so it doesn't spoil the throw arc.

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

### Finder
- **Collectibles**: orange markers, labels and 40 m beams on collectibles: letter scraps, spaceship parts, peyote plants and nuclear waste. The model list lives in the `[Collectibles]` section of `LosSantosLens.ini`. The default model names are best-known values and haven't all been verified; fix or add entries there. Collectibles are looked for among both world objects and pickups (letter scraps and spaceship parts are pickups). They only exist near the player, so this finds items in your surroundings, not across the whole map, and collected ones disappear from the world and so from the display.
- **Pickups**: markers, labels and 40 m beams on pickups, labelled with what they are and coloured by type: weapons red, health green, armour blue, ammo yellow, cash (cash, money bags, briefcases) magenta, anything else named (parachutes, snacks, packages, ...) cyan, and unknown models white. Non-weapon pickups get their type from words in their `[Pickups]` label (`health`, `armour`/`armor`, `ammo`, `cash`/`money`/`briefcase`), so keep that word in a label when renaming it. Weapon pickups are named automatically, from the same weapon list the ESP uses (DLC weapons included). Other pickups are named from the `[Pickups]` section of `LosSantosLens.ini`. Its default model names are best-known values and haven't been verified, so fix or add entries there. A pickup with an unknown model shows its model hash, e.g. `Pickup 0x1A2B3C4D`: add `0x1A2B3C4D=Name` to `[Pickups]` to name it. Collectibles aren't repeated here.
- **Pickup types**: a checklist (Weapons, Health, Armour, Ammo, Cash, Other, Unknown) of the pickup types to show, with **Show all** and **Show none**, like Vehicle types. All are checked by default.
- Collectibles and pickups each have these switches in the menu:
  - **Through walls**: markers and labels show even when the item is out of sight. When off, they only show while you have a clear line of sight to the item. On by default.
  - **Beams**: draw the beam. On by default.
  - **Beams through walls**: draw the beam over walls and terrain, as a flat line on top of the picture, instead of letting buildings hide it. On by default for collectibles, off for pickups.
- **Range** (default 300 m, under FINDER in the menu) applies to both collectibles and pickups.

### Misc
- **Damage numbers**: bright yellow `-N` numbers that float up from a ped's head and fade out when you damage it (hits on one ped within 0.2 s, e.g. fire or automatic fire, add up into one number), red `-N KILL` on a kill. Hits count while you're shooting, fighting or driving (and for 3 s after) on peds you've damaged; kills count when the game names you or your vehicle as the killer.
- **Speedometer**: while you're in a vehicle, a compact GTA-style panel in the bottom-right area (one dark background, rows with labels on the left and values on the right, like the game's race timers), from top to bottom: make in cyan and model in white (`BENEFACTOR` / `Panto`); the real-life brand the make parodies in green and the class in yellow (`Mercedes-Benz` / `Compacts`; a few brands are best guesses, e.g. Nagasaki as Kawasaki); the rated top speed (`TOP`, the game's estimate, which includes upgrades) and the fastest you've ever gone in this vehicle model (`BEST`, kept permanently in `LosSantosLens_speeds.ini`, see below); and the current speed as a big number in km/h, orange once you reach the rated top speed. On by default; position via `SpeedoX` / `SpeedoY` in the ini, which set the panel's bottom-right corner (default 0.86 / 0.985), so longer names grow to the left. Makes come from a table of ~765 models in `src/vehicle_makes.h`. Vehicles with no make (police, trains, ...) or newer than the table show only model and class.
- **Teleport to waypoint**: set a map marker and press End (or use the menu). In a vehicle, the vehicle comes with you. Over water you land on the surface, not the seabed. If no ground is found you're taken out of any vehicle and dropped from the sky with a parachute.
- **Skip time**: press Insert (or use the menu) to move the game clock forward 6 hours. The amount (1–23 hours) is set by **Hours to skip** in the menu. A notification shows the new time.

### Garage
Save the car you're in and call it back later, wherever you are. Press **Numpad +** (or open **Garage** in the menu's Misc section).

- **Save current car** (you must be in it) stores everything that makes the car yours: the model, every mod (engine, brakes, bodywork, wheels, custom tyres, turbo, xenon, tyre smoke), primary / secondary / pearlescent / wheel colours including custom RGB paint and paint type, extras, livery, window tint, neon lights and their colour, tyre smoke colour, plate text and style, and bulletproof tyres. Saving also takes a photo: the camera cuts to a 3/4 view of the car for a moment, with the HUD hidden. If the game isn't the active window, or the capture comes out black, the car is saved without one ("(no photo)"). Saving the car you called asks first: the row turns into **Overwrite saved car? < No >** and the preview shows the saved entry. Left/Right picks **Yes** (update that entry and retake its photo) or **No** (save it as a new entry, which the called car then belongs to); Enter confirms, Backspace or Up/Down cancels. It starts on No. Only road vehicles (cars, bikes, quads, bicycles) can be saved. There is no limit on how many cars you can save.
- **Calling**: pick a car and press Enter. The menu closes and the car appears empty on the nearest road 30–60 m away, out of view where possible, facing along the road, unlocked, engine off and clean. If there's no free road spot nearby it appears right next to you; if there's no dry ground next to you either (say you're swimming), you get "No room for the car here" and keep your previous car. A purple car blip shows where it is; the blip hides while you're in the car and comes back when you get out.
- **One car at a time**: calling another car takes the blip off the previous one. If it's empty it stays parked where you left it until you're 350 m away, then it's left to the game, which keeps or despawns it like any other car. Up to 3 old cars are held this way; calling a 4th releases the oldest. An old car that someone is in (you, a companion, a thief) is left to the game straight away. If the called car is destroyed you get a notification and the blip goes away.
- **Preview**: the selected car's photo and details (name, real-life brand, class, plate, favourite, whether it's the called car) appear in the menu's right-hand column. The car list stays in the left column and scrolls once it's longer than 21 rows; the GARAGE header shows how many rows are hidden above and below. Cars saved before photos existed show "No photo yet": re-save the car to take one.
- **Order**: the car you've called is the top row (tagged `CALLED`), so Enter calls it again straight away, and opening the garage selects it. Then **Save current car**, then favourites, then the rest in the order you saved them.
- **Favourites**: press F on a car to mark it with `*`.
- **Plate**: press P on a car to type a new plate in the game's on-screen keyboard (up to 8 characters, uppercase; Esc cancels). If that car is the one you called, its plate changes straight away too. Story Mode's Los Santos Customs only sells plate styles, not custom text.
- **Deleting**: select a car and press X twice. Its photo is deleted too.
- The rows show the plate on the right; the called car's row shows `CALLED` in blue instead.
- **Not saved**: damage and dirt (called cars always arrive fixed and clean), and interior, dashboard and xenon colours, which the ScriptHookV SDK this is built with can't read.
- Nothing goes through the game's cheat system.

## Menu and hotkeys

Press **Delete** to open the menu. It's laid out in two columns (ESP, Finder, Collectibles and Pickups on the left, Auto-aim and Misc on the right); Up/Down runs down the left column and on into the right one. Arrow keys move / change values (in the ESP grid, Left/Right move between cells), Enter toggles or runs an action, Backspace or Delete closes it (Backspace on a sub-page goes back to the main page, except on the garage page when you opened it with Numpad +). (Home isn't used because it opens the Rockstar Games Launcher overlay.) Settings are saved to `LosSantosLens.ini` when the menu closes.

| Key | Action |
|---|---|
| Delete | Open / close menu |
| F9 | Toggle ESP |
| F10 | Toggle pedestrians (quickly declutter busy streets) |
| F11 | Toggle vehicles |
| Page Up | Toggle auto-aim |
| Page Down | Switch aim point: head / body |
| Q / E (while aiming) | Switch auto-aim target to the left / right |
| End | Teleport to waypoint |
| Insert | Skip game time forward (6 hours by default) |
| Numpad + | Open / close the garage (on a car: Enter call, F favourite, P plate, X twice delete) |

F5–F8 are left alone because the game uses them for character switching, and F1–F3 because they run the Rockstar Editor.

A small status line near the top-left corner shows what's on, e.g. `ESP HCPV | AIM HEAD`, listing the ESP categories that are on: H = hostiles, C = cops, P = pedestrians, A = animals, V = vehicles (plus `:Type` when a vehicle type filter is set).

## Configuration (`LosSantosLens.ini`)

Created next to `LosSantosLens.asi` with defaults on first run. Edit it while the game runs and use **Reload settings file** in the menu to apply.

- `[Keys]`: key names like `F9`, `PageUp`, `Delete`, `End`, `Insert`, `Numpad5`, single letters/digits, or a hex virtual-key code like `0x7A`. `None` disables a hotkey. The garage key is `Garage=Add` (Numpad +). `AimLeft=Q` / `AimRight=E` switch the auto-aim target; the game's own Q/E actions are only turned off while these keys are Q or E.
- `[ESP]`, `[Aim]`, `[Finder]`, `[Misc]`: everything in the menu, plus `StatusX` / `StatusY` for the status line position and `SpeedoX` / `SpeedoY` for the speedometer position (0–1 screen fractions). `[ESP] VehicleTypes` is `All`, `None`, or a comma-separated list of the menu's class names (`Super, Sports, Sports Classics`); `[Finder] PickupTypes` works the same way with the pickup type names (`Weapons, Ammo`). The finder switches are `Collectibles`, `CollectiblesThroughWalls`, `CollectibleBeams`, `CollectibleBeamsThroughWalls`, and the same for pickups (`Pickups`, `PickupsThroughWalls`, `PickupBeams`, `PickupBeamsThroughWalls`). Values outside the menu's ranges are clamped, and spaces around `=` are ignored.
- `[ESP.Hostiles]`, `[ESP.Cops]`, `[ESP.Pedestrians]`, `[ESP.Animals]`, `[ESP.Vehicles]`: the grid, with keys `Enabled`, `Box`, `Health`, `Distance`, `Labels`, `Tracers`.
- `[Garage]`: `BlipSprite` (default 225, a car icon) and `BlipColour` (default 27, purple) for the called car's blip. These ids are best-known values and haven't been verified in-game; change them here if the icon or colour is wrong.
- `[Collectibles]`: `model_name=Label` lines. A model can also be given as its hash, `0x1A2B3C4D=Label`.
- `[Pickups]`: `model_name=Label` (or `0xHASH=Label`) lines naming non-weapon pickups. Default entries missing from the section are added back on load, so new default names reach existing ini files; to rename one, change its label instead of deleting the line.
- Older ini files with `[Finder] Letters=` (letter scraps used to be a separate group) get `prop_ld_scrap` added to `[Collectibles]`, `Letters=true` turns collectibles on, and the old `Letters` / `LetterCount` keys are removed.

### Best speeds file (`LosSantosLens_speeds.ini`)

The speedometer's `BEST` is kept per vehicle model (every Panto shares one record, upgraded or not) in `LosSantosLens_speeds.ini` next to `LosSantosLens.asi`, one `0xMODELHASH=km/h ; name` line per model under `[BestSpeed]`, fastest first. Records are counted whenever you're in a vehicle, even with the speedometer turned off. A new record is written within 2 seconds, and straight away when you leave the vehicle. Anything that moves the vehicle fast counts, including falling or being launched. **Clear best speeds** in the menu (Misc) forgets every record. It asks for a second Enter within 3 seconds first. To drop a single model, delete its line while the game is closed.

### Garage file (`LosSantosLens_garage.ini`)

Saved cars are kept in `LosSantosLens_garage.ini` next to `LosSantosLens.asi`, one `[CarN]` section per car, written whenever you save or delete a car. Back it up to keep your cars. You can edit it (rename a car with `Name=`, change `Plate=`, which is cut to 8 characters) while the game is closed, or while it runs followed by **Reload settings file** straight away: saving or deleting a car in-game before reloading overwrites your edits. A car whose model the game doesn't know shows as `(invalid)` and can't be called. `Favorite=1` marks a favourite, and `Photo=` names the car's photo in the `LosSantosLens_garage\` folder next to the file (512×288 PNGs, deleted with the car).

## Installing

1. Download `LosSantosLens-vX.Y.Z.zip` from the latest [release](../../releases/latest).
2. Extract everything into the GTA V Enhanced folder (next to `GTA5_Enhanced.exe`), replacing existing files.
3. Launch with **BattlEye disabled** and load Story Mode. The bundled `args.txt` passes `-nobattleye -noBE` to the game; you can also turn BattlEye off in the Rockstar Games Launcher (Settings → your GTA V edition).

The zip is self-contained, with versions verified against the current game version:

| File | What it is |
|---|---|
| `LosSantosLens.asi` | This plugin |
| `ScriptHookV.dll` | ScriptHookV runtime (must match the game version) |
| `xinput1_4.dll` | ASI loader for Enhanced: loads every `.asi` in the folder |
| `dinput8.dll` | ASI loader for Legacy (`GTA5.exe`); unused on Enhanced, harmless |
| `ScriptHookVDotNet.asi`, `ScriptHookVDotNet.ini`, `ScriptHookVDotNet2.dll`, `ScriptHookVDotNet3.dll` | ScriptHookVDotNet, for .NET script mods |
| `NativeTrainer.asi` | ScriptHookV's sample trainer |
| `DirectStorageFix.asi`, `MinHook.x64.dll` | DirectStorage fix and the hook library it uses |
| `StraightToStoryMode.asi` | Boots straight into Story Mode |
| `args.txt` | Game launch arguments (`-nobattleye -noBE`) |

`NativeTrainer.asi` and the ScriptHookVDotNet console (`ConsoleKeyBinding` in `ScriptHookVDotNet.ini`) both default to **F4**.

After a game update, ScriptHookV stops loading until it's updated, so wait for a new release.

To uninstall or before playing GTA Online, remove `xinput1_4.dll` and `dinput8.dll` (stops all ASI plugins loading) or all the files above.

## Releases

Every push to `main` builds the plugin in GitHub Actions (`.github/workflows/release.yml`) and publishes the next release with the zip above. Versions count up on their own: the next patch version each time (`v1.0.0`, `v1.0.1`, ...), or the next minor / major version when any commit since the last release has `[minor]` / `[major]` on a line of its own in its message. Pushes that only change Markdown files or `docs/` don't make a release. The release notes list the commits since the previous release.

The version shows in the menu banner (`Los Santos Lens v1.0.1`) and in the `.asi` file's Properties → Details. Local builds are version `dev`.

To update a dependency (a new ScriptHookV after a game update, for example), replace the file in `deps/` and push.

## Building

Requirements: Visual Studio 2022 or Build Tools 2022 with the **Desktop development with C++** workload (includes CMake). The ScriptHookV SDK (`sdk/inc`, `sdk/lib/ScriptHookV.lib`) is in the repo.

```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Output: `build/Release/LosSantosLens.asi`, version `dev`. Pass `-DLSL_VERSION=X.Y.Z` when configuring to set a version (quote it in PowerShell: `"-DLSL_VERSION=1.2.3"`). The CRT is linked statically, so the only runtime dependency is `ScriptHookV.dll`.

## Code layout

| File | Purpose |
|---|---|
| `src/invoke.h` | Native invoker over the SDK's `nativeInit` / `nativePush64` / `nativeCall`, and native hashes (each checked against the SDK's `natives.h`) |
| `src/ui.*` | Drawing helpers (text, rects, sprites, 3D lines, bounding boxes), notifications, keyboard input |
| `src/settings.*` | `Settings` struct and `LosSantosLens.ini` load / save |
| `src/esp.cpp` | Ped categories, ped / vehicle ESP, labels, threat arrows |
| `src/aim.cpp` | Auto-aim target selection, target switching, line of sight, aim zone |
| `src/world.cpp` | Collectible / pickup finder, damage numbers, speedometer, teleport, skip time |
| `src/garage.cpp` | Garage: saving a car's setup, `LosSantosLens_garage.ini`, calling a car, its blip, favourites, plates |
| `src/photo.*` | Garage photos: camera shot, window capture, PNG encoding, drawing them in the menu |
| `src/vehicle_makes.h` | Generated table: vehicle model hash -> in-game make and real-life brand |
| `src/menu.*` | In-game menu |
| `src/main.cpp` | Plugin entry point, hotkeys, status line |
| `src/version.h.in`, `src/version.rc` | Version header generated by CMake, and the `.asi`'s version resource |
| `deps/` | Files bundled into the release zip next to the plugin |
| `sdk/` | ScriptHookV SDK headers and import library |
| `.github/workflows/release.yml` | Builds and publishes a release on every push to `main` |
