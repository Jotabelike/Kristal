#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;

class CommunityBrowserPopup : public geode::Popup {
protected:
    CommunityNetwork* m_net = nullptr;
    ScrollLayer* m_scroll = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    std::vector<CommunityInfo> m_communities;
    std::function<void()> m_onChanged = nullptr;

    bool init(std::function<void()> onChanged) {
        if (!Popup::init(380.f, 260.f, "GJ_square01.png")) return false;
        m_onChanged = onChanged;

        this->setTitle("Explore Communities", "bigFont.fnt", 0.5f);

        auto bgSize = m_mainLayer->getContentSize();
        float centerX = bgSize.width / 2;

        m_net = CommunityNetwork::create();
        m_net->retain();

        m_net->setOnAllCommunitiesLoaded([this](const std::vector<CommunityInfo>& list) {
            m_communities = list;
            this->buildList();
            });

        m_net->setOnJoinResult([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Success", msg.c_str(), "OK", nullptr, 300)->show();
            if (m_onChanged) m_onChanged();
            this->reload();
            });

        m_net->setOnJoinRequestSent([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Application", msg.c_str(), "OK", nullptr, 300)->show();
            this->reload();
            });

        m_net->setOnError([this](const std::string& err) {
            FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
            });

        auto scrollBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        scrollBg->setContentSize({ 350.0f, 195.0f });
        scrollBg->setColor({ 0, 0, 0 });
        scrollBg->setOpacity(100);
        scrollBg->setPosition({ centerX, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(scrollBg);

        m_scroll = ScrollLayer::create({ 350.0f, 195.0f });
        m_scroll->setPosition({ centerX - 175.0f, bgSize.height / 2 - 107.5f });
        m_mainLayer->addChild(m_scroll, 1);

        auto scrollbar = Scrollbar::create(m_scroll);
        scrollbar->setPosition({ centerX + 185.0f, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(scrollbar);

        m_statusLabel = CCLabelBMFont::create("Loading...", "chatFont.fnt");
        m_statusLabel->setScale(0.45f);
        m_statusLabel->setColor({ 200, 200, 200 });
        m_statusLabel->setPosition({ centerX, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(m_statusLabel, 2);

        this->reload();
        return true;
    }

    void reload() {
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);
        m_net->cargarTodasComunidades(myId);
        if (m_statusLabel) m_statusLabel->setVisible(true);
    }

    void buildList() {
        m_scroll->m_contentLayer->removeAllChildren();
        if (m_statusLabel) m_statusLabel->setVisible(false);

        float scrollW = 350.0f;
        float scrollH = 195.0f;
        float itemH = 50.0f;
        float totalH = itemH * m_communities.size();
        if (totalH < scrollH) totalH = scrollH;

        m_scroll->m_contentLayer->setContentSize({ scrollW, totalH });

        if (m_communities.empty()) {
            auto emptyLabel = CCLabelBMFont::create("There are no communities created.", "chatFont.fnt");
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

        for (size_t i = 0; i < m_communities.size(); i++) {
            currentY -= itemH;
            const auto& comm = m_communities[i];

            auto rowBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
            rowBg->setContentSize({ scrollW - 10.0f, itemH - 4.0f });
            rowBg->setColor({ 0, 0, 0 });
            rowBg->setOpacity(70);
            rowBg->setPosition({ scrollW / 2, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(rowBg);

            auto player = SimplePlayer::create(comm.icon);
            player->setColor(gm->colorForIdx(comm.col1));
            player->setSecondColor(gm->colorForIdx(comm.col2));
            if (comm.glow) player->setGlowOutline(gm->colorForIdx(comm.glow));
            player->setScale(0.55f);
            player->setPosition({ 30.0f, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(player, 1);

            auto nameLabel = CCLabelBMFont::create(comm.name.c_str(), "bigFont.fnt");
            nameLabel->setScale(0.35f);
            nameLabel->setAnchorPoint({ 0.0f, 0.5f });
            nameLabel->setPosition({ 55.0f, currentY + itemH / 2 + 10.0f });
            m_scroll->m_contentLayer->addChild(nameLabel, 1);

            std::string infoText = (comm.isPublic ? "Public" : "Private");
            infoText += " | " + std::to_string(comm.memberCount) + " members";
            auto infoLabel = CCLabelBMFont::create(infoText.c_str(), "chatFont.fnt");
            infoLabel->setScale(0.3f);
            infoLabel->setAnchorPoint({ 0.0f, 0.5f });
            infoLabel->setPosition({ 55.0f, currentY + itemH / 2 - 8.0f });
            infoLabel->setColor({ 180, 180, 180 });
            m_scroll->m_contentLayer->addChild(infoLabel, 1);

            auto descSpr = CCSprite::createWithSpriteFrameName("rule_group.png"_spr);
            descSpr->setScale(0.6f);
            auto descBtn = CCMenuItemSpriteExtra::create(descSpr, this, menu_selector(CommunityBrowserPopup::onShowDescription));
            descBtn->setPosition({ scrollW - 80.0f, currentY + itemH / 2 });
            descBtn->setTag(static_cast<int>(i));
            menu->addChild(descBtn);

            if (comm.isMember) {
                auto joinedLabel = CCLabelBMFont::create("Member", "bigFont.fnt");
                joinedLabel->setScale(0.25f);
                joinedLabel->setColor({ 100, 255, 100 });
                joinedLabel->setPosition({ scrollW - 40.0f, currentY + itemH / 2 });
                m_scroll->m_contentLayer->addChild(joinedLabel, 1);
            }
            else if (comm.hasPending) {
                auto pendingLabel = CCLabelBMFont::create("Pending", "bigFont.fnt");
                pendingLabel->setScale(0.25f);
                pendingLabel->setColor({ 255, 255, 100 });
                pendingLabel->setPosition({ scrollW - 40.0f, currentY + itemH / 2 });
                m_scroll->m_contentLayer->addChild(pendingLabel, 1);
            }
            else {
                CCSprite* btnSpr = nullptr;
                if (comm.isPublic) {
                    btnSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
                    btnSpr->setScale(0.35f);
                }
                else {
                    btnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
                    btnSpr->setScale(0.35f);
                    btnSpr->setFlipX(true);
                }
                auto joinBtn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(CommunityBrowserPopup::onJoin));
                joinBtn->setPosition({ scrollW - 40.0f, currentY + itemH / 2 });
                joinBtn->setTag(static_cast<int>(i));
                menu->addChild(joinBtn);
            }
        }

        float minY = -(totalH - scrollH);
        if (minY > 0.0f) minY = 0.0f;
        m_scroll->m_contentLayer->setPositionY(minY);

        if (this->isRunning()) {
            geode::cocos::handleTouchPriority(this);
        }
    }

    void onShowDescription(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = btn->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_communities.size())) return;

        const auto& comm = m_communities[idx];
        std::string desc = comm.description.empty() ? "No description." : comm.description;
        std::string info = "<cg>" + comm.name + "</c>\n\n" + desc
            + "\n\n<cy>Members</c>: " + std::to_string(comm.memberCount)
            + "\n<cy>Tipe</c>: " + (comm.isPublic ? "Public" : "Private");

        FLAlertLayer::create(nullptr, "Community", info.c_str(), "Ok", nullptr, 320)->show();
    }

    void onJoin(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = btn->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_communities.size())) return;

        const auto& comm = m_communities[idx];
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);

        if (comm.isPublic) {
            m_net->unirseComunidad(myId, comm.communityId);
        }
        else {
            m_net->enviarSolicitudUnion(myId, comm.communityId);
        }
    }

    void onClose(CCObject* sender) override {
        if (m_net) {
            m_net->setOnAllCommunitiesLoaded(nullptr);
            m_net->setOnJoinResult(nullptr);
            m_net->setOnJoinRequestSent(nullptr);
            m_net->setOnError(nullptr);
            m_net->release();
            m_net = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static CommunityBrowserPopup* create(std::function<void()> onChanged = nullptr) {
        auto ret = new CommunityBrowserPopup();
        if (ret && ret->init(onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};