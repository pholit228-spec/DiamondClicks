#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

namespace {
    bool p1Held = false;
    bool p2Held = false;

    // Synthetic click state. We keep the generated press alive for one
    // physics tick instead of doing release+press in the same tick.
    bool p1SyntheticHeld = false;
    bool p2SyntheticHeld = false;

    double p1LastClick = -1.0;
    double p2LastClick = -1.0;

    bool modeEnabled(GJBaseGameLayer* layer, PlayerObject* p) {
        if (!p) return false;

        auto* m = Mod::get();

        if (layer->m_levelSettings && layer->m_levelSettings->m_platformerMode)
            return m->getSettingValue<bool>("platformer");

        if (p->m_isShip) return m->getSettingValue<bool>("ship");
        if (p->m_isBall) return m->getSettingValue<bool>("ball");
        if (p->m_isBird) return m->getSettingValue<bool>("ufo");
        if (p->m_isDart) return m->getSettingValue<bool>("wave");
        if (p->m_isRobot) return m->getSettingValue<bool>("robot");
        if (p->m_isSpider) return m->getSettingValue<bool>("spider");
        if (p->m_isSwing) return m->getSettingValue<bool>("swing");

        return m->getSettingValue<bool>("cube");
    }

    double timeMs() {
        return static_cast<double>(
            cocos2d::CCDirector::get()->getTotalFrames()
        ) * cocos2d::CCDirector::get()->getAnimationInterval() * 1000.0;
    }

    bool due(double& last) {
        auto delay = Mod::get()->getSettingValue<int64_t>("delay");

        if (delay <= 0)
            return true;

        const double now = timeMs();

        if (last < 0.0 || now - last >= static_cast<double>(delay)) {
            last = now;
            return true;
        }

        return false;
    }

    void releaseSynthetic(GJBaseGameLayer* layer, bool p1, bool& syntheticHeld) {
        if (!syntheticHeld)
            return;

        layer->handleButton(false, 1, p1);
        syntheticHeld = false;
    }

    void click(GJBaseGameLayer* layer, bool p1, bool& lastSyntheticHeld, double& lastClick) {
        auto* player = p1 ? layer->m_player1 : layer->m_player2;

        if (!player || player->m_isDead || !modeEnabled(layer, player))
            return;

        if (lastSyntheticHeld)
            return;

        if (!due(lastClick))
            return;

        // Generate a real in-game press. It remains held until the next
        // physics tick, where it is released before another press is made.
        layer->handleButton(true, 1, p1);
        lastSyntheticHeld = true;
    }
}

class $modify(DiamondClicksLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool player1) {
        if (button == 1) {
            if (player1)
                p1Held = down;
            else
                p2Held = down;
        }

        GJBaseGameLayer::handleButton(down, button, player1);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

        if (!Mod::get()->getSettingValue<bool>("enabled"))
            return;

        // The generated press is released on the following command tick.
        // This avoids putting release+press on the same physics tick.
        releaseSynthetic(this, true, p1SyntheticHeld);
        releaseSynthetic(this, false, p2SyntheticHeld);

        if (p1Held)
            click(this, true, p1SyntheticHeld, p1LastClick);

        if (p2Held)
            click(this, false, p2SyntheticHeld, p2LastClick);
    }
};

$on_game(Loaded) {
    p1Held = false;
    p2Held = false;
    p1SyntheticHeld = false;
    p2SyntheticHeld = false;
    p1LastClick = -1.0;
    p2LastClick = -1.0;

    log::info("DiamondClicks by diamondsoup loaded.");
}
