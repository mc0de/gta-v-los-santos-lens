# SPESP: notes for Claude

SPESP is a ScriptHookV `.asi` plugin (C++17, MSVC, CMake) for GTA V **Story Mode**. It adds ESP overlays, auto-aim, a collectible, pickup and letter finder, damage numbers, teleport to waypoint, and an in-game menu backed by `SPESP.ini`. `README.md` is the user-facing spec (features, hotkeys, ini keys, code layout), so read it first.

## Build

```
cmake -B build -G "Visual Studio 17 2022" -A x64   # once
cmake --build build --config Release               # -> build/Release/SPESP.asi
```

- `sdk/` (gitignored) holds the ScriptHookV SDK 1.0.617.1a: `sdk/inc/{main.h,types.h,natives.h}` and `sdk/lib/ScriptHookV.lib`.
- On this machine `cmake` isn't on PATH. Use the VS 2022 Build Tools copy (the full path is in Claude's project memory).
- There are no tests. The only way to verify a change is to deploy it and play. Say so instead of claiming something works in-game.

## Architecture

- `main.cpp`: `DllMain` registers `ScriptMain`, which loops `Tick()` + `scriptWait(0)` once per frame. `Tick` skips when the pause menu is open, then runs hotkeys, `AimTick`, `EspTick`, `FinderTick`, `LettersTick`, `DamageNumbersTick`, `MenuTick` and the status line.
- `features.h`: declarations shared by the feature modules (`esp.cpp`, `aim.cpp`, `world.cpp`) plus `g_aimTarget`, `IsThreat`, `HasLineOfSight` and `IsLawEnforcement`.
- `invoke.h`: `Invoke<R>(hash, args...)` over `nativeInit`/`nativePush64`/`nativeCall`, and every native hash, in `namespace N`. Natives are called by hash, not through the SDK's `natives.h` wrappers.
- `ui.*`: drawing helpers (text, rects, sprites, 3D lines, bounding boxes), colour constants (`kRed`, `kCyan`, ...), and `KeyPressed`/`KeyRepeat`, which ignore input while the game window isn't focused.
- `settings.*`: one global `Settings g`. INI read/write is in `settings.cpp`.
- `menu.cpp`: the item list is built from helpers like `Toggle(...)` and `IntRange(...)`. The ESP section is a grid with one row per category and one column per `EspStyle` field.

**Adding a setting** touches four places: the field and its default in `settings.h`, `Read*` in `LoadSettings` and `Write*` in `SaveSettings` (`settings.cpp`), the menu item in `menu.cpp`, and the README (feature text plus the ini section).

## Rules

- **Native hashes:** grep `sdk/inc/natives.h` for every new native and copy its hash from there. Never write a hash from memory, because a wrong one makes ScriptHookV fatal ("Can't find native") and crashes the game. Names in the 2016 SDK can differ from current NativeDB names, so search by hash as well. Update the "checked against natives.h" comment in `invoke.h`.
- **Read-only toward the world:** features must not modify other peds or vehicles, or their damage and relationship records. Calling `CLEAR_ENTITY_LAST_DAMAGE_ENTITY` erased the player's kill credit and broke mission kill counters. Explicit player actions such as teleport are fine, and so is moving the camera for auto-aim.
- **Unverifiable model or asset names** (collectible props, for example) go in the ini, so the user can fix them. Tell the user they're unverified.
- **Keys:** Home opens the Rockstar launcher overlay, and F5–F8 are the game's character switch, so don't bind those.
- **Docs:** keep `README.md` in sync with every user-visible change (features, defaults, colours, keys, ini).
- **Commits:** a short imperative subject, then `- ` bullets for each change when there's more than one. Never add AI attribution or `Co-Authored-By` lines.

## Deploy and runtime gotchas

- Copy `build/Release/SPESP.asi` into the game folder. This only works while the game is closed, because the running game locks the file. Check for `GTA5_Enhanced` / `PlayGTAV` processes first. After a ScriptHookV fatal error the process can linger hidden.
- The target is GTA V **Enhanced** (`GTA5_Enhanced.exe`), which needs an Enhanced-capable ASI loader as `dinput8.dll`. The 2015 loader bundled with ScriptHookV only works with Legacy. BattlEye must be off.
- `ScriptHookV.log` and `asiloader.log` are written to the game folder.
- `SPESP.ini` sits next to the `.asi` and is re-saved whenever the menu closes. Edit or delete it only with the game closed, or use "Reload settings file" in the menu.
