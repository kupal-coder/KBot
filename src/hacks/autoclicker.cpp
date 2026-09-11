#include "../includes.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(GJBaseGameLayer) {

    struct Fields {
        int framesHolding[2] = {0, 0}; 
        int framesReleasing[2] = {0, 0};
        bool autoclick[2] = {false, false};
        bool holding[2] = {false, false};
    };

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

        auto& g = Global::get();

        if (isHalfTick) return;
        if (!g.autoclicker) return;
        if (!PlayLayer::get()) return;

        auto& f = m_fields;
        const int holdFor[2] = {g.holdFor, g.holdFor2};
        const int releaseFor[2] = {g.releaseFor, g.releaseFor2};

        for (int i = 0; i < 2; i++) { // check 2 player mode
            bool player2 = (i == 1);
            if ((!g.autoclickerP1 && !player2) || (!g.autoclickerP2 && player2)) continue;

            if (f->framesReleasing[i] < (releaseFor[i] - 1) / (f->holding[i] ? 2.f : 1.f)) {
                f->framesReleasing[i]++;
                continue;
            }

            if (f->framesHolding[i] < holdFor[i]) {
                if (f->framesHolding[i] == 0) {
                    f->autoclick[i] = true;
                    GJBaseGameLayer::handleButton(true, 1, Macro::flipControls() ? !player2 : player2);
                    f->autoclick[i] = false;
                }

                f->framesHolding[i]++;
            }
            else {
                f->autoclick[i] = true;
                GJBaseGameLayer::handleButton(false, 1, Macro::flipControls() ? !player2 : player2);
                f->autoclick[i] = false;

                f->framesReleasing[i] = 0;
                f->framesHolding[i] = 0;
            }
        }

    }

    void handleButton(bool hold, int button, bool player2) {
        if (button != 1) return GJBaseGameLayer::handleButton(hold, button, player2);

        auto& g = Global::get();
        if (!g.autoclicker) return GJBaseGameLayer::handleButton(hold, button, player2);

        auto& f = m_fields;
        bool realPlayer2 = Macro::flipControls() ? !player2 : player2;
        int i = realPlayer2 ? 1 : 0;

        if (f->autoclick[i] || (!realPlayer2 && !g.autoclickerP1) || (realPlayer2 && !g.autoclickerP2)) return GJBaseGameLayer::handleButton(hold, button, player2);

        f->holding[i] = hold;

        if (!hold) {
            f->framesHolding[i] = 0;
            f->framesReleasing[i] = 0;
            GJBaseGameLayer::handleButton(hold, button, player2);
        }

    }

};