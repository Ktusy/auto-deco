# Auto Deco — Geode mod for Geometry Dash

One-click layout decoration for the level editor, with presets such as
**Slaughterhouse** and **Acheron**.

Targets **Geode v5.x** and **Geometry Dash 2.2081** (Windows, macOS, Android, iOS).

> **Read this first — build status.**
> I wrote this against the Geode v5 docs and the community object-ID list, but I
> could not compile it against the real Geode SDK or run it inside Geometry Dash
> (no SDK/network in my sandbox). What I *did* verify: the whole generator
> (`Presets` + `Generator`) compiles with `-Wall -Wextra` and passes a native test
> under AddressSanitizer/UBSan (`tests/`). The Geode-facing files (`main.cpp`,
> `Bridge.cpp`, `DecoPopup.cpp`) are the part most likely to need a small fix if
> a Geode API differs from what I expect. The GitHub Actions workflow below will
> tell you exactly what, if anything, to adjust.

---

## 1. Using the mod

1. Open a level in the editor.
2. Open Auto Deco by any of:
   - the floating green **Deco** button (position configurable in the mod settings),
   - **Pause menu → Auto Deco**,
   - `Ctrl+Shift+D` (`Cmd+Shift+D` on macOS) — rebindable in settings.
3. Pick a preset with the arrows, tweak the options, press **Decorate**.
4. Not happy? Press **Roll** for a new seed and **Decorate** again — with
   *Replace Last* on, the previous result is replaced instead of piling up.
   **Undo Last** and **Remove All** delete what Auto Deco made this session.

| Option | What it does |
|---|---|
| Outlines / Glow | Outline + glow object on every exposed block face |
| Top Decor | Thorns, spikes, rods, grass on upward faces |
| Ground Decor | Same, along the ground line under the layout |
| Hanging Decor | Chains, rods, icicles under platforms/ceilings |
| Side Decor | Thorns growing out of walls |
| Props | Gears, skulls, glow orbs resting on surfaces |
| Background | Pulses, clouds, gears, shapes scattered behind the layout |
| Theme Colors | Adds colour triggers that apply the preset palette |
| Selected Only | Only decorate the blocks you currently have selected |
| Density / Seed | How much gets placed / reproducible randomness |

**About colours.** Presets set their palette with *colour triggers* placed at the
start of the level (they are tagged and removed by Undo like everything else).
Colour triggers only take effect during playtesting / editor preview — in plain
edit mode the new deco may look white/default. That's how GD works, not a bug.
Auto Deco uses 6 colour channels starting at 981 (change in settings if your
level already uses them) plus the built-in BG / Ground / Line / Obj channels.

**Safety.** Everything Auto Deco places is non-solid, non-hazard decoration,
and anything that comes back as a hazard is deleted automatically, so it can
never change how your level plays. Decoration is only added to cells that are
empty of gameplay objects.

**Limits.** Undo/Remove only know about objects made in the *current* editor
session. If you want to be able to wipe deco later, set *Deco Editor Layer* in
the settings so everything lands on its own editor layer.

---

## 2. Getting a `.geode` file

### Option A — GitHub Actions (easiest, no local setup)
1. Create a new GitHub repository and push this folder to it.
2. Open the **Actions** tab → *Build Auto Deco* runs automatically.
3. When finished, download **Build Output** — it contains the `.geode` file
   for Windows, macOS and Android.
4. If a platform fails, open the log: the first compiler error tells you which
   line to fix (see "If it doesn't compile" below).

### Option B — build locally
1. Install the [Geode CLI](https://docs.geode-sdk.org/getting-started/geode-cli)
   and set up the SDK: `geode sdk install` then `geode sdk install-binaries`.
2. From this folder:
   ```
   geode build
   ```
   (or `cmake -B build && cmake --build build --config RelWithDebInfo`).
3. Copy the resulting `yourname.auto-deco.geode` into your Geode `mods` folder
   (or run `geode package install <file>`).

### Before you build
- In `mod.json` change `"id"` (`yourname.auto-deco`) and `"developer"` to your own.
- If your Geode SDK is a newer 5.x, set `"geode"` to your installed version
  (`geode sdk version`) and the `"gd"` versions to the GD version it supports.

---

## 3. Tuning the look (no Geode knowledge needed)

All presets live in **`src/Presets.cpp`** as plain tables:

```cpp
P(123, 2.5f, .9f, 1.2f, 30, Slot::Primary, Slot::Secondary)
//  id  weight scale-range length  main colour  detail colour
```

- `id` – object ID (each has the object's name in a comment).
- `length` – the object's height in GD units (30 = one block). Used to sit
  pieces flush on a surface. If something floats or sinks in-game, change
  this number (or add an `offset`) and rebuild.
- `Slot::…` – which palette colour it uses (Primary, Secondary, Accent, Dark, Light, Glow).
- Add your own preset by copying one of the functions and adding it to `buildAll()`.
- Colours are the `Palette{ … rgb(r,g,b) … }` blocks at the top of each preset.

Presets are *inspired by* the general look of Slaughterhouse, Acheron,
Bloodbath, Nine Circles, etc. — they are not recreations of those levels.

---

## 4. If it doesn't compile

Most likely spots (all small):

| Symptom | Where | Likely fix |
|---|---|---|
| `Popup::init` argument errors | `DecoPopup.cpp` `init()` | Match the signature in the Geode v5 `Popup` docs |
| `ButtonSprite::create` / `Notification::create` overload errors | `DecoPopup.cpp`, `main.cpp` | Adjust to the overloads in your SDK |
| `listenForKeybindSettingPresses` not found | bottom of `main.cpp` | Delete that `$on_game(Loaded)` block and the `open-popup-keybind` setting in `mod.json` — the mod works fine without the hotkey |
| `createObjectsFromString` / `getAllObjects` / `removeObject` | `Bridge.cpp` | Check the signature in the Geode `LevelEditorLayer` docs |
| `GameObjectType::…` | `Bridge.cpp` | Check enum names in your bindings |
| `EditorPauseLayer::customSetup` | `main.cpp` | Delete the pause-layer hook; the floating button + hotkey still work |

## 5. Project layout

```
mod.json              mod metadata + settings
CMakeLists.txt        build script (globs src/*.cpp)
about.md, changelog.md, logo.png
src/main.cpp          editor button, pause-menu button, hotkey
src/DecoPopup.*       the UI window
src/Bridge.*          reads the editor, inserts/removes objects (Geode-facing)
src/Generator.*       decoration algorithm (pure C++, unit-tested)
src/Presets.*         all presets + palettes (pure C++)
tests/                native test for the generator
.github/workflows/    CI build for Windows / macOS / Android
```

Run the native generator test (no Geode needed):
```
g++ -std=c++20 -O1 -g -fsanitize=address,undefined -I src \
    tests/test_generator.cpp src/Generator.cpp src/Presets.cpp -o test_generator
./test_generator
```

MIT licensed — see `LICENSE`.
