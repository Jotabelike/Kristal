#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;



class CommunityInvitesPopup : public geode::Popup, public FLAlertLayerProtocol {
protected:
    ScrollLayer* m_scrollLayer = nullptr;
    CommunityNetwork* m_net = nullptr;
    std::function<void()> m_onCloseCallback;

    bool init(std::function<void()> onCloseCallback) {
        if (!Popup::init(380.0f, 240.0f, "GJ_square01.png")) return false;
        m_onCloseCallback = onCloseCallback;

        this->setTitle("Community Invites", "bigFont.fnt", 0.6f);

        auto bgSize = m_mainLayer->getContentSize();

        auto bg = CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
        bg->setColor({ 0, 0, 0 });
        bg->setOpacity(100);
        bg->setContentSize({ 340.0f, 150.0f });
        bg->setPosition({ bgSize.width / 2, bgSize.height / 2 - 10.0f });
        m_mainLayer->addChild(bg);

        m_scrollLayer = ScrollLayer::create({ 340.0f, 150.0f });
        m_scrollLayer->setPosition({ bgSize.width / 2 - 170.0f, bgSize.height / 2 - 85.0f });
        m_mainLayer->addChild(m_scrollLayer);

        m_net = CommunityNetwork::create();
        m_net->retain();
        m_net->setOnInvitesLoaded([this](const std::vector<CommunityInviteInfo>& invites) {
            this->populateList(invites);
            });
        m_net->setOnError([](const std::string& err) {
            FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
            });

        loadInvites();

        return true;
    }

    void loadInvites() {
        auto am = GJAccountManager::sharedState();
        m_net->cargarInvitaciones(std::to_string(am->m_accountID));
    }

    void populateList(const std::vector<CommunityInviteInfo>& invites) {
        m_scrollLayer->m_contentLayer->removeAllChildren();
        if (invites.empty()) {
            auto lbl = CCLabelBMFont::create("No pending invites.", "chatFont.fnt");
            lbl->setPosition({ 170.0f, 75.0f });
            m_scrollLayer->m_contentLayer->addChild(lbl);
            return;
        }

        float spacing = 45.0f;
        float totalHeight = invites.size() * spacing;
        if (totalHeight < 150.0f) totalHeight = 150.0f;
        m_scrollLayer->m_contentLayer->setContentSize({ 340.0f, totalHeight });

        auto menu = CCMenu::create();
        menu->setContentSize({ 340.0f, totalHeight });
        menu->setPosition({ 0, 0 });
        m_scrollLayer->m_contentLayer->addChild(menu);

        auto gm = GameManager::sharedState();
        float currentY = totalHeight - 25.0f;

        for (const auto& invite : invites) {
         
            auto iconSpr = SimplePlayer::create(invite.icon);
            iconSpr->setColor(gm->colorForIdx(invite.col1));
            iconSpr->setSecondColor(gm->colorForIdx(invite.col2));
            if (invite.glow) {
                iconSpr->setGlowOutline(gm->colorForIdx(invite.glow));
            }
            iconSpr->setScale(0.55f);
            iconSpr->setPosition({ 25.0f, currentY });
            m_scrollLayer->m_contentLayer->addChild(iconSpr);

            // 2. Nombre de la comunidad (movido a la derecha y con límite de ancho)
            auto nameLbl = CCLabelBMFont::create(invite.name.c_str(), "chatFont.fnt");
            nameLbl->setAnchorPoint({ 0.0f, 0.5f });
            nameLbl->setPosition({ 45.0f, currentY });
            // Esto evita que nombres muy largos tapen los botones
            nameLbl->limitLabelWidth(140.0f, 1.0f, 0.1f);
            m_scrollLayer->m_contentLayer->addChild(nameLbl);

            // 3. Botón de Aceptar (Escalado y reposicionado)
            auto acceptSpr = ButtonSprite::create("Accept", "bigFont.fnt", "GJ_button_01.png", 0.5f);
            auto acceptBtn = CCMenuItemSpriteExtra::create(acceptSpr, this, menu_selector(CommunityInvitesPopup::onAccept));
            acceptBtn->setPosition({ 230.0f, currentY });
            acceptBtn->setScale(0.65f); // Reducimos el tamaño general del botón
            acceptBtn->setUserObject(CCString::create(invite.communityId));
            menu->addChild(acceptBtn);

            // 4. Botón de Rechazar (Escalado y reposicionado)
            auto rejectSpr = ButtonSprite::create("Reject", "bigFont.fnt", "GJ_button_06.png", 0.5f);
            auto rejectBtn = CCMenuItemSpriteExtra::create(rejectSpr, this, menu_selector(CommunityInvitesPopup::onReject));
            rejectBtn->setPosition({ 295.0f, currentY });
            rejectBtn->setScale(0.65f); // Reducimos el tamaño general del botón
            rejectBtn->setUserObject(CCString::create(invite.communityId));
            menu->addChild(rejectBtn);

            currentY -= spacing;
        }
        m_scrollLayer->m_contentLayer->setPositionY(150.0f - totalHeight);
    }

    void onAccept(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto strObj = static_cast<CCString*>(btn->getUserObject());
        if (!strObj) return;

        std::string commId = strObj->getCString();
        auto am = GJAccountManager::sharedState();

        m_net->setOnInviteHandled([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Success", msg.c_str(), "OK", nullptr, 300)->show();
            this->loadInvites();
            });
        m_net->aceptarInvitacion(std::to_string(am->m_accountID), commId);
    }

    void onReject(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto strObj = static_cast<CCString*>(btn->getUserObject());
        if (!strObj) return;

        std::string commId = strObj->getCString();
        auto am = GJAccountManager::sharedState();

        m_net->setOnInviteHandled([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Success", msg.c_str(), "OK", nullptr, 300)->show();
            this->loadInvites();
            });
        m_net->rechazarInvitacion(std::to_string(am->m_accountID), commId);
    }

    void onClose(CCObject* sender) override {
        if (m_onCloseCallback) m_onCloseCallback();
        if (m_net) {
            m_net->release();
            m_net = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static CommunityInvitesPopup* create(std::function<void()> onCloseCallback) {
        auto ret = new CommunityInvitesPopup();
        if (ret && ret->init(onCloseCallback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};