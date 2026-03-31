#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;

class CommunityPopup : public geode::Popup {
protected:
    TextInput* m_nameInput = nullptr;
    TextInput* m_descInput = nullptr;
    ScrollLayer* m_iconScroll = nullptr;

    CommunityNetwork* m_communityNet = nullptr;

    int m_selectedIcon = 1;
    int m_selectedCol1 = 0;
    int m_selectedCol2 = 3;
    int m_selectedGlow = 0;
    bool m_isPublic = true;

    SimplePlayer* m_previewPlayer = nullptr;

    CCLabelBMFont* m_visibilityStatusLabel = nullptr;
    CCMenuItemToggler* m_visibilityToggle = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;

    std::function<void()> m_onCreated = nullptr;

    bool init(std::function<void()> onCreated) {
        if (!Popup::init(400.f, 280.f, "GJ_square01.png")) return false;
        m_onCreated = onCreated;

        this->setTitle("Crear Comunidad", "bigFont.fnt", 0.6f);

        auto bgSize = m_mainLayer->getContentSize();
        float centerX = bgSize.width / 2;
        float centerY = bgSize.height / 2;

       
        for (int i = 0; i < 4; i++) {
            auto sideArt = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
            float hw = m_bgSprite->getContentSize().width / 2;
            float hh = m_bgSprite->getContentSize().height / 2;
            float sw = sideArt->getContentSize().width / 2;
            float sh = sideArt->getContentSize().height / 2;
            CCPoint pos;
            switch (i) {
            case 0: pos = ccp(centerX - hw + sw, centerY - hh + sh); break;
            case 1: sideArt->setFlipX(true); pos = ccp(centerX + hw - sw, centerY - hh + sh); break;
            case 2: sideArt->setFlipY(true); pos = ccp(centerX - hw + sw, centerY + hh - sh); break;
            case 3: sideArt->setFlipX(true); sideArt->setFlipY(true); pos = ccp(centerX + hw - sw, centerY + hh - sh); break;
            }
            sideArt->setPosition(pos);
            m_mainLayer->addChild(sideArt);
        }

      
        m_communityNet = CommunityNetwork::create();
        m_communityNet->retain();

        m_communityNet->setOnCommunityCreated([this](const CommunityInfo& info) {
            FLAlertLayer::create(nullptr, "Comunidad Creada", ("Tu comunidad <cg>" + info.name + "</c> fue creada!").c_str(), "Genial!", nullptr, 320)->show();
            if (m_onCreated) m_onCreated();
            });
        m_communityNet->setOnError([this](const std::string& err) { if (m_statusLabel) { m_statusLabel->setString(err.c_str()); m_statusLabel->setColor({ 255, 80, 80 }); } });
        m_communityNet->setLogCallback([this](const std::string& msg) { if (m_statusLabel) { m_statusLabel->setString(msg.c_str()); m_statusLabel->setColor({ 255, 255, 100 }); } });

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        m_mainLayer->addChild(menu, 2);

        auto gm = GameManager::sharedState();
        m_selectedCol1 = gm->getPlayerColor();
        m_selectedCol2 = gm->getPlayerColor2();
        m_selectedGlow = gm->getPlayerGlow() ? gm->getPlayerGlowColor() : 0;

       
        float leftX = 105.0f;
        float rightX = 295.0f;
        float topY = bgSize.height - 50.0f;

       

        auto previewBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        previewBg->setContentSize({ 50.0f, 50.0f });
        previewBg->setColor({ 0, 0, 0 });
        previewBg->setOpacity(100);
        previewBg->setPosition({ leftX, topY - 15.0f });
        m_mainLayer->addChild(previewBg);

 
        m_previewPlayer = SimplePlayer::create(m_selectedIcon);
        m_previewPlayer->setColor(gm->colorForIdx(gm->getPlayerColor()));
        m_previewPlayer->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));
        if (gm->getPlayerGlow()) {
            m_previewPlayer->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
        }
        m_previewPlayer->updateColors(); 
        m_previewPlayer->setPosition({ leftX, topY - 15.0f });
        m_mainLayer->addChild(m_previewPlayer, 1);

        auto previewLabel = CCLabelBMFont::create("Icono", "goldFont.fnt");
        previewLabel->setScale(0.4f);
        previewLabel->setPosition({ leftX, topY + 15.0f });
        m_mainLayer->addChild(previewLabel);

        
        float gridW = 140.0f;
        float gridH = 120.0f;
        float gridCenterY = topY - 105.0f;

        auto gridBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        gridBg->setContentSize({ gridW, gridH });
        gridBg->setColor({ 0, 0, 0 });
        gridBg->setOpacity(100);
        gridBg->setPosition({ leftX, gridCenterY });
        m_mainLayer->addChild(gridBg);

        m_iconScroll = ScrollLayer::create({ gridW, gridH });
        m_iconScroll->setPosition({ leftX - gridW / 2, gridCenterY - gridH / 2 });
        m_mainLayer->addChild(m_iconScroll, 1);

        auto gridScrollbar = Scrollbar::create(m_iconScroll);
        gridScrollbar->setPosition({ leftX + gridW / 2 + 8.0f, gridCenterY });
        m_mainLayer->addChild(gridScrollbar);

        buildIconGrid();

      
        float inputW = 170.0f;
        float fieldY = topY + 5.0f;

        // --- Nombre ---
        auto nameLabel = CCLabelBMFont::create("Nombre", "goldFont.fnt");
        nameLabel->setScale(0.4f);
        nameLabel->setPosition({ rightX, fieldY });
        m_mainLayer->addChild(nameLabel);

        auto nameBg = CCScale9Sprite::create("square02_small.png", { 0, 0, 40, 40 });
        nameBg->setContentSize({ inputW, 28.0f });
        nameBg->setColor({ 0, 0, 0 });
        nameBg->setOpacity(100);
        nameBg->setPosition({ rightX, fieldY - 22.0f });
        m_mainLayer->addChild(nameBg);

        m_nameInput = TextInput::create(inputW - 10.0f, "Nombre de la comunidad...", "chatFont.fnt");
        m_nameInput->setPosition({ rightX, fieldY - 22.0f });
        m_nameInput->getBGSprite()->setVisible(false);
        m_nameInput->setMaxCharCount(24);
        m_mainLayer->addChild(m_nameInput);

        // --- Descripcion ---
        float descY = fieldY - 60.0f;

        auto descLabel = CCLabelBMFont::create("Descripcion", "goldFont.fnt");
        descLabel->setScale(0.4f);
        descLabel->setPosition({ rightX, descY });
        m_mainLayer->addChild(descLabel);

        auto descBg = CCScale9Sprite::create("square02_small.png", { 0, 0, 40, 40 });
        descBg->setContentSize({ inputW, 45.0f });
        descBg->setColor({ 0, 0, 0 });
        descBg->setOpacity(100);
        descBg->setPosition({ rightX, descY - 30.0f });
        m_mainLayer->addChild(descBg);

        m_descInput = TextInput::create(inputW - 10.0f, "Describe tu comunidad...", "chatFont.fnt");
        m_descInput->setPosition({ rightX, descY - 30.0f });
        m_descInput->getBGSprite()->setVisible(false);
        m_descInput->setMaxCharCount(80);
        m_mainLayer->addChild(m_descInput);

        // --- Toggle de Visibilidad ---
        float toggleY = descY - 60.0f;

        auto visTitleLabel = CCLabelBMFont::create("Visibilidad", "goldFont.fnt");
        visTitleLabel->setScale(0.4f);
        visTitleLabel->setPosition({ rightX, toggleY });
        m_mainLayer->addChild(visTitleLabel);

        m_visibilityStatusLabel = CCLabelBMFont::create("Publico", "chatFont.fnt");
        m_visibilityStatusLabel->setScale(0.5f);
        m_visibilityStatusLabel->setAnchorPoint({ 1.0f, 0.5f });
        m_visibilityStatusLabel->setPosition({ rightX - 8.0f, toggleY - 22.0f });
        m_mainLayer->addChild(m_visibilityStatusLabel);

        auto checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        m_visibilityToggle = CCMenuItemToggler::create(
            checkOff, checkOn, this, menu_selector(CommunityPopup::onToggleVisibilityMark)
        );
        m_visibilityToggle->toggle(m_isPublic);
        m_visibilityToggle->setScale(0.6f);
        m_visibilityToggle->setPosition({ rightX + 15.0f, toggleY - 22.0f });
        menu->addChild(m_visibilityToggle);

        // ============================================
        //  STATUS + BOTON CREAR
        // ============================================
        m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_statusLabel->setScale(0.4f);
        m_statusLabel->setPosition({ centerX, 50.0f });
        m_mainLayer->addChild(m_statusLabel);

        auto createBtnSpr = ButtonSprite::create("Crear", "bigFont.fnt", "GJ_button_01.png", 0.8f);
        createBtnSpr->setScale(0.85f);
        auto createBtn = CCMenuItemSpriteExtra::create(createBtnSpr, this, menu_selector(CommunityPopup::onCreateCommunity));
        createBtn->setPosition({ centerX, 25.0f });
        menu->addChild(createBtn);

        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(CommunityPopup::onInfo));
        infoBtn->setPosition({ bgSize.width - 15.0f, bgSize.height - 15.0f });
        menu->addChild(infoBtn);

        return true;
    }

    void buildIconGrid() {
        auto iconMenu = CCMenu::create();
        iconMenu->setPosition({ 0, 0 });

        int totalIcons = 20;
        int cols = 4;
        float cellSize = 32.0f;
        float padding = 2.0f;
        float totalCellW = cellSize + padding;
        float totalCellH = cellSize + padding;

        int rows = (totalIcons + cols - 1) / cols;
        float gridW = 140.0f;
        float gridH = 120.0f;
        float contentH = rows * totalCellH + 5.0f;
        if (contentH < gridH) contentH = gridH;

        m_iconScroll->m_contentLayer->setContentSize({ gridW, contentH });

        float startX = (gridW - (cols * totalCellW)) / 2 + totalCellW / 2;
        auto gm = GameManager::sharedState();

        for (int i = 0; i < totalIcons; i++) {
            int iconId = i + 1;
            int col = i % cols;
            int row = i / cols;

            float x = startX + col * totalCellW;
            float y = contentH - (row * totalCellH) - totalCellH / 2;

            auto cellNode = CCNode::create();
            cellNode->setContentSize({ cellSize, cellSize });

            auto cellBg = CCScale9Sprite::create("square02_small.png", { 0, 0, 40, 40 });
            cellBg->setContentSize({ cellSize, cellSize });
            cellBg->setPosition({ cellSize / 2, cellSize / 2 });
            cellBg->setTag(100);

            if (iconId == m_selectedIcon) {
                cellBg->setColor({ 80, 200, 80 });
                cellBg->setOpacity(150);
            }
            else {
                cellBg->setColor({ 0, 0, 0 });
                cellBg->setOpacity(60);
            }
            cellNode->addChild(cellBg);

            
            auto player = SimplePlayer::create(iconId);
            player->setColor(gm->colorForIdx(gm->getPlayerColor()));
            player->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));
            if (gm->getPlayerGlow()) {
                player->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
            }
            player->updateColors();  
            player->setScale(0.55f);
            player->setPosition({ cellSize / 2, cellSize / 2 });
            cellNode->addChild(player);

            auto iconBtn = CCMenuItemSpriteExtra::create(cellNode, this, menu_selector(CommunityPopup::onSelectIcon));
            iconBtn->setPosition({ x, y });
            iconBtn->setTag(iconId);
            iconMenu->addChild(iconBtn);
        }

        m_iconScroll->m_contentLayer->addChild(iconMenu);

        float minY = -(contentH - gridH);
        if (minY > 0.0f) minY = 0.0f;
        m_iconScroll->m_contentLayer->setPositionY(minY);
    }

    void onToggleVisibilityMark(CCObject* sender) {
        m_isPublic = !m_isPublic;
        if (m_visibilityStatusLabel) {
            m_visibilityStatusLabel->setString(m_isPublic ? "Publico" : "Privado");
        }
    }

    void onSelectIcon(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);

        
        auto btnWorldPos = btn->getParent()->convertToWorldSpace(btn->getPosition());

     
        auto scrollWorldPos = m_iconScroll->getParent()->convertToWorldSpace(m_iconScroll->getPosition());
        float scrollHeight = m_iconScroll->getContentSize().height;
 
        if (btnWorldPos.y < scrollWorldPos.y || btnWorldPos.y >(scrollWorldPos.y + scrollHeight)) {
            return;
        }

        int iconId = btn->getTag();
        m_selectedIcon = iconId;

      
        auto iconMenu = btn->getParent();
        for (int i = 0; i < iconMenu->getChildrenCount(); ++i) {
            if (auto otherBtn = static_cast<CCMenuItemSpriteExtra*>(iconMenu->getChildren()->objectAtIndex(i))) {
                if (auto cellNode = static_cast<CCNode*>(otherBtn->getNormalImage())) {
                    if (auto bg = static_cast<CCScale9Sprite*>(cellNode->getChildByTag(100))) {
                        bg->setColor({ 0, 0, 0 });
                        bg->setOpacity(60);
                    }
                }
            }
        }

      
        auto cellNode = static_cast<CCNode*>(btn->getNormalImage());
        if (auto bg = static_cast<CCScale9Sprite*>(cellNode->getChildByTag(100))) {
            bg->setColor({ 80, 200, 80 });
            bg->setOpacity(150);
        }

        updatePreview();
    }

    void updatePreview() {
        if (!m_previewPlayer) return;
        auto parent = m_previewPlayer->getParent();
        auto pos = m_previewPlayer->getPosition();
        int z = m_previewPlayer->getZOrder();
        m_previewPlayer->removeFromParent();

        auto gm = GameManager::sharedState();
        m_previewPlayer = SimplePlayer::create(m_selectedIcon);
        m_previewPlayer->setColor(gm->colorForIdx(gm->getPlayerColor()));
        m_previewPlayer->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));
        if (gm->getPlayerGlow()) {
            m_previewPlayer->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
        }
        m_previewPlayer->updateColors();  
        m_previewPlayer->setScale(1.0f);
        m_previewPlayer->setPosition(pos);

        m_previewPlayer->setScale(0.0f);
        m_previewPlayer->runAction(CCEaseBackOut::create(CCScaleTo::create(0.25f, 1.0f)));
        parent->addChild(m_previewPlayer, z);
    }

    void onInfo(CCObject* sender) { FLAlertLayer::create(nullptr, "Info", "<cg>Comunidad Publica</c>: Cualquiera puede unirse con el ID.\n<cr>Comunidad Privada</c>: Solo por invitacion.", "Okei", nullptr, 360)->show(); }
    void onCreateCommunity(CCObject* sender) { std::string name = m_nameInput->getString(); if (name.empty()) { if (m_statusLabel) { m_statusLabel->setString("Escribe un nombre!"); m_statusLabel->setColor({ 255, 255, 100 }); } return; } if (m_statusLabel) { m_statusLabel->setString("Creando comunidad..."); m_statusLabel->setColor({ 255, 255, 255 }); } auto am = GJAccountManager::sharedState(); std::string ownerId = std::to_string(am->m_accountID); m_communityNet->crearComunidad(ownerId, name, m_descInput->getString(), m_selectedIcon, m_selectedCol1, m_selectedCol2, m_selectedGlow, m_isPublic); }
    void onClose(CCObject* sender) override { if (m_communityNet) { m_communityNet->setOnCommunityCreated(nullptr); m_communityNet->setOnError(nullptr); m_communityNet->setLogCallback(nullptr); m_communityNet->release(); m_communityNet = nullptr; } Popup::onClose(sender); }

public:
    static CommunityPopup* create(std::function<void()> onCreated = nullptr) {
        auto ret = new CommunityPopup();
        if (ret && ret->init(onCreated)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};