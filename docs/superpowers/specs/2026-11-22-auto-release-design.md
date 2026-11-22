# Automatic releases

## Why

Every push to `main` should produce a ready-to-play GitHub release without the user doing anything: one zip whose contents get copied straight into the GTA V Enhanced folder.

## Goals

- A push to `main` builds the plugin in GitHub Actions and publishes a release with the next version.
- The release zip is flat: `LosSantosLens.asi` plus every file from the user's verified dependency set, ready to drop into the game folder.
- The first release is `v1.0.0`.

## Non-goals

- No download of ScriptHookV or its SDK from dev-c.com (it refuses automated downloads), and no links to dev-c.com.
- No release PRs, changelog files or commit-message conventions.

## Anti-goals

- Any step the user has to do by hand per release.

## Approaches

- **A. Manual "Release" button** with a patch/minor/major choice. Rejected: the user doesn't want to attend releases at all.
- **B. Release on every push to `main`** (picked by the user).
- **C. Version from commit-message prefixes** (release-please / semantic-release). Rejected: it needs `feat:`/`fix:` commits and a release-PR step.

## Design

- **Dependencies in the repo:**
  - `deps/` holds the 12 files from `C:\Users\lun\Desktop\gta-v-enhanced-deps`: `ScriptHookV.dll`, `xinput1_4.dll` (the Enhanced ASI loader), `dinput8.dll`, `MinHook.x64.dll`, `DirectStorageFix.asi`, `NativeTrainer.asi`, `ScriptHookVDotNet.asi`/`.ini`, `ScriptHookVDotNet2.dll`, `ScriptHookVDotNet3.dll`, `StraightToStoryMode.asi` and `args.txt`. They're all included because the user asked for all of them; to update a dependency, replace the file in `deps/` and push.
  - `sdk/` (`inc/` and `lib/ScriptHookV.lib`) is committed and removed from `.gitignore`, so CI can compile.
- **Workflow:** `.github/workflows/release.yml` runs on `windows-latest` on every push to `main`. Pushes that only touch `*.md` or `docs/**` are skipped *(builder's call: so that docs edits don't become releases)*. A concurrency group runs one release at a time.
- **Next version:** the highest `vX.Y.Z` tag plus one patch. With no tags yet, it's `v1.0.0`. A commit message containing `[minor]` or `[major]` bumps that part instead *(builder's call: an optional escape hatch that needs no action otherwise)*.
- **Build:** CMake configures with `-DLSL_VERSION=X.Y.Z` and builds Release.
- **Package:** `LosSantosLens-vX.Y.Z.zip` contains `deps/*` plus `LosSantosLens.asi`, all at the zip root.
- **Release:** `gh release create vX.Y.Z` with the zip, generated notes (the commits since the last release) and a short install line at the top. It uses the workflow's `GITHUB_TOKEN` with `contents: write`.
- **Version in the plugin** *(builder's call)*: CMake passes `LSL_VERSION` (default `dev` for local builds) as a compile definition and into a version resource. The menu banner reads "Los Santos Lens v1.0.0", and the `.asi` file's Properties → Details shows the version.
- **README:** the Installing section becomes "extract the release zip into the game folder", with the file list and BattlEye off. Fix the stale `dinput8.dll` claim: on Enhanced, the loader is `xinput1_4.dll`. Add a Releases section on how versions are made.

## Known effects of including everything

- `NativeTrainer.asi` isn't in the user's current game folder. Its menu hotkey is F4 by default, and the SHVDN console (`ScriptHookVDotNet.ini`) is also F4.
- `dinput8.dll` is the legacy loader. It doesn't load plugins on Enhanced, but it's harmless.

## Left to the builder

- Exact workflow steps and action versions, and the version-resource details.
