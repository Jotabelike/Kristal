#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;

class CommunityAddMemberPopup : public geode::Popup {
protected:
    TextInput* m_inputId = nullptr;
    std::string m_communityId;
    std::string m_ownerId;
    CommunityNetwork* m_net = nullptr;

    bool init(std::string commId, std::string ownerId) {
        if (!Popup::init(260.0f, 160.0f, "GJ_square01.png")) return false;
        m_communityId = commId;
        m_ownerId = ownerId;

        this->setTitle("Anadir Miembro", "bigFont.fnt", 0.6f);

        m_inputId = TextInput::create(200.0f, "AccountID del usuario...", "chatFont.fnt");
        m_inputId->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 + 10.0f });
        m_inputId->getInputNode()->setAllowedChars("0123456789");
        m_mainLayer->addChild(m_inputId);

        auto btnSpr = ButtonSprite::create("Anadir", "bigFont.fnt", "GJ_button_01.png", 0.8f);
        auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(CommunityAddMemberPopup::onAdd));
        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 35.0f });
        m_mainLayer->addChild(menu);

        m_net = CommunityNetwork::create();
        m_net->retain();
        m_net->setOnMemberAdded([this](const std::string& msg) {
            FLAlertLayer::create(nullptr, "Exito", msg.c_str(), "OK", nullptr, 300)->show();
            this->onClose(nullptr);
            });
        m_net->setOnError([this](const std::string& err) {
            FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
            });

        return true;
    }

    void onAdd(CCObject*) {
        std::string target = m_inputId->getString();
        if (target.empty()) return;
        m_net->agregarMiembroComunidad(m_ownerId, m_communityId, target);
    }

    void onClose(CCObject* sender) override {
        if (m_net) {
            m_net->release();
            m_net = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static CommunityAddMemberPopup* create(std::string commId, std::string ownerId) {
        auto ret = new CommunityAddMemberPopup();
        if (ret && ret->init(commId, ownerId)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};