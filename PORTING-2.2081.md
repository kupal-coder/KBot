# xdBot — port to GD 2.2081 / Geode 5.10.1

This document summarises everything that was changed to bring the archived
xdBot source (GD 2.2074 / Geode 4.4.0) up to **Geometry Dash 2.2081** and the
**latest stable Geode SDK, v5.10.1** (released 2026-08-29), plus how to build it
and what is still known-risky.

---

## 1. Project metadata

### `mod.json`
| Field | Before | After |
| --- | --- | --- |
| `geode` | `4.4.0` | `5.10.1` |
| `gd.win` / `gd.android` | `2.2074` | `2.2081` |
| `version` | `2.4.1` | `2.5.0` |
| `dependencies` | array form, `geode.custom-keybinds` **required** on Windows | object/map form, `geode.custom-keybinds` **optional** (`"required": false`) |
| `button-setting` | `"type": "custom:button"` (custom setting registered in C++) | `"type": "button"` (built-in Geode 5 setting) |
| keybinds | registered at runtime through the Custom Keybinds API | declared as native `"type": "keybind"` settings (`keybind_open_menu`, `keybind_toggle_recording`, `keybind_toggle_playing`, `keybind_toggle_speedhack`, `keybind_toggle_frame_stepper`, `keybind_step_frame`, `keybind_toggle_render`, `keybind_toggle_noclip`, `keybind_show_trajectory`) with `migrate-from` entries pointing at the old Custom Keybinds IDs |

Custom setting registration (`src/ui/button_setting.cpp`) was deleted; Geode 4's
`custom:` setting API no longer exists in 5.x and the built-in `button` type
covers the exact same use case.

### `CMakeLists.txt`
* `cmake_minimum_required` 3.21 → **3.25** (Geode 5 requirement).
* `CMAKE_CXX_STANDARD` 20 → **23** (Geode 5 headers require C++23).
* Removed `src/ui/button_setting.cpp` from the source list.

### CI (`.github/workflows/*.yml`) — **recommended, not applied**
The workflow files were left untouched because the push credentials used for this
port are not allowed to modify `.github/workflows`. Apply this by hand:

```yaml
      - name: Build the mod
        uses: geode-sdk/build-geode-mod@main
        with:
          sdk: v5.10.1          # <- add: pin the SDK the mod declares
          bindings: geode-sdk/bindings
          bindings-ref: main    # already present: 2.2081 bindings
```

and, if you want branches other than `main` built, add them to the `push.branches`
list of `multi-platform.yml`.

---

## 2. Deprecated / removed Geode APIs replaced

| Old API (Geode ≤4.4) | New API (Geode 5.x) | Where |
| --- | --- | --- |
| `geode::Popup<...>` layout/`initAnchored` differences | `xdbot::Popup<...>` compatibility shim in `src/includes.hpp` + `STATIC_CREATE` calling `initAnchored(w, h, Utils::getTexture())` | all 19 files in `src/ui/` |
| `custom:` settings / `SettingValue` subclasses | built-in `button` setting + `ButtonSettingPressedEventV3(...).listen(...)` | `src/keybinds.cpp`, `src/ui/button_setting.cpp` (deleted) |
| `keybinds::BindManager` (Custom Keybinds API) | `geode::listenForKeybindSettingPresses("keybind_<id>", cb)`; the callback returns `bool` (`true` = consume the input, `false` = propagate). xdBot returns `false`, matching the old `ListenerResult::Propagate`. | `src/keybinds.cpp` |
| reading binds from Custom Keybinds | still supported but **optional**: if the dependency is missing, the native keybind settings are read instead | `src/global.cpp` |
| `listenForSettingChanges("id", cb)` with deduced type | explicit `listenForSettingChanges<std::string>` / `<int64_t>` / `<bool>` | `src/main.cpp` |
| `CCARRAY_FOREACH(arr, obj)` | `for (CCNode* child : CCArrayExt<CCNode*>(arr))` | `load_macro_layer.cpp`, `macro_editor.cpp` (×3), `render_settings_layer.cpp` |
| `file::pick(...).listen(...)` (Task API) | `async::spawn(file::pick(...), [self](file::PickResult res) { ... })` with a `geode::Ref<>` capture and `res.isOk() && res.unwrap().has_value()` | `clickbot_layer.cpp`, `load_macro_layer.cpp` |

---

## 3. 2.2081 game-side changes

Every hook in the mod was diffed against the 2.2074 → 2.2081 bindings by
argument count and return type. The hits that affected xdBot:

| Function | 2.2074 | 2.2081 | Fix |
| --- | --- | --- | --- |
| `GJBaseGameLayer::processCommands` | `(float dt)` | `(float dt, bool isHalfTick, bool isLastTick)` | signature updated in `src/main.cpp` (macro core) and `src/hacks/autoclicker.cpp`; macro logic runs only when `!isHalfTick`, so it still executes exactly once per 240 Hz tick; all three arguments are forwarded to the original |
| `GJBaseGameLayer::getModifiedDelta` | `float` return | `double` return | return type changed in `src/hacks/tps_bypass.cpp` |
| `cocos2d::CCKeyboardDispatcher::dispatchKeyboardMSG` | `(key, down, repeat)` | `(key, down, repeat, double timestamp)` | signature + call to original updated in `src/keybinds.cpp` |
| `CheckpointObject::create` (win-inline) | hooked `init()` on Windows, `create()` elsewhere | `init()` is exported on every platform | single `init()` hook, `#ifdef` removed (`src/practice_fixes/play_layer.cpp`) |
| seed read/write via hardcoded `base::get() + seedAddr` | Windows-only pointer arithmetic | `GameToolbox::fast_srand()` / `GameToolbox::getfast_srand()` bindings (available on all platforms) | `src/global.cpp`, `src/practice_fixes/play_layer.cpp` |

There are **no hardcoded offsets or `geode::base::get()` arithmetic left in the
source** — a repo-wide grep for `0x…`/`base::get()` in `src/` returns nothing.

Signatures that changed in 2.2081 but that xdBot does **not** use (checked, no
action needed): `keyDown`/`keyUp`/`UILayer::handleKeypress` (`double timestamp`),
`PlayerObject::postCollision(float, bool)`, `PlayLayer::toggleDebugDraw()`,
`CCDrawNode::drawPolygon` (the new `BorderAlignment` parameter is defaulted).

All other hooked members (`PlayLayer::init/postUpdate/resetLevel/destroyPlayer/
levelComplete/loadFromCheckpoint/storeCheckpoint/showNewBest/addObject/
setupHasCompleted/playEndAnimationToPos`, `GJBaseGameLayer::update/handleButton/
collisionCheckObjects/canBeActivatedByPlayer/playerTouchedRing/playerTouchedTrigger/
activateSFXTrigger/activateSongEditTrigger/gameEventTriggered/toggleFlipped/
checkpointActivated`, `PlayerObject::update/updateRotation/ringJump/incrementJumps/
playSpiderDashEffect/playDeathEffect/playSpawnEffect`, `PauseLayer::*`,
`EndLevelLayer::*`, `GJGameLevel::savePercentage`, `EffectGameObject::triggerObject`,
`LevelTools::verifyLevelIntegrity`, `FMODAudioEngine::playEffect`,
`CCScheduler::update`, `CCDirector::drawScene`, `CCEGLView::onGLFWMouseMoveCallBack`,
`SliderTouchLogic::ccTouchBegan`, `CCTextInputNode::ccTouchBegan`) were verified
unchanged against the 2.2081 bindings, as were every `m_…` member the mod touches.

---

## 4. Features

Nothing was removed. Feature-by-feature status after the port:

| Feature | Status |
| --- | --- |
| Macro record + playback | Ported (`processCommands` signature change is the only functional touch point) |
| Practice fixes / checkpoints | Ported, now identical on all platforms (`CheckpointObject::init` + `GameToolbox` seed) |
| Seed modifier | Ported, no longer Windows-only pointer math |
| TPS bypass / Speedhack / Frame Stepper | Ported (`getModifiedDelta` return type) |
| Noclip, Safe Mode, Instant Respawn, No Death Effect / Respawn Flash | Unchanged, hooks verified |
| Show Trajectory | Unchanged, hooks verified |
| Layout Mode | Unchanged, hooks verified |
| Renderer (Windows) | Unchanged; still uses an external `ffmpeg.exe` (path configurable in settings) |
| ClickBot / Autoclicker | Ported (`processCommands`) |
| Macro format (.gdr / .gdr.json / .xd) | Unchanged — old macros still load |

---

## 5. Building

```bash
# 1. Install the Geode CLI (>= 3.x) and the SDK
geode sdk install                 # or: git clone https://github.com/geode-sdk/geode
geode sdk update v5.10.1          # pin the same version the mod declares
geode sdk install-binaries        # downloads the prebuilt loader for your platform

export GEODE_SDK=/path/to/geode   # Windows: setx GEODE_SDK C:\path\to\geode

# 2. Configure + build (Windows, from a Developer PowerShell / with clang-cl)
cd KBot
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --config RelWithDebInfo

# 3. The packaged mod lands in build/zilko.xdbot.geode
geode install build/zilko.xdbot.geode
```

Android (needs the Android NDK, `ANDROID_NDK_HOME` set):

```bash
cmake -B build-android64 -DCMAKE_TOOLCHAIN_FILE=$GEODE_SDK/cmake/Android.cmake \
      -DANDROID_ABI=arm64-v8a -DCMAKE_BUILD_TYPE=Release
cmake --build build-android64
```

Requirements: CMake ≥ 3.25, a C++23 compiler (clang 17+/clang-cl; MSVC alone is
not supported by Geode), and bindings from `geode-sdk/bindings@main` (2.2081).
CI (`.github/workflows/multi-platform.yml`) does all of this automatically for
Windows, Android32 and Android64 (add the `sdk: v5.10.1` pin described above).

---

## 6. Known issues / things that could not be verified here

* **The port was not compiled.** The sandbox used for this work has no network
  access to GitHub release assets (they redirect to `objects.githubusercontent.com`,
  which is blocked) and no MSVC/Android NDK, so neither the Geode binaries nor a
  cross-compiler could be installed. Everything above was verified at source
  level against the Geode 5.10.1 headers and the 2.2081 broma bindings, but the
  first real compile should be done through CI (push triggers the workflow) or
  locally. Expect a small number of mechanical compile errors to shake out.
* **Keybind migration**: `migrate-from` moves binds from Custom Keybinds IDs to
  the new native settings. If Custom Keybinds is installed, xdBot still reads the
  old binds; users who had exotic binds should double-check them once.
* **Renderer** is still Windows-only in practice and still depends on an external
  `ffmpeg.exe`. The Camellia 2.7 build additionally supports `eclipse.ffmpeg-api`
  on Android/iOS — that was intentionally *not* pulled in here to keep the change
  minimal.
* **macOS/iOS** are still not declared in `mod.json` (they weren't in the
  original either). The source now compiles-by-inspection on those platforms
  (the Windows-only `#ifdef`s around checkpoints/seed are gone), so adding them
  is mostly a testing exercise.
* Half-tick handling in `processCommands`: macro logic deliberately ignores half
  ticks. This matches 2.2074 behaviour, but levels that rely heavily on the new
  half-tick physics path should be spot-checked for playback drift.
