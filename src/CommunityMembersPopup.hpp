#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;

class CommunityMembersPopup : public geode::Popup, public FLAlertLayerProtocol {
protected:
    std::string m_communityId;
    std::string m_ownerId;
    CommunityNetwork* m_net = nullptr;
    ScrollLayer* m_scroll = nullptr;
    std::vector<MemberInfo> m_members;
    std::function<void()> m_onChanged = nullptr;

    bool init(std::string commId, std::string ownerId, std::function<void()> onChanged) {
        if (!Popup::init(300.0f, 220.0f, "GJ_square01.png")) return false;
        m_communityId = commId;
        m_ownerId = ownerId;
        m_onChanged = onChanged;

        this->setTitle("Miembros", "bigFont.fnt", 0.6f);

        auto bgSize = m_mainLayer->getContentSize();

        auto scrollBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        scrollBg->setContentSize({ 270.0f, 160.0f });
        scrollBg->setColor({ 0, 0, 0 });
        scrollBg->setOpacity(100);
        scrollBg->setPosition({ bgSize.width / 2, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(scrollBg);

        m_scroll = ScrollLayer::create({ 270.0f, 160.0f });
        m_scroll->setPosition({ bgSize.width / 2 - 135.0f, bgSize.height / 2 - 90.0f });
        m_mainLayer->addChild(m_scroll, 1);

        auto scrollbar = Scrollbar::create(m_scroll);
        scrollbar->setPosition({ bgSize.width / 2 + 145.0f, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(scrollbar);

        m_net = CommunityNetwork::create();
        m_net->retain();
        m_net->setOnMembersLoaded([this](const std::vector<MemberInfo>& members) {
            m_members = members;
            this->buildList();
            });
        m_net->setOnMemberRemoved([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Exito", msg.c_str(), "OK", nullptr, 300)->show();
            if (m_onChanged) m_onChanged();
            m_net->listarMiembros(m_communityId);
            });
        m_net->setOnError([this](const std::string& err) {
            FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
            });

        m_net->listarMiembros(m_communityId);
        return true;
    }

    void buildList() {
        m_scroll->m_contentLayer->removeAllChildren();

        float itemH = 40.0f;
        float scrollW = 270.0f;
        float scrollH = 160.0f;
        float totalH = itemH * m_members.size();
        if (totalH < scrollH) totalH = scrollH;

        m_scroll->m_contentLayer->setContentSize({ scrollW, totalH });

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        m_scroll->m_contentLayer->addChild(menu);

        auto gm = GameManager::sharedState();
        float currentY = totalH;

        for (size_t i = 0; i < m_members.size(); i++) {
            currentY -= itemH;
            const auto& member = m_members[i];

            auto rowBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
            rowBg->setContentSize({ scrollW - 10.0f, itemH - 4.0f });
            rowBg->setColor({ 0, 0, 0 });
            rowBg->setOpacity(80);
            rowBg->setPosition({ scrollW / 2, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(rowBg);

            auto player = SimplePlayer::create(member.icon);
            player->setColor(gm->colorForIdx(member.col1));
            player->setSecondColor(gm->colorForIdx(member.col2));
            if (member.glow) {
                player->setGlowOutline(gm->colorForIdx(member.glow));
            }
            player->setScale(0.55f);
            player->setPosition({ 25.0f, currentY + itemH / 2 });
            m_scroll->m_contentLayer->addChild(player, 1);

            std::string displayName = member.username;
            if (member.role == "owner") displayName += " (Admin)";
            auto nameLabel = CCLabelBMFont::create(displayName.c_str(), "chatFont.fnt");
            nameLabel->setScale(0.45f);
            nameLabel->setAnchorPoint({ 0.0f, 0.5f });
            nameLabel->setPosition({ 50.0f, currentY + itemH / 2 });
            if (member.role == "owner") {
                nameLabel->setColor({ 255, 215, 0 });
            }
            m_scroll->m_contentLayer->addChild(nameLabel, 1);

         
            if (member.role != "owner" && member.accountId != m_ownerId) {
                auto kickSpr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
                kickSpr->setScale(0.55f);
                auto kickBtn = CCMenuItemSpriteExtra::create(kickSpr, this, menu_selector(CommunityMembersPopup::onKickMember));
                kickBtn->setPosition({ scrollW - 25.0f, currentY + itemH / 2 });
                kickBtn->setTag(static_cast<int>(i));
                menu->addChild(kickBtn);
            }
        }

        float minY = -(totalH - scrollH);
        if (minY > 0.0f) minY = 0.0f;
        m_scroll->m_contentLayer->setPositionY(minY);

        if (this->isRunning()) {
            geode::cocos::handleTouchPriority(this);
        }
    }

    void onKickMember(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int idx = btn->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_members.size())) return;

        const auto& member = m_members[idx];
        std::string msg = "Expulsar a <cr>" + member.username + "</c> de la comunidad?";

        auto confirm = FLAlertLayer::create(this, "Confirmar", msg.c_str(), "Cancelar", "Expulsar", 300);
        confirm->setTag(idx);
        confirm->show();
    }

    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        if (!btn2) return;
        int idx = alert->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_members.size())) return;
        m_net->expulsarMiembro(m_ownerId, m_communityId, m_members[idx].accountId);
    }

    void onClose(CCObject* sender) override {
        if (m_net) {
            m_net->release();
            m_net = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static CommunityMembersPopup* create(std::string commId, std::string ownerId, std::function<void()> onChanged = nullptr) {
        auto ret = new CommunityMembersPopup();
        if (ret && ret->init(commId, ownerId, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};