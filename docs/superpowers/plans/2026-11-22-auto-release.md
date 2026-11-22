# Automatic Releases Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every push to `main` builds Los Santos Lens in GitHub Actions and publishes a flat, drop-in zip as the next `vX.Y.Z` release.

**Architecture:** The dependencies (`deps/`) and the ScriptHookV SDK (`sdk/`) are committed so CI is self-contained. A single workflow computes the next version from tags, builds with that version baked in, zips `deps/*` plus the `.asi`, and creates the release with `gh`.

**Tech Stack:** CMake + MSVC (VS 2022 on `windows-latest`), a Windows version resource, GitHub Actions, `gh` CLI, PowerShell.

**Spec:** `docs/superpowers/specs/2026-11-22-auto-release-design.md`

## Global Constraints

- Zip name `LosSantosLens-vX.Y.Z.zip`; every file sits at the zip root; contents are exactly `deps/*` plus `LosSantosLens.asi`.
- First release `v1.0.0`; after that the default is patch + 1; `[minor]` / `[major]` in the head commit message bumps that part (resetting the lower parts).
- Skip pushes that only change `**.md` or `docs/**`.
- Local builds report version `dev`.
- No links to dev-c.com. Commits follow CLAUDE.md (imperative subject, `- ` bullets, no AI attribution).
- There's no test framework. Verification is a build, a workflow run, and inspecting the published zip. In-game checks are left to the user.

## Review Focus

- `deps/` files must reach the zip byte-for-byte (CRLF conversion of `.ini`/`.txt` would alter them): `.gitattributes` marks `deps/** -text binary`; Task 3 compares hashes against the desktop folder.
- Two pushes in quick succession must not both claim the same version: the workflow uses a `concurrency` group without cancellation.
- Tags that aren't `vN.N.N` (none today) must not break version parsing: only tags matching `^v\d+\.\d+\.\d+$` count.
- The commit message is untrusted text: read it through an `env:` variable, never `${{ }}` interpolation inside the script.
- A build failure must not leave a tag behind: the tag is created only by `gh release create` (via `--target`), in the last step.

---

### Task 1: Version in the plugin

**Files:**
- Modify: `CMakeLists.txt`
- Create: `src/version.rc`
- Modify: `src/menu.cpp:524`

**Interfaces:**
- Produces: CMake cache var `LSL_VERSION` (string, default `dev`). Compile definitions `LSL_VERSION_STR` (`"1.2.3"` or `"dev"`) and `LSL_VERSION_MAJOR/MINOR/PATCH` (ints; `0` for `dev`), available to C++ and to the `.rc` file.

- [ ] **Step 1:** In `CMakeLists.txt`, add `set(LSL_VERSION "dev" CACHE STRING ...)`, parse `X.Y.Z` with `string(REGEX MATCH ...)` into three ints (default 0), add `src/version.rc` to the sources, and add the four definitions with `target_compile_definitions` (they reach the RC compiler too).
- [ ] **Step 2:** Write `src/version.rc`: a `VS_VERSION_INFO` with `FILEVERSION`/`PRODUCTVERSION` from the ints, and `StringFileInfo` with `FileDescription` "Los Santos Lens", `ProductName` "Los Santos Lens", `FileVersion`/`ProductVersion` = `LSL_VERSION_STR`, `OriginalFilename` "LosSantosLens.asi".
- [ ] **Step 3:** In the menu banner, draw `"Los Santos Lens v" LSL_VERSION_STR` for release builds and plain "Los Santos Lens" for `dev` builds.
- [ ] **Step 4: Verify.** Configure and build with `-DLSL_VERSION=1.2.3`. Expected: the build succeeds and `(Get-Item build\Release\LosSantosLens.asi).VersionInfo` shows FileVersion `1.2.3` and description "Los Santos Lens". Then reconfigure with `-DLSL_VERSION=dev` and build. Expected: the build succeeds and the version is `0.0.0.0` / `dev`.
- [ ] **Step 5: Commit**, with the subject `Embed the release version in the plugin` (not pushed yet).

### Task 2: Dependencies, SDK and the release workflow

**Files:**
- Create: `deps/` (12 files copied from `C:\Users\lun\Desktop\gta-v-enhanced-deps`)
- Create: `.gitattributes`, `.github/workflows/release.yml`
- Modify: `.gitignore` (drop `sdk/`), `README.md`

**Interfaces:**
- Consumes: `-DLSL_VERSION=<X.Y.Z>` from Task 1.

- [ ] **Step 1:** Copy the deps, write `.gitattributes` (`deps/** -text binary`, `sdk/lib/** binary`), remove `sdk/` from `.gitignore`, and `git add deps sdk`. Expected: `git ls-files deps | wc -l` = 12 and `sdk` shows 6 files.
- [ ] **Step 2:** Write `release.yml`:
  - Trigger on push to `main` with `paths-ignore: ['**.md', 'docs/**']`. Set `permissions: contents: write` and `concurrency: { group: release, cancel-in-progress: false }`. Run on `windows-latest`.
  - Check out with `fetch-depth: 0`.
  - **Version step** (pwsh): take tags matching `^v\d+\.\d+\.\d+$` and sort them as `[version]` to find the highest. With none, the version is `1.0.0`. Otherwise bump by `$env:MSG` (the head commit message): `[major]`, else `[minor]`, else patch. Write `version=X.Y.Z` to `$GITHUB_OUTPUT`.
  - **Build:** `cmake -B build -A x64 -DLSL_VERSION=...`, then `cmake --build build --config Release`.
  - **Package:** stage `deps/*` and `build/Release/LosSantosLens.asi` into one folder, then `Compress-Archive` it to `LosSantosLens-vX.Y.Z.zip`.
  - **Release:** `gh release create vX.Y.Z <zip> --target $GITHUB_SHA --title "Los Santos Lens vX.Y.Z" --generate-notes --notes "<install line>"`, with `GH_TOKEN: ${{ github.token }}`. The install line is "Extract everything into your GTA V Enhanced folder (next to `GTA5_Enhanced.exe`) and launch with BattlEye off."
- [ ] **Step 3:** Update the README:
  - Installing: download the latest release zip and extract it into the game folder. List the bundled files and what each is for. On Enhanced the loader is `xinput1_4.dll` (`dinput8.dll` is the legacy loader). Keep BattlEye off. Note that `NativeTrainer.asi` and the SHVDN console both default to F4. Remove the dev-c.com runtime download step.
  - Building: the SDK is in the repo, and `-DLSL_VERSION` is optional.
  - Add a Releases section: every push to `main` makes the next patch release, `[minor]`/`[major]` in the commit message bumps more, docs-only pushes are skipped, and updating a dependency means replacing it in `deps/` and pushing.
  - Mention that the menu banner shows the version.
- [ ] **Step 4: Verify locally.** Run `cmake --build build --config Release`. Expected: the build succeeds.
- [ ] **Step 5: Commit** with the subject `Publish a release on every push to main`, then bullets. Commit the spec and this plan alongside it.

### Task 3: First release

- [ ] **Step 1:** `git push`. Expected: the workflow `release` starts (`gh run list --limit 1`).
- [ ] **Step 2:** `gh run watch <id> --exit-status`. Expected: success.
- [ ] **Step 3:** `gh release view v1.0.0`. Expected: one asset, `LosSantosLens-v1.0.0.zip`.
- [ ] **Step 4:** Download the zip into the scratchpad and extract it. Expected: 13 files at the root. Each of the 12 deps matches the desktop folder by SHA-256, and the `.asi` reports FileVersion `1.0.0`.
- [ ] **Step 5:** Deploy that `.asi` to the game folder if the game is closed.
