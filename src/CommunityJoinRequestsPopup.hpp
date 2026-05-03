#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;

class CommunityJoinRequestsPopup : public geode::Popup {
protected:
    std::string m_communityId;
    std::string m_ownerId;
    CommunityNetwork* m_net = nullptr;
    ScrollLayer* m_scroll = nullptr;
    std::vector<JoinRequestInfo> m_requests;
    std::function<void()> m_onChanged = nullptr;

    bool init(std::string commId, std::string ownerId, std::function<void()> onChanged) {
        if (!Popup::init(320.0f, 220.0f, "GJ_square01.png")) return false;
        m_communityId = commId;
        m_ownerId = ownerId;
        m_onChanged = onChanged;

        this->setTitle("Union Applications", "bigFont.fnt", 0.45f);

        auto bgSize = m_mainLayer->getContentSize();

        auto scrollBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        scrollBg->setContentSize({ 290.0f, 160.0f });
        scrollBg->setColor({ 0, 0, 0 });
        scrollBg->setOpacity(100);
        scrollBg->setPosition({ bgSize.width / 2, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(scrollBg);

        m_scroll = ScrollLayer::create({ 290.0f, 160.0f });
        m_scroll->setPosition({ bgSize.width / 2 - 145.0f, bgSize.height / 2 - 90.0f });
        m_mainLayer->addChild(m_scroll, 1);

        auto scrollbar = Scrollbar::create(m_scroll);
        scrollbar->setPosition({ bgSize.width / 2 + 155.0f, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(scrollbar);

        m_net = CommunityNetwork::create();
        m_net->retain();

        m_net->setOnJoinRequestsLoaded([this](const std::vector<JoinRequestInfo>& requests) {
            m_requests = requests;
            this->buildList();
        });

        m_net->setOnJoinRequestHandled([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Success", msg.c_str(), "OK", nullptr, 300)->show();
            if (m_onChanged) m_onChanged();
            m_net->cargarSolicitudesUnion(m_ownerId, m_communityId);
        });

        m_net->setOnError([this](const std::string& err) {
            FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
        });

        m_net->cargarSolicitudesUnion(m_ownerId, m_communityId);
        return true;
    }

    void buildList() {
        m_scroll->m_contentLayer->removeAllChildren();

        float scrollW = 290.0f;
        float scrollH = 160.0f;
        float itemH = 42.0f;
        float totalH = itemH * m_requests.size();
        if (totalH < scrollH) totalH = scrollH;

        m_scroll->m_contentLayer->setContentSize({ scrollW, totalH });

        if (m_requests.empty()) {
            auto emptyLabel = CCLabelBMFont::create("There are no requests.", "chatFont.fnt");
            emptyLabel->setScale(0.45f);
            emptyLabel->setColor({ 150, 150, 150 });
            emptyLabel->setPosition({ scrollW / 2, scrollH / 2 });
            m_scroll->m_contentLayer->addChild(emptyLabel);
            return;
        }

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        m_scroll->m_contentLayer->addChild(menu);

        auto gm = GameManager::sharedState();
        float currentY = totalH;

        for (size_t i = 0; i < m_requests.size(); i++) {
            currentY -= itemH;
            const auto& req = m_requests[i];

            auto rowBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
            rowBg->setContentSize({ scrollW - 10.0f, itemH - 4.0f });
            rowBg->setColor({ 0, 0, 0 });
            rowBg->setOpacity(70);
            rowBg->setPosition({ scrollW / 2, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(rowBg);

            auto player = SimplePlayer::create(req.icon);
            player->setColor(gm->colorForIdx(req.col1));
            player->setSecondColor(gm->colorForIdx(req.col2));
            if (req.glow) player->setGlowOutline(gm->colorForIdx(req.glow));
            player->setScale(0.5f);
            player->setPosition({ 25.0f, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(player, 1);

            auto nameLabel = CCLabelBMFont::create(req.username.c_str(), "chatFont.fnt");
            nameLabel->setScale(0.4f);
            nameLabel->setAnchorPoint({ 0.0f, 0.5f });
            nameLabel->setPosition({ 50.0f, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(nameLabel, 1);

            auto acceptSpr = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            acceptSpr->setScale(0.55f);
            auto acceptBtn = CCMenuItemSpriteExtra::create(acceptSpr, this, menu_selector(CommunityJoinRequestsPopup::onAccept));
            acceptBtn->setPosition({ scrollW - 55.0f, currentY + itemH / 2 });
            acceptBtn->setTag(static_cast<int>(i));
            menu->addChild(acceptBtn);

            auto rejectSpr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
            rejectSpr->setScale(0.55f);
            auto rejectBtn = CCMenuItemSpriteExtra::create(rejectSpr, this, menu_selector(CommunityJoinRequestsPopup::onReject));
            rejectBtn->setPosition({ scrollW - 25.0f, currentY + itemH / 2 });
            rejectBtn->setTag(static_cast<int>(i));
            menu->addChild(rejectBtn);
        }

        float minY = -(totalH - scrollH);
        if (minY > 0.0f) minY = 0.0f;
        m_scroll->m_contentLayer->setPositionY(minY);

        if (this->isRunning()) {
            geode::cocos::handleTouchPriority(this);
        }
    }

    void onAccept(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = btn->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_requests.size())) return;
        m_net->aceptarSolicitudUnion(m_ownerId, m_communityId, m_requests[idx].accountId);
    }

    void onReject(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = btn->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_requests.size())) return;
        m_net->rechazarSolicitudUnion(m_ownerId, m_communityId, m_requests[idx].accountId);
    }

    void onClose(CCObject* sender) override {
        if (m_net) {
            m_net->setOnJoinRequestsLoaded(nullptr);
            m_net->setOnJoinRequestHandled(nullptr);
            m_net->setOnError(nullptr);
            m_net->release();
            m_net = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static CommunityJoinRequestsPopup* create(std::string commId, std::string ownerId, std::function<void()> onChanged = nullptr) {
        auto ret = new CommunityJoinRequestsPopup();
        if (ret && ret->init(commId, ownerId, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
