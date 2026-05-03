#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "StickerManager.hpp"

using namespace geode::prelude;

class ScrollAwareMenu : public CCMenu {
protected:
    CCPoint m_touchStart;
    bool m_isSwiping = false;

public:
    static ScrollAwareMenu* create() {
        auto ret = new ScrollAwareMenu();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    virtual void registerWithTouchDispatcher() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, this->getTouchPriority(), false);
    }

    virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        m_touchStart = touch->getLocation();
        m_isSwiping = false;
        return CCMenu::ccTouchBegan(touch, event);
    }

    virtual void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (ccpDistance(touch->getLocation(), m_touchStart) > 10.0f) {
            m_isSwiping = true;
            if (m_pSelectedItem) {
                m_pSelectedItem->unselected();
                m_pSelectedItem = nullptr;
                m_eState = kCCMenuStateWaiting;
            }
        }
        CCMenu::ccTouchMoved(touch, event);
    }

    virtual void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        if (m_isSwiping) {
            if (m_pSelectedItem) {
                m_pSelectedItem->unselected();
            }
            m_eState = kCCMenuStateWaiting;
            return;
        }
        CCMenu::ccTouchEnded(touch, event);
    }
};

class StickersPopup : public geode::Popup {
protected:
    std::function<void(std::string)> m_callback;
    ScrollLayer* m_scroll = nullptr;
    CCScale9Sprite* m_scrollBG = nullptr;
    Scrollbar* m_scrollbar = nullptr;

    float m_scrollW = 250.0f;
    float m_scrollH = 125.0f;
    float m_scrollY = 0.0f;

    bool init(std::function<void(std::string)> callback) {
        if (!Popup::init(300.f, 220.f, "GJ_square01.png")) return false;

        m_callback = callback;
        this->setTitle("Stickers", "bigFont.fnt", 0.6f);

        auto bgSize = m_mainLayer->getContentSize();
        float centerX = bgSize.width / 2.0f;
        float centerY = bgSize.height / 2.0f;

        m_scrollY = centerY - 12.0f;

        for (int i = 0; i < 2; i++) {
            auto sideArt = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");

            float MedioW = m_bgSprite->getContentSize().width / 2;
            float MedioH = m_bgSprite->getContentSize().height / 2;
            float BgMedioW = sideArt->getContentSize().width / 2;
            float BgMedioH = sideArt->getContentSize().height / 2;

            CCPoint pos;
            switch (i) {
            case 0:
                pos = ccp(centerX - MedioW + BgMedioW, centerY - MedioH + BgMedioH);
                break;
            case 1:
                sideArt->setFlipY(true);
                pos = ccp(centerX - MedioW + BgMedioW, centerY + MedioH - BgMedioH);
                break;
            }

            sideArt->setPosition(pos);
            m_mainLayer->addChild(sideArt);
        }

        m_scrollBG = CCScale9Sprite::create("square02b_001.png");
        m_scrollBG->setColor({ 0, 0, 0 });
        m_scrollBG->setOpacity(80);
        m_scrollBG->setContentSize({ m_scrollW, m_scrollH });
        m_scrollBG->setPosition({ centerX, m_scrollY });
        m_mainLayer->addChild(m_scrollBG);

        loadStickers();

        return true;
    }

    void loadStickers() {
        if (m_scroll) {
            m_scroll->removeFromParent();
            m_scroll = nullptr;
        }
        if (m_scrollbar) {
            m_scrollbar->removeFromParent();
            m_scrollbar = nullptr;
        }

        auto bgSize = m_mainLayer->getContentSize();
        float centerX = bgSize.width / 2.0f;

        std::vector<std::string> commands = StickerManager::getStickerCommands();

        m_scroll = ScrollLayer::create({ m_scrollW, m_scrollH });
        m_scroll->setPosition({
            centerX - m_scrollW / 2.0f,
            m_scrollY - m_scrollH / 2.0f
            });
        m_mainLayer->addChild(m_scroll);

        m_scrollbar = Scrollbar::create(m_scroll);
        m_scrollbar->setPosition({ centerX + m_scrollW / 2.0f + 12.0f, m_scrollY });
        m_mainLayer->addChild(m_scrollbar);

        int columns = 4;
        float cellSize = 55.0f;
        float padX = (m_scrollW - (columns * cellSize)) / 2.0f;

        int rows = (static_cast<int>(commands.size()) + columns - 1) / columns;

        float totalHeight = rows * cellSize;
        if (totalHeight < m_scrollH) totalHeight = m_scrollH;

        m_scroll->m_contentLayer->setContentSize({ m_scrollW, totalHeight });
        auto menu = ScrollAwareMenu::create();

        menu->setContentSize({ m_scrollW, totalHeight });
        menu->setPosition({ 0, 0 });
        m_scroll->m_contentLayer->addChild(menu);

        for (size_t i = 0; i < commands.size(); i++) {
            int col = static_cast<int>(i) % columns;
            int row = static_cast<int>(i) / columns;

            float x = padX + (col * cellSize) + cellSize / 2.0f;
            float y = totalHeight - (row * cellSize) - cellSize / 2.0f;

            auto container = CCSprite::create();
            container->setContentSize({ cellSize, cellSize });

            auto spr = StickerManager::createStickerPreview(commands[i]);
            if (spr) {
                float maxSize = 40.0f;
                auto sprSize = spr->getContentSize();
                float scale = 1.0f;
                if (sprSize.width > 0 && sprSize.height > 0) {
                    float scaleX = maxSize / sprSize.width;
                    float scaleY = maxSize / sprSize.height;
                    scale = std::min(scaleX, scaleY);
                    if (scale > 1.0f) scale = 1.0f;
                }
                spr->setScale(scale);
                spr->setPosition({ cellSize / 2.0f, cellSize / 2.0f });
                container->addChild(spr);
            }

            auto btn = CCMenuItemSpriteExtra::create(container, this, menu_selector(StickersPopup::onSelectSticker));
            btn->setID(commands[i]);
            btn->setPosition({ x, y });
            menu->addChild(btn);
        }

        m_scroll->m_contentLayer->setPositionY(m_scrollH - totalHeight);

        if (this->isRunning()) {
            geode::cocos::handleTouchPriority(this);
        }
    }

    void onInfo(CCObject* sender) {
        std::string Info =
            "Los <cg>Stickers</c> son imagenes para usar en el chat.\n"
            "<cr>Solo se envia el sticker</c>\n"
            "No se puede mezclar con texto.";

        FLAlertLayer::create(nullptr, "Stickers", Info.c_str(), "Okei", nullptr, 360)->show();
    }

    void onSelectSticker(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (btn && m_callback) {
            m_callback(btn->getID());
        }
        this->onClose(sender);
    }

    void onClose(CCObject* sender) override {
        Popup::onClose(sender);
    }

public:
    static StickersPopup* create(std::function<void(std::string)> callback) {
        auto ret = new StickersPopup();
        if (ret && ret->init(callback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};