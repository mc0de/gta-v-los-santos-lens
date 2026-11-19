# Garage: save cars and call them back

## Intent

A personal garage that lives outside the game's own one. You save a car you're sitting in, with all its customisation, and later call it back wherever you are. It works like calling a car in Cyberpunk 2077 or ordering a taxi: the car turns up nearby and a blip shows where it is. It's for when your car is wrecked, impounded or parked on the other side of the map.

## Goals

- **Save current car** (you must be in it) records everything that makes the car yours:
  - model
  - every mod slot, plus turbo, xenon and tyre smoke
  - primary, secondary, pearlescent and wheel colours, including custom RGB paint
  - wheel type and custom tyres
  - extras, livery and window tint
  - neon lights and their colour, and tyre smoke colour
  - plate text and plate style
- **Call** spawns the saved car empty on the nearest road spot 30–60 m away. Where possible the spot is out of camera view, and the car faces along the road. It arrives unlocked with the engine off.
- **One car out at a time.** Calling another car removes the previous one, if you aren't in it and it's off screen. Otherwise the old car is released to the game, which keeps it or despawns it as it would any other car.
- **Blip:** a car icon in its own colour. It's hidden while you're in the car and shown when you get out. If the car is destroyed, the blip is removed and you get a notification.
- **Separate menu and hotkey:** Numpad `+` by default (`[Keys] Garage` in `SPESP.ini`). The menu lists saved cars as make and model. Enter calls the selected car, X deletes it, Backspace closes the menu. **Save current car** is the first row.
- Saved cars go in `SPESP_garage.ini` next to the `.asi`, one section per car, so the file can be backed up or edited by hand.

## Non-goals

- A valet or driver bringing the car. Option 2 was rejected because GTA traffic AI gets stuck, and it would mean creating and controlling a ped.
- Saving damage, dirt, fuel or how worn the car is. A called car always arrives clean.
- Touching the game's own garages, personal vehicles, impound or save file.
- Spawning cars you haven't saved, so there's no vehicle spawner by model name.

## Anti-goals

- Anything that goes through the game's cheat-code system or sets its cheat flag.
- Changing other peds or vehicles. Spawning, blipping and removing *our own* called car is the only world write, and it's an explicit player action like teleport.
- Hand-written native hashes. Every native is copied from `sdk/inc/natives.h`, or the feature it would serve is left out.

## Constraints

- ScriptHookV SDK 1.0.617.1a (2016). Customisation features added after 2016 (for example newer livery types or interior colours) may have no hash there. Those are skipped, and the README says what isn't preserved.
- Hotkeys: not Home, not F5–F8 (character switch), not F1–F3 (Rockstar Editor).
- No tests exist. The only check is building the plugin and trying it in-game.

## Approaches

1. **Appear on the road a short way off (chosen).** The car is empty, out of view, blipped, and you walk to it. It never gets stuck and needs no ped.
2. Valet drive-up: closest to Cyberpunk, but unreliable AI and it needs a ped. Rejected.
3. Spawn right beside you: instant, but it clips into walls and cars in tight spots. Rejected.

## Left to the builder

- Code layout (likely a new `garage.cpp`) and how the garage menu shares drawing code with `menu.cpp`.
- The exact road-node query and its fallback when no node is in range (for example, beside the player).
- Model streaming and timeout, and the notification text.
- Blip sprite and colour.
- The `SPESP_garage.ini` key format.
