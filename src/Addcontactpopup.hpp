#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "ContactsNetwork.hpp"

using namespace geode::prelude;

class AddContactPopup : public geode::Popup, public TextInputDelegate {
protected:
    TextInput* m_idInput = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    ScrollLayer* m_requestsScroll = nullptr;
    CCMenu* m_requestsMenu = nullptr;
    ContactsNetwork* m_network = nullptr;
    std::function<void()> m_onContactsChanged;
    std::string m_debugLog = "";

    bool init() override {
        if (!Popup::init(320.f, 240.f, "GJ_square01.png")) return false;

        this->setTitle("Contactos", "bigFont.fnt", 0.55f);

        auto bgSize = m_mainLayer->getContentSize();
        float centerX = bgSize.width / 2;
        float centerY = bgSize.height / 2;

        m_network = ContactsNetwork::create();
        m_network->retain();

        m_network->setLogCallback([this](const std::string& msg) {
            m_debugLog += msg + " | ";
            if (m_statusLabel) {
                m_statusLabel->setString(msg.c_str());
                m_statusLabel->setColor({ 255, 255, 100 });
            }
            });

        m_network->setOnSolicitudSent([this](bool success, const std::string& msg) {
            if (m_statusLabel) {
                m_statusLabel->setString(msg.c_str());
                m_statusLabel->setColor(success ? ccColor3B{ 100, 255, 100 } : ccColor3B{ 255, 100, 100 });
            }
            });

        m_network->setOnRequestsLoaded([this](const std::vector<ContactInfo>& requests) {
            this->mostrarSolicitudes(requests);
            });

        m_network->setOnRequestAccepted([this]() {
            m_network->cargarSolicitudes();
            if (m_onContactsChanged) m_onContactsChanged();
            });

        for (int i = 0; i < 4; i++) {
            auto sideArt = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");

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
                sideArt->setFlipX(true);
                pos = ccp(centerX + MedioW - BgMedioW, centerY - MedioH + BgMedioH);
                break;
            case 2:
                sideArt->setFlipY(true);
                pos = ccp(centerX - MedioW + BgMedioW, centerY + MedioH - BgMedioH);
                break;
            case 3:
                sideArt->setFlipX(true);
                sideArt->setFlipY(true);
                pos = ccp(centerX + MedioW - BgMedioW, centerY + MedioH - BgMedioH);
                break;
            }

            sideArt->setPosition(pos);
            m_mainLayer->addChild(sideArt);
        }

        auto sendSectionLabel = CCLabelBMFont::create("Agregar por ID:", "bigFont.fnt");
        sendSectionLabel->setScale(0.4f);
        sendSectionLabel->setPosition({ centerX, bgSize.height - 55.0f });
        m_mainLayer->addChild(sendSectionLabel);

        m_idInput = TextInput::create(150.0f, "Account ID...", "chatFont.fnt");
        m_idInput->setPosition({ centerX - 30.0f, bgSize.height - 80.0f });
        m_idInput->getInputNode()->setAllowedChars("0123456789");
        m_idInput->setMaxCharCount(30);
        m_mainLayer->addChild(m_idInput);

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        m_mainLayer->addChild(menu);

        auto InfoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        auto InfoBtn = CCMenuItemSpriteExtra::create(InfoSpr, this, menu_selector(AddContactPopup::onInfo));
        InfoBtn->setPosition({ bgSize.width - 5, bgSize.height - 5 });
        InfoBtn->setZOrder(10);
        menu->addChild(InfoBtn);

        auto sendSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
        sendSprite->setScale(0.5f);
        sendSprite->setFlipX(true);
        auto sendBtn = CCMenuItemSpriteExtra::create(sendSprite, this, menu_selector(AddContactPopup::onEnviarSolicitud));
        sendBtn->setPosition({ centerX + 105.0f, bgSize.height - 80.0f });
        menu->addChild(sendBtn);

        m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_statusLabel->setScale(0.45f);
        m_statusLabel->setPosition({ centerX, bgSize.height - 100.0f });
        m_mainLayer->addChild(m_statusLabel);

        auto requestsLabel = CCLabelBMFont::create("Solicitudes Recibidas:", "bigFont.fnt");
        requestsLabel->setScale(0.35f);
        requestsLabel->setPosition({ centerX, bgSize.height - 130.0f });
        m_mainLayer->addChild(requestsLabel);

        float scrollW = 270.0f;
        float scrollH = 80.0f;

        auto scrollBG = CCScale9Sprite::create("square02b_001.png");
        scrollBG->setColor({ 0, 0, 0 });
        scrollBG->setOpacity(80);
        scrollBG->setContentSize({ scrollW, scrollH });
        scrollBG->setPosition({ centerX, bgSize.height - 177.0f });
        m_mainLayer->addChild(scrollBG);

        m_requestsScroll = ScrollLayer::create({ scrollW, scrollH });
        m_requestsScroll->setPosition({
            centerX - scrollW / 2,
            bgSize.height - 177.0f - scrollH / 2
            });
        m_mainLayer->addChild(m_requestsScroll);

        auto emptyLabel = CCLabelBMFont::create("Cargando...", "chatFont.fnt");
        emptyLabel->setScale(0.5f);
        emptyLabel->setPosition({ scrollW / 2, scrollH / 2 });
        emptyLabel->setColor({ 150, 150, 150 });
        emptyLabel->setTag(999);
        m_requestsScroll->m_contentLayer->addChild(emptyLabel);
        m_requestsScroll->m_contentLayer->setContentSize({ scrollW, scrollH });

        m_network->cargarSolicitudes();

        return true;
    }

    void onInfo(CCObject* sender) {
        auto GM2 = GJAccountManager::sharedState();

        std::string Info =
            "Al <co>agregar contactos</c> puedes hablar con ellos.\n"
            "Puedes agregar usando su <cg>AccountID</c> o recibir solicitudes de otros.\n"
            "<cg>Tu AccountID</c>: " + std::to_string(GM2->m_accountID);

        FLAlertLayer::create(nullptr, "Contactos", Info.c_str(), "Okei", nullptr, 360)->show();
    }

    void onEnviarSolicitud(CCObject* sender) {
        std::string targetId = m_idInput->getString();
        if (targetId.empty()) {
            m_statusLabel->setString("Escribe un Account ID.");
            m_statusLabel->setColor({ 255, 255, 100 });
            return;
        }

        auto am = GJAccountManager::sharedState();
        if (targetId == std::to_string(am->m_accountID)) {
            m_statusLabel->setString("No puedes agregarte a ti mismo!");
            m_statusLabel->setColor({ 255, 100, 100 });
            return;
        }

        m_statusLabel->setString("Enviando...");
        m_statusLabel->setColor({ 255, 255, 255 });
        m_network->enviarSolicitud(targetId);
        m_idInput->setString("");
    }

    void mostrarSolicitudes(const std::vector<ContactInfo>& requests) {
        m_requestsScroll->m_contentLayer->removeAllChildren();

        float scrollW = 270.0f;
        float scrollH = 80.0f;

        if (requests.empty()) {
            std::string debugText = m_debugLog.empty() ? "No hay solicitudes." : m_debugLog;
            auto emptyLabel = CCLabelBMFont::create(debugText.c_str(), "chatFont.fnt");
            emptyLabel->setScale(0.3f);
            emptyLabel->setAnchorPoint({ 0.0f, 1.0f });

            float labelH = emptyLabel->getContentSize().height * 0.3f;
            float contentH = labelH > scrollH ? labelH + 10.0f : scrollH;

            m_requestsScroll->m_contentLayer->setContentSize({ scrollW, contentH });
            emptyLabel->setPosition({ 5.0f, contentH - 5.0f });
            emptyLabel->setColor({ 255, 200, 100 });
            m_requestsScroll->m_contentLayer->addChild(emptyLabel);
            return;
        }

        float rowHeight = 35.0f;
        float totalHeight = rowHeight * requests.size();
        if (totalHeight < scrollH) totalHeight = scrollH;

        m_requestsScroll->m_contentLayer->setContentSize({ scrollW, totalHeight });

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        m_requestsScroll->m_contentLayer->addChild(menu);

        float currentY = totalHeight;
        for (const auto& req : requests) {
            currentY -= rowHeight;

            std::string displayText = req.username + " (ID: " + req.accountId + ")";
            auto nameLabel = CCLabelBMFont::create(displayText.c_str(), "chatFont.fnt");
            nameLabel->setScale(0.45f);
            nameLabel->setAnchorPoint({ 0.0f, 0.5f });
            nameLabel->setPosition({ 10.0f, currentY + rowHeight / 2 });
            m_requestsScroll->m_contentLayer->addChild(nameLabel);

            auto acceptSprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            acceptSprite->setScale(0.6f);
            auto acceptBtn = CCMenuItemSpriteExtra::create(acceptSprite, this, menu_selector(AddContactPopup::onAceptar));
            acceptBtn->setPosition({ scrollW - 50.0f, currentY + rowHeight / 2 });
            acceptBtn->setTag(std::stoi(req.accountId.empty() ? "0" : req.accountId));
            menu->addChild(acceptBtn);

            auto rejectSprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
            rejectSprite->setScale(0.6f);
            auto rejectBtn = CCMenuItemSpriteExtra::create(rejectSprite, this, menu_selector(AddContactPopup::onRechazar));
            rejectBtn->setPosition({ scrollW - 20.0f, currentY + rowHeight / 2 });
            rejectBtn->setTag(std::stoi(req.accountId.empty() ? "0" : req.accountId));
            menu->addChild(rejectBtn);
        }

        m_requestsScroll->m_contentLayer->setPositionY(0.0f);
    }

    void onAceptar(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int fromId = btn->getTag();
        if (fromId <= 0) return;
        m_network->aceptarSolicitud(std::to_string(fromId));
    }

    void onRechazar(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        int fromId = btn->getTag();
        if (fromId <= 0) return;
        m_network->rechazarSolicitud(std::to_string(fromId));
    }

    void onClose(CCObject* sender) override {
        if (m_onContactsChanged) {
            m_onContactsChanged();
        }

        if (m_network) {
            m_network->setLogCallback(nullptr);
            m_network->setOnSolicitudSent(nullptr);
            m_network->setOnRequestsLoaded(nullptr);
            m_network->setOnRequestAccepted(nullptr);
            m_network->release();
            m_network = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static AddContactPopup* create(std::function<void()> onChanged = nullptr) {
        auto ret = new AddContactPopup();
        ret->m_onContactsChanged = onChanged;
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};