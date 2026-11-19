# Garage: photo preview, plate editor, favorites

Extends the garage (`2026-11-19-garage-design.md`).

## Intent

- **Preview:** while you scroll the garage, see what each car actually looks like, with your own paint, wheels and neon, not just its name.
- **Plate editor:** set the plate text yourself. Story Mode's Los Santos Customs only offers plate styles.
- **Favorites:** keep the cars you use most at the top of the list.

## Goals

### Photo on save
Choosing **Save current car** also takes a photo:
1. A scripted camera cuts to a 3/4 front view of the car, framed from the model's size, for a few frames. The HUD, radar and our menu are hidden.
2. The plugin captures the game window's client area and crops it to 16:9 around the car.
3. The crop is scaled to 512×288 and saved as a PNG in a `SPESP_garage\` folder next to the `.asi`.
4. The camera goes back to normal.

The car's section records the file as `Photo=<file>.png`. Re-saving the called car (update in place) retakes its photo. Deleting a car deletes its photo.

### Preview panel
A panel to the right of the menu, shown only on the Garage page, for the selected car:
- **Photo:** drawn with ScriptHookV's `createTexture` / `drawTexture`.
- **Spec lines:** the name, the real-life brand (from `vehicle_makes.h`), the class, the plate, ★ if it's a favourite, and `(out)` if it's the called car.

A car with no photo (saved before this change, or the capture failed) shows the spec lines only, with "No photo yet: re-save the car to take one."

### Plate editor
**P** on a car row opens the game's on-screen keyboard (`DISPLAY_ONSCREEN_KEYBOARD`), prefilled with the current plate.
- **Limit:** at most 8 characters, uppercased.
- **Confirming** updates the saved entry and the file. If that car is the called car and still exists, its plate changes too.
- **Cancelling** changes nothing.

### Favorites
**F** on a car row toggles ★. Favorites are listed first; otherwise the list keeps its save order. Stored as `Favorite=1`.

### Help bar
`Enter: call    F: favorite    P: plate    X: delete    Backspace: …`

### README
Covers all of the above, plus the `SPESP_garage\` folder.

## Non-goals

- A live 3D view, or spawning a preview copy of the car.
- The game's website thumbnails.
- Photos for cars you aren't sitting in. A photo is only taken on save.
- Editing anything other than the plate text from the garage.

## Anti-goals

- The photo camera must never be left on, and the HUD must never be left hidden. Every exit path, including a failed capture, restores both.
- No change to other peds or vehicles. The plate change applies only to our own called car.
- No hand-written native hashes. Copy every hash from `sdk/inc/natives.h`.

## Constraints and risks

- **Unverified: `drawTexture` on Enhanced.** Whether it works with the Enhanced (DX12) ScriptHookV build can't be told from the code. If it doesn't, the panel shows the spec lines only, and the photo files are still saved.
- **Unverified: the capture.** Capturing the game window with GDI (BitBlt from the window's client area, or `PrintWindow` with `PW_RENDERFULLCONTENT`) may come out black in some display modes. Detect an all-black capture, skip the photo and notify. Don't save a black image.
- **Encoding:** PNG via the Windows Imaging Component (built into Windows), so no new dependencies.
- **Textures:** they can't be freed until scripts reload. Create one per photo file, lazily, and cache it. At 40 cars that's fine.
- **Keys:** F, P and X only act while the Garage page is open. Block the game controls they'd otherwise trigger while the menu is open.
- **Testing:** there are no tests. The first plan task builds the capture and drawing on their own, so the user can confirm in one game launch that both work before the rest is built.

## Approaches

1. **A photo taken on save (chosen).** It shows your actual customised car, but needs the unverified capture and `drawTexture`.
2. The game's website thumbnails. Rejected: stock paint only, only some models have one, and the texture names are unverified.
3. A live 3D render of a hidden copy. Rejected: it spawns entities, and the camera takes the whole screen, so it can't sit in a side panel.

## Left to the builder

- The exact camera offset and FOV.
- How many frames to wait before capturing.
- The panel's layout and sizes.
- The class lookup for a model: use a native if the SDK has one; otherwise store the class at save time.
- How favourite sort order is kept separate from garage indices: a display-order index list is suggested, so `GarageCalledIndex` stays valid.
