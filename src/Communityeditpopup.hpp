#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "CommunityNetwork.hpp"

using namespace geode::prelude;

class CommunityEditPopup : public geode::Popup {
protected:
    TextInput* m_nameInput = nullptr;
    TextInput* m_descInput = nullptr;
    ScrollLayer* m_iconScroll = nullptr;
    CommunityNetwork* m_net = nullptr;

    std::string m_communityId;
    std::string m_ownerId;
    std::string m_origName;
    std::string m_origDesc;

    int m_selectedIcon = 1;
    int m_selectedCol1 = 0;
    int m_selectedCol2 = 3;
    int m_selectedGlow = 0;
    bool m_isPublic = true;

    SimplePlayer* m_previewPlayer = nullptr;
    CCLabelBMFont* m_visibilityStatusLabel = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    std::function<void()> m_onEdited = nullptr;

    bool init(std::string commId, std::string ownerId, CommunityInfo info, std::function<void()> onEdited) {
        if (!Popup::init(400.f, 280.f, "GJ_square01.png")) return false;
        m_communityId = commId;
        m_ownerId = ownerId;
        m_onEdited = onEdited;
        m_origName = info.name;
        m_origDesc = info.description;
        m_selectedIcon = info.icon;
        m_selectedCol1 = info.col1;
        m_selectedCol2 = info.col2;
        m_selectedGlow = info.glow;
        m_isPublic = info.isPublic;

        this->setTitle("Editar Comunidad", "bigFont.fnt", 0.6f);

        auto bgSize = m_mainLayer->getContentSize();
        float centerX = bgSize.width / 2;

        m_net = CommunityNetwork::create();
        m_net->retain();
        m_net->setOnCommunityEdited([this]() {
            FLAlertLayer::create(nullptr, "Exito", "Comunidad editada!", "OK", nullptr, 300)->show();
            if (m_onEdited) m_onEdited();
            this->onClose(nullptr);
            });
        m_net->setOnError([this](const std::string& err) {
            if (m_statusLabel) { m_statusLabel->setString(err.c_str()); m_statusLabel->setColor({ 255, 80, 80 }); }
            });

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        m_mainLayer->addChild(menu, 2);

        auto gm = GameManager::sharedState();

       
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
        m_previewPlayer->setColor(gm->colorForIdx(m_selectedCol1));
        m_previewPlayer->setSecondColor(gm->colorForIdx(m_selectedCol2));
        if (m_selectedGlow) {
            m_previewPlayer->setGlowOutline(gm->colorForIdx(m_selectedGlow));
        }
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

        m_nameInput = TextInput::create(inputW - 10.0f, "Nombre...", "chatFont.fnt");
        m_nameInput->setPosition({ rightX, fieldY - 22.0f });
        m_nameInput->getBGSprite()->setVisible(false);
        m_nameInput->setMaxCharCount(24);
        m_nameInput->setString(m_origName);
        m_mainLayer->addChild(m_nameInput);

      
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

        m_descInput = TextInput::create(inputW - 10.0f, "Descripcion...", "chatFont.fnt");
        m_descInput->setPosition({ rightX, descY - 30.0f });
        m_descInput->getBGSprite()->setVisible(false);
        m_descInput->setMaxCharCount(80);
        m_descInput->setString(m_origDesc);
        m_mainLayer->addChild(m_descInput);

       
        float toggleY = descY - 60.0f;

        auto visTitleLabel = CCLabelBMFont::create("Visibilidad", "goldFont.fnt");
        visTitleLabel->setScale(0.4f);
        visTitleLabel->setPosition({ rightX, toggleY });
        m_mainLayer->addChild(visTitleLabel);

        m_visibilityStatusLabel = CCLabelBMFont::create(m_isPublic ? "Publico" : "Privado", "chatFont.fnt");
        m_visibilityStatusLabel->setScale(0.5f);
        m_visibilityStatusLabel->setAnchorPoint({ 1.0f, 0.5f });
        m_visibilityStatusLabel->setPosition({ rightX + 5.0f, toggleY - 22.0f });
        m_mainLayer->addChild(m_visibilityStatusLabel);

        auto checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        auto checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto visToggle = CCMenuItemToggler::create(checkOff, checkOn, this, menu_selector(CommunityEditPopup::onToggleVisibility));
        visToggle->toggle(m_isPublic);
        visToggle->setScale(0.6f);
        visToggle->setPosition({ rightX + 15.0f, toggleY - 22.0f });
        menu->addChild(visToggle);

     
        m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_statusLabel->setScale(0.4f);
        m_statusLabel->setPosition({ centerX, 50.0f });
        m_mainLayer->addChild(m_statusLabel);

        auto saveBtnSpr = ButtonSprite::create("Guardar", "bigFont.fnt", "GJ_button_01.png", 0.8f);
        saveBtnSpr->setScale(0.85f);
        auto saveBtn = CCMenuItemSpriteExtra::create(saveBtnSpr, this, menu_selector(CommunityEditPopup::onSave));
        saveBtn->setPosition({ centerX, 25.0f });
        menu->addChild(saveBtn);

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
            player->setColor(gm->colorForIdx(m_selectedCol1));
            player->setSecondColor(gm->colorForIdx(m_selectedCol2));
            if (m_selectedGlow) {
                player->setGlowOutline(gm->colorForIdx(m_selectedGlow));
            }
            player->setScale(0.55f);
            player->setPosition({ cellSize / 2, cellSize / 2 });
            cellNode->addChild(player);

            auto iconBtn = CCMenuItemSpriteExtra::create(cellNode, this, menu_selector(CommunityEditPopup::onSelectIcon));
            iconBtn->setPosition({ x, y });
            iconBtn->setTag(iconId);
            iconMenu->addChild(iconBtn);
        }

        m_iconScroll->m_contentLayer->addChild(iconMenu);
        float minY = -(contentH - gridH);
        if (minY > 0.0f) minY = 0.0f;
        m_iconScroll->m_contentLayer->setPositionY(minY);
    }

    void onSelectIcon(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto btnWorldPos = btn->getParent()->convertToWorldSpace(btn->getPosition());
        auto scrollWorldPos = m_iconScroll->getParent()->convertToWorldSpace(m_iconScroll->getPosition());
        float scrollHeight = m_iconScroll->getContentSize().height;
        if (btnWorldPos.y < scrollWorldPos.y || btnWorldPos.y >(scrollWorldPos.y + scrollHeight)) return;

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
        m_previewPlayer->setColor(gm->colorForIdx(m_selectedCol1));
        m_previewPlayer->setSecondColor(gm->colorForIdx(m_selectedCol2));
        if (m_selectedGlow) {
            m_previewPlayer->setGlowOutline(gm->colorForIdx(m_selectedGlow));
        }
        m_previewPlayer->setScale(0.0f);
        m_previewPlayer->setPosition(pos);
        m_previewPlayer->runAction(CCEaseBackOut::create(CCScaleTo::create(0.25f, 1.0f)));
        parent->addChild(m_previewPlayer, z);
    }

    void onToggleVisibility(CCObject* sender) {
        m_isPublic = !m_isPublic;
        if (m_visibilityStatusLabel) {
            m_visibilityStatusLabel->setString(m_isPublic ? "Publico" : "Privado");
        }
    }

    void onSave(CCObject* sender) {
        std::string name = m_nameInput->getString();
        if (name.empty()) {
            if (m_statusLabel) { m_statusLabel->setString("Escribe un nombre!"); m_statusLabel->setColor({ 255, 255, 100 }); }
            return;
        }
        if (m_statusLabel) { m_statusLabel->setString("Guardando..."); m_statusLabel->setColor({ 255, 255, 255 }); }
        m_net->editarComunidad(m_ownerId, m_communityId, name, m_descInput->getString(),
            m_selectedIcon, m_selectedCol1, m_selectedCol2, m_selectedGlow, m_isPublic);
    }

    void onClose(CCObject* sender) override {
        if (m_net) {
            m_net->setOnCommunityEdited(nullptr);
            m_net->setOnError(nullptr);
            m_net->release();
            m_net = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static CommunityEditPopup* create(std::string commId, std::string ownerId, CommunityInfo info, std::function<void()> onEdited = nullptr) {
        auto ret = new CommunityEditPopup();
        if (ret && ret->init(commId, ownerId, info, onEdited)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};