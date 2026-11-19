# Garage Preview, Plate Editor and Favorites Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Saving a car also takes a photo of it. The Garage page shows a preview panel with that photo and the car's details. P edits a car's plate, and F marks it as a favourite, which keeps it at the top of the list.

**Architecture:**
- **New `src/photo.cpp`:** the camera shot, the window capture, PNG encoding through WIC, and drawing a cached ScriptHookV texture.
- **`garage.cpp`:** takes the photo when you save, and gets favourites, plate editing and the display order.
- **`menu.cpp`:** draws the panel and handles the keys.
- **Deferred actions:** saving and plate editing both wait across frames (`scriptWait`), so the menu runs them deferred, the way it already runs calling a car.

**Tech Stack:** C++17, MSVC, CMake, ScriptHookV SDK 1.0.617.1a (`createTexture` / `drawTexture` in `main.h`), GDI, WIC (`windowscodecs.lib`).

**Spec:** `docs/superpowers/specs/2026-11-19-garage-preview-design.md` (extends `2026-11-19-garage-design.md`)

## Global Constraints

- Copy every native hash from `sdk/inc/natives.h`; the tables below already were. Keep the "checked against natives.h" claim true.
- Every exit from taking a photo, including failures, must turn the scripted camera off (`RENDER_SCRIPT_CAMS(0, …)`, `DESTROY_CAM`) and leave the HUD visible.
- Photos are 512×288 PNGs in `SPESP_garage\` next to the `.asi`. Each car records its file as `Photo=<file>.png`. Never save an all-black capture.
- Plates are at most 8 characters, uppercased.
- Keys: **F** toggles favourite, **P** edits the plate, **X** (twice) deletes. They only act on a car row of the Garage page. While the menu is open, block game controls 23 (`INPUT_ENTER`), 73 (`INPUT_VEH_DUCK`) and 75 (`INPUT_VEH_EXIT`), as well as the ones already blocked.
- Changes are limited to our own called car (its plate). Don't touch any other vehicles or peds.
- Build: `"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release`. It must finish with no errors and no new warnings. There are no tests, and nothing in-game may be claimed as working.
- Git: use `git -c safe.directory='*'` (or the `GIT_CONFIG_*` environment variables), work on a `garage-preview` branch, and never add AI attribution.
- Keep `README.md` in sync.

## Review Focus

1. **The capture fails:** the game isn't the foreground window, the image is all black, or WIC fails. The car is still saved without a photo, you get the notification "Saved <name> (no photo)", and the camera and HUD are restored. (Task 1)
2. **A photo file is missing or deleted by hand.** The panel shows the no-photo text, with no crash and no `createTexture` call on a missing file. (Task 1)
3. **The plate keyboard is cancelled (Esc) or returns empty text:** the plate is unchanged and nothing is written. (Task 2)
4. **Favourite, delete and re-save:** the menu row, `GarageCalledIndex()` and the selection still point at the same car after the display order changes. (Task 2)
5. **Re-saving the called car:** its old photo file is deleted and the panel shows the new one, never a stale cached texture. Textures are cached by file name, and new photos get new names. (Task 1)

---

### Task 1: Photo capture and drawing (feasibility build)

**Files:**
- Create: `src/photo.cpp`, `src/photo.h`
- Modify: `src/invoke.h`, `src/features.h` (`SavedCar::photo`), `src/garage.cpp`, `src/menu.cpp`, `CMakeLists.txt` (add `src/photo.cpp`, link `windowscodecs`)

**Interfaces:**
- Produces (`photo.h`):
  - `std::string PhotoDir();` returns `<asi folder>\SPESP_garage\` and creates it if it's missing.
  - `bool TakeCarPhoto(Vehicle v, const std::string& file);` calls `scriptWait`. `file` is a bare name, saved into `PhotoDir()`. Returns false on any failure. On every path it restores the camera and HUD.
  - `bool DrawPhoto(const std::string& file, float x, float y, float w, float h);` draws centred at x/y with screen-fraction size. It returns false, without drawing, when the file doesn't exist or the texture can't be created. Textures are created once per file name and cached.
- Produces (`features.h`): `SavedCar::photo` (`std::string`, empty = no photo), read from and written to `Photo=`.
- Produces (`garage.cpp`): `GarageSaveCurrent()` now calls `scriptWait`, so the menu must run it deferred.

| Native | Hash |
|---|---|
| CREATE_CAM ("DEFAULT_SCRIPTED_CAMERA", 1) | 0xC3981DCE61D9E13F |
| SET_CAM_COORD | 0x4D41783FB745E42E |
| SET_CAM_FOV | 0xB13C14F66A00D047 |
| POINT_CAM_AT_ENTITY (cam, v, 0,0,0, 1) | 0x5640BFF86B16E8DC |
| SET_CAM_ACTIVE | 0x026FB97D0A425F84 |
| RENDER_SCRIPT_CAMS (render, 0, 0, 1, 0) | 0x07E5B515DB0636FC |
| DESTROY_CAM (cam, 0) | 0x865908C81A2C22E9 |
| HIDE_HUD_AND_RADAR_THIS_FRAME | 0x719FF505F097FD20 |

- [ ] **Step 1: Write `TakeCarPhoto` in `photo.cpp`.**
  - **Camera:** the car's size comes from `GET_MODEL_DIMENSIONS` (length L = max.y − min.y). Place the camera with `GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(v, -0.8·L, 1.0·L, 0.35·L)`, a front-left 3/4 view, pointed at the car, FOV 40. Render it for 4 frames, calling `HIDE_HUD_AND_RADAR_THIS_FRAME` every frame, then capture.
  - **Capture:** fail unless `GetForegroundWindow()` belongs to this process. Use its client rect (`GetClientRect` + `ClientToScreen`). BitBlt from the screen DC into a 32-bit DIB section, crop to the largest centred 16:9 area, and `StretchBlt` (HALFTONE) to 512×288.
  - **Black check:** if the mean of R+G+B over all pixels is below 8, treat it as a failure.
  - **Encode:** call `CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)` once (ignore `RPC_E_CHANGED_MODE`). Use the WIC PNG encoder with `GUID_WICPixelFormat24bppBGR` (or 32bppBGRA). Write to `PhotoDir() + file`.
  - **Restore:** `RENDER_SCRIPT_CAMS(0, …)` and `DESTROY_CAM` on every return path. Use an RAII guard.

- [ ] **Step 2: Write `DrawPhoto` with ScriptHookV `drawTexture`.**
  - Arguments: index 0, level 0, time 100 ms, centre 0.5/0.5, rotation 0, colour 1,1,1,1, and `screenHeightScaleFactor = AspectRatio()`.
  - Size: scale the sizes so the drawn image fills w×h screen fractions. Per `main.h`, a size of 1.0 is 20% of the screen, so divide by 0.2. Check this in-game.
  - Cache: an `std::unordered_map<std::string,int>` from file name to texture id.
  - Before creating a texture, check that the file exists (`GetFileAttributesA`).

- [ ] **Step 3: Make the garage use photos.**
  - **Saving:** after a successful capture, `GarageSaveCurrent` names the photo `car_<GetTickCount64 as hex>.png`. When updating the called car in place, delete the old photo file first.
  - **Failure:** if the capture fails, save the car without a photo and notify `Saved <name> (no photo)` (or `Updated …`).
  - **Deleting:** `GarageDelete` deletes the car's photo file.
  - **File:** `Photo` is read and written like the other keys.

- [ ] **Step 4: Run the save deferred and add the minimal panel.**
  - **Deferred save:** the `Save current car` action sets `g_pendingSave`, and `MenuTick` runs it before the input pass, like `g_pendingCall`. The menu stays open.
  - **Minimal panel:** on the Garage page, draw `DrawPhoto(selected car's photo, …)` to the right of the menu, at `kX + totalW + 0.01` from the left edge, 0.22 wide, with a 16:9 height (`0.22 * AspectRatio() * 9/16`).

- [ ] **Step 5: Build.** Expected: `gta_esp.vcxproj -> ...\SPESP.asi`, no errors, no new warnings.

- [ ] **Step 6: Commit, then deploy if the game is closed.** Commit as `Take a photo when saving a garage car and show it beside the menu`, with bullets. Then check `Get-Process GTA5_Enhanced,PlayGTAV`. If it prints nothing, copy `SPESP.asi` to the game folder.

- [ ] **Step 7: Ask the user for the feasibility check before Task 2.** Ask them to save one car, then report:
  - (a) whether a PNG appeared in `SPESP_garage\`, and whether it shows the car rather than black;
  - (b) whether the photo shows beside the garage menu.

  If (b) fails, Tasks 2 and 3 still go ahead and the panel becomes text-only (`DrawPhoto` returning false already gives that). Record this as a ruling.

### Task 2: Favourites and plate editor

**Files:**
- Modify: `src/features.h`, `src/garage.cpp`, `src/menu.cpp`, `src/invoke.h`

**Interfaces:**
- Consumes: `SavedCar`, `GarageCalledIndex()`, and the menu's deferred-action pattern.
- Produces:
  - `SavedCar::favorite` (`bool`), read from and written to `Favorite=`.
  - `std::vector<int> GarageDisplayOrder();` returns garage indices with favourites first, otherwise in save order (a stable partition).
  - `void GarageToggleFavorite(int index);`
  - `bool GarageSetPlate(int index, const std::string& text);` trims, uppercases and cuts the text to 8 characters. Empty text gives false with no change. Otherwise it writes the file, and if `index == GarageCalledIndex()` and the called car exists, applies `SET_VEHICLE_NUMBER_PLATE_TEXT` to it.

| Native | Hash |
|---|---|
| DISPLAY_ONSCREEN_KEYBOARD (0, "FMMC_KEY_TIP8", "", current, "", "", "", 8) | 0x00DC833F2568DBF6 |
| UPDATE_ONSCREEN_KEYBOARD (0 = editing, 1 = done, 2 = cancelled; 3 = not active) | 0x0CF2B696BBF945AE |
| GET_ONSCREEN_KEYBOARD_RESULT | 0x8362B09B91893647 |

- [ ] **Step 1: Add the favourite field, `GarageDisplayOrder`, `GarageToggleFavorite` and `GarageSetPlate`.**

- [ ] **Step 2: Build the menu's car rows in `GarageDisplayOrder()`.** `Item::index` stays the garage index, so calling, deleting and the `(out)` marker keep working. The label shows `★ ` before favourites; if the menu font has no ★, use `* `.

- [ ] **Step 3: Add the keys on Garage-page car rows.**
  - **F** toggles the favourite. The selection follows the same car to its new row.
  - **P** sets `g_pendingPlate = index`. `MenuTick` runs it deferred: show the keyboard, `scriptWait(0)` while the status is 0, and on status 1 call `GarageSetPlate` with the result and notify `Plate set to <TEXT>`.
  - **Disarm:** F, P, Left and Right also cancel an armed delete.
  - **Blocking:** add controls 23, 73 and 75 to `BlockGameInput`.
  - **Help bar:** `Up/Down: move    Enter: call    F: favorite    P: plate    X: delete    Backspace: …`

- [ ] **Step 4: Build.** Same expected output as Task 1.

- [ ] **Step 5: Commit.** `Add garage favorites and a plate editor`, with bullets.

### Task 3: Preview panel details and README

**Files:**
- Modify: `src/menu.cpp`, `src/invoke.h`, `README.md`

| Native | Hash |
|---|---|
| GET_VEHICLE_CLASS_FROM_NAME | 0xDEDF1C8BD47C2200 |

- [ ] **Step 1: Complete the panel.** It's a dark panel (`kRowBg` style) in the column from Task 1 Step 4. From top to bottom:
  - the photo, or the dim two-line text "No photo yet" / "Re-save the car to take one"
  - the name in white
  - the real brand in green (`FindVehicleMake(model)->real`, skipped if it's unknown)
  - the class in yellow (`kVehicleClasses[GET_VEHICLE_CLASS_FROM_NAME(model)]`, guarded to the range 0–22)
  - `Plate  <TEXT>`
  - `★ Favorite` and `(out)`, when they apply

  Only draw the panel when a car row is selected.

- [ ] **Step 2: Update `README.md`.** Cover the photos (the brief camera cut, `SPESP_garage\`, re-save to retake, nothing saved if the capture is black or you're alt-tabbed), the panel, P (8 characters, applies to the called car too), F (favourites on top), the help bar, and `Photo=` and `Favorite=` in the garage-file section. Add `src/photo.cpp` to the code layout.

- [ ] **Step 3: Build.** Same expected output as Task 1.

- [ ] **Step 4: Commit.** `Show car details in the garage preview panel`, with bullets.

### Task 4: Deploy and the in-game checklist

- [ ] **Step 1: Deploy.** Check the processes and copy, as in Task 1 Step 6.

- [ ] **Step 2: Give the user the checklist; don't mark any of it verified.**
  - Save a car: brief camera cut, photo in the panel.
  - Alt-tab during a save: "(no photo)", and the camera is back to normal.
  - P: type a plate, then Esc on a second try (no change). The called car's plate changes.
  - F: the car jumps to the top, and Enter still calls the right car.
  - Delete a car: its PNG is gone from `SPESP_garage\`.
