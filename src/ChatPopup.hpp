#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "ChatNetwork.hpp"
#include "ContactsNetwork.hpp"
#include "AddContactPopup.hpp"
#include "ChatBubble.hpp" 
#include "StickerManager.hpp" 
#include "StickersPopup.hpp"  
#include <fstream>

using namespace geode::prelude;

class ChatPopup : public geode::Popup, public TextInputDelegate {
protected:
    TextInput* m_input;
    ScrollLayer* m_scrollLayer;
    ScrollLayer* m_contactsScroll = nullptr;

    CCNode* m_typingNode = nullptr;
    SimplePlayer* m_typingIcon = nullptr;

    std::vector<std::pair<CCNode*, float>> m_messages;

    ChatNetwork* m_network = nullptr;
    ContactsNetwork* m_contactsNetwork = nullptr;

    std::vector<ContactInfo> m_contactList;
    static inline std::vector<std::string> s_contactPriority;  
    ContactInfo m_activeContact;
    std::string m_activeChatId = "";
    std::string m_activeChatName = "";

    size_t m_lastMessageCount = 0;
    bool m_isTyping = false;
    bool m_closedByBubble = false;
    bool m_animateLastSent = false;

    float m_chatWidth = 310.0f;
    float m_chatHeight = 200.0f;
    float m_scrollHeight = 145.0f;

    std::string applySmartWrapping(const std::string& input, int maxCharsPerLine = 28) {
        std::string result;
        int currentLineLength = 0;
        int lastSpaceIndexInResult = -1;
        for (size_t i = 0; i < input.length(); ++i) {
            char c = input[i];
            result += c;
            currentLineLength++;
            if (c == ' ') lastSpaceIndexInResult = static_cast<int>(result.length()) - 1;
            else if (c == '\n') { currentLineLength = 0; lastSpaceIndexInResult = -1; }
            if (currentLineLength >= maxCharsPerLine) {
                if (lastSpaceIndexInResult != -1) {
                    result[lastSpaceIndexInResult] = '\n';
                    currentLineLength = static_cast<int>(result.length()) - 1 - lastSpaceIndexInResult;
                    lastSpaceIndexInResult = -1;
                }
                else { result += "-\n"; currentLineLength = 0; lastSpaceIndexInResult = -1; }
            }
        }
        return result;
    }

    std::string truncateName(const std::string& name, size_t maxLength = 6) {
        if (name.length() > maxLength) return name.substr(0, maxLength) + "...";
        return name;
    }

    void enviarEstadoEscribiendo(bool state) {
        if (m_activeChatId.empty()) return;
        auto am = GJAccountManager::sharedState();
        m_network->enviarEscribiendo(std::to_string(am->m_accountID), m_activeChatId, state);
    }

    void onStopTyping(float dt) {
        if (m_isTyping) {
            m_isTyping = false;
            enviarEstadoEscribiendo(false);
        }
    }

    bool init(ContactInfo* preOpenContact) {
        if (!Popup::init(420.f, 260.f, "GJ_ChatBg_001.png"_spr)) return false;
        cargarPrioridad();
        std::string Channel = "Mundo";  
        if (preOpenContact) {
            Channel = preOpenContact->username;
        }
        this->setTitle(("Chat: " + Channel).c_str(), "bigFont.fnt", 0.6f);

        ChatBubble::sharedBubble()->hide();

        ChatBubble::sharedBubble()->setupCallback([](ContactInfo info) {
            auto popup = info.accountId.empty() ? ChatPopup::create() : ChatPopup::create(&info);
            if (popup) popup->show();
            });

        m_network = ChatNetwork::create();
        m_network->retain();

        m_network->setOnMessagesLoaded([this](const std::vector<ChatMessage>& mensajes) {
            if (mensajes.size() == m_lastMessageCount) return;
            m_lastMessageCount = mensajes.size();
            auto am = GJAccountManager::sharedState();
            std::string myId = std::to_string(am->m_accountID);
            this->limpiarChat(false);
            for (size_t idx = 0; idx < mensajes.size(); ++idx) {
                const auto& msg = mensajes[idx];
                bool isMe = (msg.senderId == myId);
                bool animateThis = m_animateLastSent && isMe && (idx == mensajes.size() - 1);
                this->addMessage(msg.texto, isMe, animateThis);
            }
            m_animateLastSent = false;
            });

        m_network->setOnTypingStatus([this](bool isTyping) {
            if (m_typingNode && !m_activeChatId.empty()) {
                m_typingNode->setVisible(isTyping);
            }
            });

        m_contactsNetwork = ContactsNetwork::create();
        m_contactsNetwork->retain();
        m_contactsNetwork->setOnContactsLoaded([this](const std::vector<ContactInfo>& contactos) {
            m_contactList = contactos;
            aplicarOrdenPrioridad();
            this->mostrarContactos(m_contactList);
            });

        auto bgSize = m_mainLayer->getContentSize();

        float contactsWidth = 40.0f;
        auto contactsBG = CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
        contactsBG->setColor({ 0, 0, 0 });
        contactsBG->setOpacity(100);
        contactsBG->setContentSize({ contactsWidth, m_chatHeight });
         
        contactsBG->setPosition({ bgSize.width / 2 - 168.0f, bgSize.height / 2 - 5.0f });

         
        contactsBG->setInsetLeft(10);
        contactsBG->setInsetRight(10);
        contactsBG->setInsetTop(10);
        contactsBG->setInsetBottom(10);

        m_mainLayer->addChild(contactsBG);

     
        auto fixedMenu = CCMenu::create();
        fixedMenu->setPosition({ 0, 0 });
        m_mainLayer->addChild(fixedMenu, 2);

      
        float topMenuY = bgSize.height - 20.0f;

      
        auto bubbleSprite = CCSprite::createWithSpriteFrameName("BubbleChatSpr.png"_spr);
        bubbleSprite->setScale(0.55f);
        auto bubbleBtn = CCMenuItemSpriteExtra::create(bubbleSprite, this, menu_selector(ChatPopup::onToggleBubble));
        bubbleBtn->setPosition({ bgSize.width - 30.0f, topMenuY });
        fixedMenu->addChild(bubbleBtn);

      
        auto addContactSprite = CCSprite::createWithSpriteFrameName("AddContanctSpr.png"_spr);
        addContactSprite->setScale(0.55f);
        auto addContactBtn = CCMenuItemSpriteExtra::create(addContactSprite, this, menu_selector(ChatPopup::onOpenAddContact));
        addContactBtn->setPosition({ bgSize.width - 65.0f, topMenuY });
        fixedMenu->addChild(addContactBtn);

         
        auto InfoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        auto InfoBtn = CCMenuItemSpriteExtra::create(InfoSpr, this, menu_selector(ChatPopup::onInfo));
        InfoBtn->setPosition({ bgSize.width - 15.0f, 15.0f });
        InfoBtn->setZOrder(10);
        fixedMenu->addChild(InfoBtn);

       
        float contactsScrollH = m_chatHeight - 20.0f;
        m_contactsScroll = ScrollLayer::create({ contactsWidth, contactsScrollH });
        m_contactsScroll->setPosition({
            contactsBG->getPositionX() - contactsWidth / 2,
            contactsBG->getPositionY() - contactsScrollH / 2
            });
        m_mainLayer->addChild(m_contactsScroll, 1);

      
        auto contactsScrollbar = Scrollbar::create(m_contactsScroll);
        contactsScrollbar->setPosition({
            contactsBG->getPositionX() - (contactsWidth / 2) - 10.0f,
            contactsBG->getPositionY()
            });
        m_mainLayer->addChild(contactsScrollbar);

        auto chatBG = CCScale9Sprite::create("square02b_001.png");
        chatBG->setColor({ 0, 0, 0 });
        chatBG->setOpacity(100);
        chatBG->setContentSize({ m_chatWidth, m_chatHeight });
        chatBG->setPosition({ bgSize.width / 2 + 15.0f, bgSize.height / 2 - 5.0f });
        m_mainLayer->addChild(chatBG);

        m_scrollLayer = ScrollLayer::create({ m_chatWidth, m_scrollHeight });
        m_scrollLayer->setPosition({
            chatBG->getPositionX() - m_chatWidth / 2,
            chatBG->getPositionY() - m_chatHeight / 2 + 45.0f
            });
        m_mainLayer->addChild(m_scrollLayer);

        auto scrollbar = Scrollbar::create(m_scrollLayer);
        scrollbar->setPosition({ chatBG->getPositionX() + m_chatWidth / 2 + 12.0f, chatBG->getPositionY() + 20.0f });
        m_mainLayer->addChild(scrollbar);

        m_typingNode = CCNode::create();
        m_typingNode->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 20.0f, chatBG->getPositionY() - m_chatHeight / 2 + 55.0f });
        m_typingNode->setVisible(false);
        m_mainLayer->addChild(m_typingNode);

        for (int i = 0; i < 3; i++) {
            auto dot = CCLabelBMFont::create(".", "bigFont.fnt");
            dot->setScale(0.6f);
            dot->setPosition({ 20.0f + (i * 10.0f), -4.0f });
            m_typingNode->addChild(dot);

            auto delayBefore = CCDelayTime::create(i * 0.15f);
           
            auto moveUp = CCEaseExponentialIn::create(CCMoveBy::create(0.3f, CCPoint(0.0f, 5.0f)));
            auto moveDown = CCEaseBackOut::create(CCMoveBy::create(0.3f, CCPoint(0.0f, -5.0f)));
           
            auto delayAfter = CCDelayTime::create(1.0f - (i * 0.15f) - 0.3f);
            auto seq = CCSequence::create(delayBefore, moveUp, moveDown, delayAfter, nullptr);
            dot->runAction(CCRepeatForever::create(seq));
        }

        auto inputMenu = CCMenu::create();
        inputMenu->setPosition({ 0, 0 });
        inputMenu->setZOrder(1);  
        m_mainLayer->addChild(inputMenu);


        auto stickerBtnSprite = CCSprite::createWithSpriteFrameName("EmojiSpr.png"_spr);
        stickerBtnSprite->setScale(0.5f);
        auto stickerBtn = CCMenuItemSpriteExtra::create(stickerBtnSprite, this, menu_selector(ChatPopup::onAddSticker));
        stickerBtn->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 224.0f, chatBG->getPositionY() - m_chatHeight / 2 + 22.0f });
        inputMenu->addChild(stickerBtn);

     
        auto ImputBg = CCScale9Sprite::create("square02_small.png", CCRectMake(0.0f, 0.0f, 40.0f, 40.0f));
        ImputBg->setContentSize({ 235.0f, 30.0f }); ///235, 30
        ImputBg->setOpacity(90);
        ImputBg->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 126.0f, chatBG->getPositionY() - m_chatHeight / 2 + 22.0f });

        m_mainLayer->addChild(ImputBg);

        float anchoInput = m_chatWidth - 120.0f;
        m_input = TextInput::create(anchoInput, "Escribe un mensaje...", "chatFont.fnt");
    
        m_input->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 110.0f, chatBG->getPositionY() - m_chatHeight / 2 + 22.0f });
        m_input->getBGSprite()->setVisible(false);
        m_input->setTextAlign(geode::TextInputAlign::Left);
        m_input->getInputNode()->setDelegate(this);
        m_input->getInputNode()->setAllowedChars(
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789 !@#$%^&*()-_=+[]{}|;:',.<>?/`~\"\\Ññ"
        );
        m_mainLayer->addChild(m_input);

        auto sendBtnSprite = CCSprite::createWithSpriteFrameName("SendSpr.png"_spr);
        sendBtnSprite->setScale(0.5f);
       
        auto sendBtn = CCMenuItemSpriteExtra::create(sendBtnSprite, this, menu_selector(ChatPopup::onSend));
      
        sendBtn->setPosition({ chatBG->getPositionX() + (m_chatWidth / 2) - 48.0f, m_input->getPositionY() });
        inputMenu->addChild(sendBtn);

         
        auto SharedSpr = CCSprite::createWithSpriteFrameName("Shared.png"_spr);
        SharedSpr->setScale(0.5f);
        auto SharedBtn = CCMenuItemSpriteExtra::create(SharedSpr, this, nullptr);
        SharedBtn->setPosition({ chatBG->getPositionX() + (m_chatWidth / 2) - 20.0f, m_input->getPositionY() });
        inputMenu->addChild(SharedBtn);

        this->schedule(schedule_selector(ChatPopup::onPollServer), 2.0f);
        m_network->registrarJugador();
        m_contactsNetwork->cargarContactos();

        if (preOpenContact) {
            m_activeChatId = preOpenContact->accountId;
            m_activeChatName = preOpenContact->username;
            m_activeContact = *preOpenContact;

            auto gm = GameManager::sharedState();
            m_typingIcon = SimplePlayer::create(m_activeContact.icon);
            m_typingIcon->setColor(gm->colorForIdx(m_activeContact.col1));
            m_typingIcon->setSecondColor(gm->colorForIdx(m_activeContact.col2));
            if (m_activeContact.glow) m_typingIcon->setGlowOutline(gm->colorForIdx(m_activeContact.glow));
            m_typingIcon->setScale(0.6f);
            m_typingIcon->setPosition({ 2.0f, -7.0f });
            m_typingNode->addChild(m_typingIcon);

            auto am = GJAccountManager::sharedState();
            m_network->cargarMensajes(std::to_string(am->m_accountID), m_activeChatId);
        }

        return true;
    }

    void onInfo(CCObject* sender) {
        std::string Info = "<cr>(1) No ser toxico</c>: No decir groserias o andar insultando a la gente\n<co>Cero Spam</c>: Nada de decir promos o invitaciones a otros servers etc...\n<cg>Hacer uso del sentido comun</c>: Hay muchas reglas que todos ya deben conocer, si aqui no puse alguna tu debes tener sentido comun y decir esto no estaria permitido";
        FLAlertLayer::create(nullptr, "Reglas", Info.c_str(), "Okei", nullptr, 360)->show();
    }

    void onAddSticker(CCObject* sender) {
        auto popup = StickersPopup::create([this](std::string command) {

            m_input->setString(command);
            });
        popup->show();
    }

    void onToggleBubble(CCObject* sender) {
        if (!m_activeChatId.empty()) {
            ChatBubble::sharedBubble()->updateInfo(m_activeContact);
        }
        m_closedByBubble = true;
        this->onClose(sender);
    }

    void onPollServer(float dt) {
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);
        if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
        if (!m_activeChatId.empty()) {
            m_network->checkEscribiendo(myId, m_activeChatId);
            m_network->cargarMensajes(myId, m_activeChatId);
        }
    }

    virtual void textChanged(CCTextInputNode* input) override {
        if (m_activeChatId.empty()) return;
        std::string text = input->getString();
        if (text.empty()) {
            if (m_isTyping) {
                m_isTyping = false;
                enviarEstadoEscribiendo(false);
                this->unschedule(schedule_selector(ChatPopup::onStopTyping));
            }
        }
        else {
            if (!m_isTyping) {
                m_isTyping = true;
                enviarEstadoEscribiendo(true);
            }
            this->unschedule(schedule_selector(ChatPopup::onStopTyping));
            this->schedule(schedule_selector(ChatPopup::onStopTyping), 3.0f);
        }
    }

    void mostrarContactos(const std::vector<ContactInfo>& contactos) {
        m_contactsScroll->m_contentLayer->removeAllChildren();
        float contactsWidth = 40.0f; float spacing = 38.0f; float scrollH = m_chatHeight - 20.0f;

        // +1 para el contacto de anuncios del mod
        size_t totalItems = contactos.size() + 1;
        float totalHeight = spacing * totalItems;
        if (totalHeight < scrollH) totalHeight = scrollH;

        m_contactsScroll->m_contentLayer->setContentSize({ contactsWidth, totalHeight });
        auto menu = CCMenu::create(); menu->setPosition({ 0, 0 });
        m_contactsScroll->m_contentLayer->addChild(menu);

        auto gm = GameManager::sharedState();
        float currentY = totalHeight;

         
        {
            currentY -= spacing;

            auto contactHitbox = CCSprite::create();
            contactHitbox->setContentSize({ 35.0f, 35.0f });

            auto modIcon = CCSprite::createWithSpriteFrameName("IconSpr.png"_spr);
            modIcon->setScale(0.15f);
            modIcon->setPosition({ 17.5f, 17.5f });
            contactHitbox->addChild(modIcon);

            auto contactBtn = CCMenuItemSpriteExtra::create(contactHitbox, this, nullptr);
            contactBtn->setPosition({ contactsWidth / 2, currentY + spacing / 2 });
            contactBtn->setTag(99);  
            menu->addChild(contactBtn);

            auto nameLabel = CCLabelBMFont::create("Anunci...", "chatFont.fnt");
            nameLabel->setScale(0.23f);
            nameLabel->setPosition({ contactsWidth / 2, currentY + spacing / 2 - 16.0f });
            nameLabel->setColor({ 255, 215, 0 });  
            m_contactsScroll->m_contentLayer->addChild(nameLabel);
        }

     
        for (size_t i = 0; i < contactos.size(); i++) {
            currentY -= spacing;

            auto contactHitbox = CCSprite::create();
            contactHitbox->setContentSize({ 35.0f, 35.0f });

            auto contactSprite = SimplePlayer::create(contactos[i].icon);
            contactSprite->setColor(gm->colorForIdx(contactos[i].col1));
            contactSprite->setSecondColor(gm->colorForIdx(contactos[i].col2));
            if (contactos[i].glow) contactSprite->setGlowOutline(gm->colorForIdx(contactos[i].glow));
            contactSprite->setScale(0.55f);
            contactSprite->setPosition({ 17.5f, 17.5f });
            contactHitbox->addChild(contactSprite);

            if (contactos[i].unreadCount > 0) {
                std::string unreadText = std::to_string(contactos[i].unreadCount);
                if (contactos[i].unreadCount > 9) unreadText = "9+";

                auto badgeLabel = CCLabelBMFont::create(unreadText.c_str(), "bigFont.fnt");
                badgeLabel->setScale(0.40f);
                badgeLabel->setColor({ 255, 60, 60 });
                badgeLabel->setPosition({ 26.0f, 26.0f });
                contactHitbox->addChild(badgeLabel, 10);
            }

            auto contactBtn = CCMenuItemSpriteExtra::create(contactHitbox, this, menu_selector(ChatPopup::onSelectContact));
            contactBtn->setPosition({ contactsWidth / 2, currentY + spacing / 2 });
            contactBtn->setTag(static_cast<int>(i + 100));

            auto dataString = CCString::createWithFormat("%s", contactos[i].accountId.c_str());
            contactBtn->setUserObject(dataString);
            menu->addChild(contactBtn);

            std::string displayName = truncateName(contactos[i].username, 6);
            auto nameLabel = CCLabelBMFont::create(displayName.c_str(), "chatFont.fnt");
            nameLabel->setScale(0.23f);
            nameLabel->setPosition({ contactsWidth / 2, currentY + spacing / 2 - 16.0f });
            nameLabel->setColor({ 200, 200, 200 });
            m_contactsScroll->m_contentLayer->addChild(nameLabel);
        }

       
        float minY = -(totalHeight - scrollH);
        if (minY > 0.0f) minY = 0.0f;
        m_contactsScroll->m_contentLayer->setPositionY(minY);

       
        if (this->isRunning()) {
            geode::cocos::handleTouchPriority(this);
        }
    }

    void onSelectContact(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto dataStr = static_cast<CCString*>(btn->getUserObject());
        if (!dataStr) return;

        if (m_input) m_input->setString("");

        if (m_isTyping) {
            enviarEstadoEscribiendo(false);
            m_isTyping = false;
            this->unschedule(schedule_selector(ChatPopup::onStopTyping));
        }

        m_activeChatId = dataStr->getCString();
        m_typingNode->setVisible(false);

        for (const auto& c : m_contactList) {
            if (c.accountId == m_activeChatId) {
                m_activeContact = c;
                m_activeChatName = c.username;
                break;
            }
        }

        if (m_typingIcon) {
            m_typingIcon->removeFromParent();
            m_typingIcon = nullptr;
        }
        auto gm = GameManager::sharedState();
        m_typingIcon = SimplePlayer::create(m_activeContact.icon);
        m_typingIcon->setColor(gm->colorForIdx(m_activeContact.col1));
        m_typingIcon->setSecondColor(gm->colorForIdx(m_activeContact.col2));
        if (m_activeContact.glow) m_typingIcon->setGlowOutline(gm->colorForIdx(m_activeContact.glow));
        m_typingIcon->setScale(0.6f);
        m_typingIcon->setPosition({ 2.0f, -7.0f });
        m_typingNode->addChild(m_typingIcon);

        limpiarChat(true);

        auto am = GJAccountManager::sharedState();
        m_network->cargarMensajes(std::to_string(am->m_accountID), m_activeChatId);
        if (m_contactsNetwork) m_contactsNetwork->cargarContactos();

        
        this->setTitle(("Chat: " + m_activeChatName).c_str(), "bigFont.fnt", 0.6f);
    }

    void limpiarChat(bool resetCounter) {
        if (resetCounter) m_lastMessageCount = 0;
        m_messages.clear();
        m_scrollLayer->m_contentLayer->removeAllChildren();
        m_scrollLayer->m_contentLayer->setContentSize({ m_chatWidth, m_scrollHeight });
        m_scrollLayer->m_contentLayer->setPositionY(0.0f);
    }

    void onOpenAddContact(CCObject* sender) {
        auto popup = AddContactPopup::create([this]() { m_contactsNetwork->cargarContactos(); });
        popup->show();
    }

    void addMessage(const std::string& rawText, bool isMe, bool animate = false) {
        auto msgNode = CCNode::create();
        auto gm = GameManager::sharedState();

        float margenTextoTotal = 60.0f;
        float anchoMaxTexto = m_chatWidth - margenTextoTotal - 10.0f;

        CCNode* contentNode = nullptr;
        float msgHeightActual = 45.0f;

        auto stickerSpr = StickerManager::createSticker(rawText);

        if (stickerSpr) {
            contentNode = stickerSpr;
            msgHeightActual = stickerSpr->getScaledContentSize().height + 20.0f;
            if (msgHeightActual < 45.0f) msgHeightActual = 45.0f;

            if (isMe) {
                contentNode->setAnchorPoint({ 1.0f, 0.5f });
                contentNode->setPosition({ m_chatWidth - 45.0f, msgHeightActual / 2 - 5.0f });
            }
            else {
                contentNode->setAnchorPoint({ 0.0f, 0.5f });
                contentNode->setPosition({ 45.0f, msgHeightActual / 2 - 5.0f });
            }
        }
        else {
            std::string processedText = applySmartWrapping(rawText, 28);
            auto label = CCLabelBMFont::create(processedText.c_str(), "chatFont.fnt", anchoMaxTexto, kCCTextAlignmentLeft);
            label->setScale(0.55f);

            float alturaRealTexto = label->getContentSize().height * 0.55f;

           
            auto measureLabel = CCLabelBMFont::create(processedText.c_str(), "chatFont.fnt");
            float anchoRealTexto = measureLabel->getContentSize().width * 0.55f;
            if (anchoRealTexto > anchoMaxTexto) anchoRealTexto = anchoMaxTexto;

            msgHeightActual = alturaRealTexto + 20.0f;
            if (msgHeightActual < 45.0f) msgHeightActual = 45.0f;

            
            float bubblePadH = 10.0f;
            float bubblePadV = 8.0f;
            float bubbleW = anchoRealTexto + bubblePadH * 2;
            float bubbleH = alturaRealTexto + bubblePadV * 2;
            if (bubbleW < 30.0f) bubbleW = 30.0f;
            if (bubbleH < 26.0f) bubbleH = 26.0f;

            auto bubble = CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
            bubble->setContentSize({ bubbleW, bubbleH });
            bubble->setInsetLeft(10);
            bubble->setInsetRight(10);
            bubble->setInsetTop(10);
            bubble->setInsetBottom(10);

         
            auto bubbleContainer = CCNode::create();
            bubbleContainer->setContentSize({ bubbleW, bubbleH });

            bubble->setColor({ 0, 0, 0 });
            bubble->setOpacity(120);
            if (isMe) label->setColor({ 100, 255, 100 });
            else label->setColor({ 255, 255, 255 });
            label->setAnchorPoint({ 0.0f, 0.5f });
            label->setPosition({ bubblePadH, bubbleH / 2 });

            bubbleContainer->addChild(bubble);
            bubbleContainer->addChild(label, 1);

            bubble->setAnchorPoint({ 0.0f, 0.0f });
            bubble->setPosition({ 0.0f, 0.0f });

            if (isMe) {
                bubbleContainer->setAnchorPoint({ 1.0f, 0.5f });
                bubbleContainer->setPosition({ m_chatWidth - 45.0f, msgHeightActual / 2 - 5.0f });
            }
            else {
                bubbleContainer->setAnchorPoint({ 0.0f, 0.5f });
                bubbleContainer->setPosition({ 45.0f, msgHeightActual / 2 - 5.0f });
            }

            contentNode = bubbleContainer;
        }

        if (isMe) {
            auto player = SimplePlayer::create(gm->getPlayerFrame());
            player->setColor(gm->colorForIdx(gm->getPlayerColor()));
            player->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));
            if (gm->getPlayerGlow()) player->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
            player->setScale(0.65f);
            player->setPosition({ m_chatWidth - 20.0f, msgHeightActual / 2 });
            msgNode->addChild(player);
        }
        else {
            if (m_activeChatId.empty()) {
                auto botIcon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
                botIcon->setScale(0.75f);
                botIcon->setPosition({ 20.0f, msgHeightActual / 2 });
                msgNode->addChild(botIcon);
            }
            else {
                auto player = SimplePlayer::create(m_activeContact.icon);
                player->setColor(gm->colorForIdx(m_activeContact.col1));
                player->setSecondColor(gm->colorForIdx(m_activeContact.col2));
                if (m_activeContact.glow) player->setGlowOutline(gm->colorForIdx(m_activeContact.glow));
                player->setScale(0.65f);
                player->setPosition({ 20.0f, msgHeightActual / 2 });
                msgNode->addChild(player);

                auto nameLabel = CCLabelBMFont::create(m_activeChatName.c_str(), "chatFont.fnt");
                nameLabel->setScale(0.35f);
                nameLabel->setColor({ 200, 200, 200 });
                nameLabel->setAnchorPoint({ 0.0f, 0.0f });
                nameLabel->setPosition({ 45.0f, msgHeightActual - 12.0f });
                msgNode->addChild(nameLabel);
            }
        }

        msgNode->addChild(contentNode);
        msgNode->setContentSize({ m_chatWidth, msgHeightActual });

        m_messages.push_back({ msgNode, msgHeightActual });
        m_scrollLayer->m_contentLayer->addChild(msgNode);

        float totalHeightMessages = 0.0f;
        for (const auto& pair : m_messages) totalHeightMessages += pair.second;

        float totalHeight = totalHeightMessages + 15.0f;
        if (totalHeight < m_scrollHeight) totalHeight = m_scrollHeight;

        m_scrollLayer->m_contentLayer->setContentSize({ m_chatWidth, totalHeight });
        float currentY = totalHeight;
        for (const auto& pair : m_messages) {
            currentY -= pair.second;
            pair.first->setPosition({ 0.0f, currentY });
        }
        m_scrollLayer->m_contentLayer->stopAllActions();
        m_scrollLayer->m_contentLayer->setPositionY(0.0f);

        
        if (animate) {
            float slideOffset = 40.0f;
            float originalY = msgNode->getPositionY();
            msgNode->setPositionY(originalY - slideOffset);
            msgNode->setScale(0.85f);

            auto slideUp = CCEaseBackOut::create(CCMoveTo::create(0.3f, ccp(0.0f, originalY)));
            auto scaleUp = CCEaseExponentialOut::create(CCScaleTo::create(0.25f, 1.0f));

            msgNode->runAction(slideUp);
            msgNode->runAction(scaleUp);
        }
    }

    static void guardarPrioridad() {
        auto path = Mod::get()->getSaveDir() / "contact_priority.json";
        matjson::Value arr = matjson::Value::array();
        for (const auto& id : s_contactPriority) {
            arr.push(id);
        }
        std::ofstream file(path);
        if (file.is_open()) {
            file << arr.dump();
            file.close();
        }
    }

    static void cargarPrioridad() {
        if (!s_contactPriority.empty()) return;  
        auto path = Mod::get()->getSaveDir() / "contact_priority.json";
        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        if (content.empty()) return;

        auto result = matjson::parse(content);
        if (result.isOk() && result.unwrap().isArray()) {
            s_contactPriority.clear();
            for (const auto& val : result.unwrap()) {
                if (val.isString()) {
                    s_contactPriority.push_back(val.asString().unwrap());
                }
            }
        }
    }

    void aplicarOrdenPrioridad() {
        if (s_contactPriority.empty()) return;
        
        std::vector<ContactInfo> ordered;
        std::vector<ContactInfo> rest = m_contactList;

        for (const auto& prioId : s_contactPriority) {
            for (auto it = rest.begin(); it != rest.end(); ++it) {
                if (it->accountId == prioId) {
                    ordered.push_back(*it);
                    rest.erase(it);
                    break;
                }
            }
        }
     
        for (const auto& c : rest) ordered.push_back(c);
        m_contactList = ordered;
    }

    void moverContactoAlInicio(const std::string& accountId) {
         
        s_contactPriority.erase(
            std::remove(s_contactPriority.begin(), s_contactPriority.end(), accountId),
            s_contactPriority.end()
        );
        s_contactPriority.insert(s_contactPriority.begin(), accountId);
        guardarPrioridad();
      
        for (size_t i = 0; i < m_contactList.size(); i++) {
            if (m_contactList[i].accountId == accountId) {
                if (i == 0) return;
                ContactInfo contact = m_contactList[i];
                m_contactList.erase(m_contactList.begin() + i);
                m_contactList.insert(m_contactList.begin(), contact);
                mostrarContactos(m_contactList);
                return;
            }
        }
    }

    void onSend(CCObject* sender) {
        std::string text = m_input->getString();
        if (text.empty() || m_activeChatId.empty()) return;

        m_input->setString("");

        if (m_isTyping) {
            m_isTyping = false;
            enviarEstadoEscribiendo(false);
            this->unschedule(schedule_selector(ChatPopup::onStopTyping));
        }

        auto am = GJAccountManager::sharedState();
        m_animateLastSent = true;
        m_network->enviarMensaje(std::to_string(am->m_accountID), m_activeChatId, text);
        m_network->cargarMensajes(std::to_string(am->m_accountID), m_activeChatId);

      
        moverContactoAlInicio(m_activeChatId);
    }

    virtual void textInputReturn(CCTextInputNode* textInput) override { this->onSend(nullptr); }

    void onClose(CCObject* sender) override {
        if (m_isTyping) enviarEstadoEscribiendo(false);

        if (!m_closedByBubble && ChatBubble::s_instance) {
            ChatBubble::s_instance->hide();
        }

        this->unschedule(schedule_selector(ChatPopup::onPollServer));
        this->unschedule(schedule_selector(ChatPopup::onStopTyping));

        if (m_network) {
            m_network->setLogCallback(nullptr);
            m_network->setOnMessagesLoaded(nullptr);
            m_network->setOnTypingStatus(nullptr);
            m_network->release(); m_network = nullptr;
        }
        if (m_contactsNetwork) {
            m_contactsNetwork->setOnContactsLoaded(nullptr);
            m_contactsNetwork->release(); m_contactsNetwork = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static ChatPopup* create(ContactInfo* preOpenContact = nullptr) {
        auto ret = new ChatPopup();
        if (ret && ret->init(preOpenContact)) {
            ret->autorelease(); return ret;
        }
        CC_SAFE_DELETE(ret); return nullptr;
    }
};