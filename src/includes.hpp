#pragma once

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/utils/Keyboard.hpp>

#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <cmath>
#include <vector>

#include "renderer/renderer.hpp"
#include "macro.hpp"

using namespace geode::prelude;

const int indexButton[6] = { 1, 2, 3, 1, 2, 3 };

const std::map<int, int> buttonIndex[2] = { { {1, 0}, {2, 1}, {3, 2} }, { {1, 3}, {2, 4}, {3, 5} } };

const int sidesButtons[4] = { 1, 2, 4, 5 };

// Setting keys of the Custom Keybinds mod (geode.custom-keybinds), which since
// Geode v5 exposes the vanilla gameplay keybinds as regular keybind settings.
const std::string buttonIDs[6] = {
    "jump-p1",
    "move-left-p1",
    "move-right-p1",
    "jump-p2",
    "move-left-p2",
    "move-right-p2"
};

// Fallback (vanilla) keys used when the Custom Keybinds mod isn't installed.
const std::vector<cocos2d::enumKeyCodes> defaultButtonKeys[6] = {
    { cocos2d::KEY_Space, cocos2d::KEY_W },
    { cocos2d::KEY_A },
    { cocos2d::KEY_D },
    { cocos2d::KEY_Up },
    { cocos2d::KEY_Left },
    { cocos2d::KEY_Right }
};

const char* const customKeybindsID = "geode.custom-keybinds";

namespace xdbot {

// Geode v5 removed the templated `geode::Popup<Args...>` class in favor of a
// plain, non-templated `geode::Popup`. This is a minimal reimplementation of
// the old API on top of the new one, so that the layers of this mod keep
// working the same way they did before.
template <class... Args>
class Popup : public geode::Popup {
protected:
    virtual bool setup(Args... args) = 0;

    bool initAnchored(
        float width, float height, Args... args,
        char const* bg = "GJ_square01.png", cocos2d::CCRect bgRect = {}
    ) {
        if (!geode::Popup::init(width, height, bg, bgRect)) return false;
        return this->setup(std::forward<Args>(args)...);
    }

public:
    // `onClose` is protected in Geode v5, but this mod closes its layers from
    // the outside (keybinds, macro playback, the renderer, ...)
    using geode::Popup::onClose;
};

}

#define STATIC_CREATE(class, width, height) \
    static class* create() { \
        class* ret = new class(); \
        if (ret->initAnchored(width, height, Utils::getTexture().c_str())) { \
            ret->autorelease(); \
            return ret; \
        } \
        delete ret; \
        return nullptr; \
    }

class Global {

    Global() {}

public:

    static auto& get() {
        static Global instance;
        return instance;
    }

    static bool hasIncompatibleMods();

    static float getTPS();

    static int getCurrentFrame(bool editor = false);

    static void updateKeybinds();

    static void updateSeed(bool isRestart = false);

    static void updatePitch(float value);

    static void toggleSpeedhack();

    static void frameStep();

    static void toggleFrameStepper();

    static void frameStepperOn();

    static void frameStepperOff();

    static PauseLayer* getPauseLayer();

    Mod* mod = Mod::get();
    xdbot::Popup<>* layer = nullptr;

    Macro macro;
    Renderer renderer;
    state state = none;

    std::unordered_map<CheckpointObject*, CheckpointData> checkpoints;
    std::unordered_set<int> allKeybinds;
    std::unordered_set<int> playedFrames;
    std::vector<int> keybinds[6];

    int lastAutoSaveFrame = 0;
    std::chrono::time_point<std::chrono::steady_clock> lastAutoSaveMS = std::chrono::steady_clock::now();
    int currentSession = 0;

    bool stepFrame = false;
    bool stepFrameDraw = false;
    int stepFrameDrawMultiple = 0;
    int stepFrameParticle = 0;
    int frameStepperMusicTime = 0;

    bool cancelCheckpoint = false;
    bool ignoreRecordAction = false;
    bool restart = false;
    bool restartLater = false;
    bool creatingTrajectory = false;
    bool firstAttempt = false;

    bool disableShaders = false;
    bool safeMode = false;
    bool layoutMode = false;
    bool showTrajectory = false;
    bool coinFinder = false;
    bool frameStepper = false;
    bool speedhackEnabled = false;
    bool speedhackAudio = false;
    bool seedEnabled = false;
    bool clickbotEnabled = false;
    bool clickbotOnlyPlaying = false;
    bool clickbotOnlyHolding = false;
    bool frameLabel = false;
    bool trajectoryBothSides = false;
    bool p2mirror = false;
    bool lockDelta = false;
    bool stopPlaying = false;
    bool tpsEnabled = false;
    float tps = 240.f;
    bool previousTpsEnabled = false;
    float previousTps = 0.f;
    bool autoclicker = false;
    bool autoclickerP1 = false;
    bool autoclickerP2 = false;
    int holdFor = 0;
    int releaseFor = 0;
    int holdFor2 = 0;
    int releaseFor2 = 0;
    bool autosaveIntervalEnabled = false;
    int autosaveInterval = 600000;
    float autosaveCheck = 2.f;
    bool autosaveEnabled = false;

    bool ignoreStopDashing[2] = { false, false };
    bool addSideHoldingMembers[2] = { false, false };
    bool wasHolding[6] = { false, false, false, false, false, false };
    bool heldButtons[6] = { false, false, false, false, false, false };

    int delayedFrameRelease[2][2] = { { -1, -1 }, { -1, -1 } };
    int delayedFrameReleaseMain[2] = { -1, -1 };
    int delayedFrameInput[2] = { -1, -1 };
    int ignoreFrame = -1;
    int respawnFrame = -1;
    int ignoreJumpButton = -1;
    int frameOffset = 0;
    int previousFrame = 0;

    size_t currentAction = 0;
    size_t currentFrameFix = 0;
    int frameFixesLimit = 240;
    bool frameFixes = false;
    bool inputFixes = false;

    int currentPage = 0;
    float currentPitch = 1.f;
    uintptr_t latestSeed = 0;
    float leftOver = 0.f;
};