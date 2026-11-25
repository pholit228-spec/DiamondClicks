#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

namespace {
    bool p1Held = false;
    bool p2Held = false;
    double p1LastClick = -1.0;
    double p2LastClick = -1.0;

    bool modeEnabled(PlayerObject* p) {
        if (!p) return false;
        auto* m = Mod::get();

        if (p->m_isShip) return m->getSettingValue<bool>("ship");
        if (p->m_isBall) return m->getSettingValue<bool>("ball");
        if (p->m_isBird) return m->getSettingValue<bool>("ufo");
        if (p->m_isDart) return m->getSettingValue<bool>("wave");
        if (p->m_isRobot) return m->getSettingValue<bool>("robot");
        if (p->m_isSpider) return m->getSettingValue<bool>("spider");
        if (p->m_isSwing) return m->getSettingValue<bool>("swing");
        if (p->m_isPlatformer) return m->getSettingValue<bool>("platformer");

        return m->getSettingValue<bool>("cube");
    }

    double timeMs() {
        return static_cast<double>(cocos2d::CCDirector::sharedDirector()->getTotalFrames()) *
               cocos2d::CCDirector::sharedDirector()->getAnimationInterval() * 1000.0;
    }

    bool due(double& last) {
        auto delay = Mod::get()->getSettingValue<int64_t>("delay");
        if (delay <= 0) return true;

        double now = timeMs();
        if (last < 0.0 || now - last >= static_cast<double>(delay)) {
            last = now;
            return true;
        }
        return false;
    }

    void click(GJBaseGameLayer* layer, bool p1, double& last) {
        auto* player = p1 ? layer->m_player1 : layer->m_player2;
        if (!player || player->m_isDead || !modeEnabled(player) || !due(last))
            return;

        // Convert the held input into a fresh press every interval.
        layer->handleButton(false, 1, p1);
        layer->handleButton(true, 1, p1);
    }
}

class $modify(DiamondClicksLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool player1) {
        if (button == 1) {
            if (player1) p1Held = down;
            else p2Held = down;
        }

        GJBaseGameLayer::handleButton(down, button, player1);
    }

    void processCommands(float dt, bool p1, bool p2) {
        GJBaseGameLayer::processCommands(dt, p1, p2);

        if (!Mod::get()->getSettingValue<bool>("enabled"))
            return;

        if (p1Held)
            click(this, true, p1LastClick);

        if (p2Held)
            click(this, false, p2LastClick);
    }
};

$on_game(Loaded) {
    p1Held = false;
    p2Held = false;
    p1LastClick = -1.0;
    p2LastClick = -1.0;
    log::info("DiamondClicks by diamondsoup loaded.");
}
