
#include "includes.hpp"
#include "ui/record_layer.hpp"
#include "ui/game_ui.hpp"
#include "ui/clickbot_layer.hpp"
#include "ui/macro_editor.hpp"
#include "ui/render_settings_layer.hpp"
#include "hacks/layout_mode.hpp"
#include "hacks/show_trajectory.hpp"

#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

// Since Geode v5 keybinds are a built-in setting type, so the mod no longer
// needs the Custom Keybinds API to register its own keybinds. The IDs below
// are the keys of the keybind settings declared in mod.json.
const std::vector<std::string> keybindIDs = {
    "open_menu", "toggle_recording", "toggle_playing",
    "toggle_speedhack", "toggle_frame_stepper", "step_frame",
    "toggle_render", "toggle_noclip", "show_trajectory"
};

class $modify(CCKeyboardDispatcher) {
  // 2.2081 added the timestamp parameter
  bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
  
    auto& g = Global::get();

    int keyInt = static_cast<int>(key);
    if (g.allKeybinds.contains(keyInt) && !isKeyRepeat) {
      for (size_t i = 0; i < 6; i++) {
        if (std::find(g.keybinds[i].begin(), g.keybinds[i].end(), keyInt) != g.keybinds[i].end())
          g.heldButtons[i] = isKeyDown;
      }
    }

    return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
  }
};

void onKeybind(bool down, std::string_view id) {

  auto& g = Global::get();

  if (!down || (LevelEditorLayer::get() && !g.mod->getSettingValue<bool>("editor_keybinds")) || g.mod->getSettingValue<bool>("disable_keybinds"))
    return;

  if (g.state != state::recording && g.mod->getSettingValue<bool>("recording_only_keybinds"))
    return;

  if (id == "open_menu") {
    if (g.layer) {
      static_cast<RecordLayer*>(g.layer)->onClose(nullptr);
      return;
    }

    RecordLayer::openMenu();
  }

  if (id == "toggle_recording")
    Macro::toggleRecording();

  if (id == "toggle_playing")
    Macro::togglePlaying();

  if (id == "toggle_frame_stepper" && PlayLayer::get())
    Global::toggleFrameStepper();

  if (id == "step_frame")
    Global::frameStep();

  if (id == "toggle_speedhack")
    Global::toggleSpeedhack();

  if (id == "show_trajectory") {
    g.mod->setSavedValue("macro_show_trajectory", !g.mod->getSavedValue<bool>("macro_show_trajectory"));

    if (g.layer) {
      if (static_cast<RecordLayer*>(g.layer)->trajectoryToggle)
        static_cast<RecordLayer*>(g.layer)->trajectoryToggle->toggle(g.mod->getSavedValue<bool>("macro_show_trajectory"));
    }

    g.showTrajectory = g.mod->getSavedValue<bool>("macro_show_trajectory");
    if (!g.showTrajectory) ShowTrajectory::trajectoryOff();
  }

#ifdef GEODE_IS_WINDOWS

  if (id == "toggle_render" && PlayLayer::get()) {
    bool result = Renderer::toggle();

    if (result && Global::get().renderer.recording)
      Notification::create("Started Rendering", NotificationIcon::Info)->show();

    if (g.layer) {
      if (static_cast<RecordLayer*>(g.layer)->renderToggle)
        static_cast<RecordLayer*>(g.layer)->renderToggle->toggle(Global::get().renderer.recording);
    }

  }

#endif

  if (id == "toggle_noclip") {
    g.mod->setSavedValue("macro_noclip", !g.mod->getSavedValue<bool>("macro_noclip"));

    if (g.layer) {
      if (static_cast<RecordLayer*>(g.layer)->noclipToggle)
        static_cast<RecordLayer*>(g.layer)->noclipToggle->toggle(g.mod->getSavedValue<bool>("macro_noclip"));
    }
  }

}

$execute{

  for (const std::string& id : keybindIDs) {
    geode::listenForKeybindSettingPresses(
      "keybind_" + id,
      [id](geode::Keybind const&, bool down, bool repeat, double) {
        // The frame stepper keybind is the only one that may repeat
        if (repeat && id != "step_frame") return false;

        onKeybind(down, id);

        // Propagate, like the mod used to do with Custom Keybinds
        return false;
      }
    );
  }

  // Opens the menu from the "Open Menu" button in the mod's settings
  geode::ButtonSettingPressedEventV3(Mod::get(), "button-setting").listen([](std::string_view) {
    if (Global::get().layer) return true;

    if (CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren()) {
      if (FLAlertLayer* layer = typeinfo_cast<FLAlertLayer*>(children->lastObject()))
        layer->keyBackClicked();
    }

    RecordLayer::openMenu();
    return true;
  }).leak();

}
