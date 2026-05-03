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
#include "CommunityPopup.hpp"
#include "CommunityAddMemberPopup.hpp"
#include "CommunityMembersPopup.hpp"
#include "CommunityEditPopup.hpp"
#include "CommunityBrowserPopup.hpp"
#include "CommunityJoinRequestsPopup.hpp"
#include "CommunityInvitesPopup.hpp"
#include <fstream>

using namespace geode::prelude;

class ChatPopup : public geode::Popup, public TextInputDelegate, public FLAlertLayerProtocol {
protected:
    TextInput* m_input;
    ScrollLayer* m_scrollLayer;
    ScrollLayer* m_contactsScroll = nullptr;
    CCScale9Sprite* m_contactIndicator = nullptr;
    std::vector<std::pair<CCNode*, float>> m_messages;
    ChatNetwork* m_network = nullptr;
    ContactsNetwork* m_contactsNetwork = nullptr;
    std::vector<ContactInfo> m_contactList;
    static inline std::vector<std::string> s_contactPriority;
    ContactInfo m_activeContact;
    std::string m_activeChatId = "";
    std::string m_activeChatName = "";
    size_t m_lastMessageCount = 0;
    std::string m_lastMessageText = "";
    bool m_closedByBubble = false;
    bool m_animateLastSent = false;
    float m_chatWidth = 310.0f;
    float m_chatHeight = 200.0f;
    float m_scrollHeight = 145.0f;
    CCLabelBMFont* m_chatSubtitle = nullptr;
    CCMenuItemSpriteExtra* m_addMemberBtn = nullptr;
    CCMenuItemSpriteExtra* m_membersBtn = nullptr;
    CCMenuItemSpriteExtra* m_editCommunityBtn = nullptr;
    CCMenuItemSpriteExtra* m_deleteCommunityBtn = nullptr;
    CCMenuItemSpriteExtra* m_joinRequestsBtn = nullptr;
    CCMenuItemSpriteExtra* m_leaveCommunityBtn = nullptr;
    CommunityNetwork* m_communityNet = nullptr;

    std::string applySmartWrapping(const std::string& input, int maxCharsPerLine = 28) {
        std::string result;
        int currentLineLength = 0;
        int lastSpaceIndexInResult = -1;
        for (size_t i = 0; i < input.length(); ++i) {
            char c = input[i];
            result += c;
            currentLineLength++;
            if (c == ' ') {
                lastSpaceIndexInResult = static_cast<int>(result.length()) - 1;
            }
            else if (c == '\n') {
                currentLineLength = 0;
                lastSpaceIndexInResult = -1;
            }
            if (currentLineLength >= maxCharsPerLine) {
                if (lastSpaceIndexInResult != -1) {
                    result[lastSpaceIndexInResult] = '\n';
                    currentLineLength = static_cast<int>(result.length()) - 1 - lastSpaceIndexInResult;
                    lastSpaceIndexInResult = -1;
                }
                else {
                    result += "-\n";
                    currentLineLength = 0;
                    lastSpaceIndexInResult = -1;
                }
            }
        }
        return result;
    }

    std::string truncateName(const std::string& name, size_t maxLength = 6) {
        if (name.length() > maxLength) {
            return name.substr(0, maxLength) + "...";
        }
        return name;
    }

#ifdef GEODE_IS_DESKTOP
    bool isMouseOverNode(CCNode* node, CCPoint worldPos) {
        if (!node || !node->isVisible()) {
            return false;
        }
        auto local = node->convertToNodeSpace(worldPos);
        auto size = node->getContentSize();
        return local.x >= 0 && local.x <= size.width && local.y >= 0 && local.y <= size.height;
    }

    void setupScrollMouseHandling(float dt) {
        auto dispatcher = CCDirector::sharedDirector()->getMouseDispatcher();
        if (m_scrollLayer) {
            dispatcher->removeDelegate(m_scrollLayer);
        }
        if (m_contactsScroll) {
            dispatcher->removeDelegate(m_contactsScroll);
        }
        dispatcher->addDelegate(static_cast<CCMouseDelegate*>(this));
    }

    void scrollWheel(float y, float x) override {
        auto mousePos = geode::cocos::getMousePos();
        if (m_contactsScroll && isMouseOverNode(m_contactsScroll, mousePos)) {
            m_contactsScroll->scrollWheel(y, x);
            return;
        }
        if (m_scrollLayer && isMouseOverNode(m_scrollLayer, mousePos)) {
            m_scrollLayer->scrollWheel(y, x);
            return;
        }
    }
#endif

    bool init(ContactInfo* preOpenContact) {
        if (!Popup::init(420.f, 260.f, "GJ_square02.png")) {
            return false;
        }

        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) {
            FLAlertLayer::create(nullptr, "Kristal", "You must be logged into Geometry Dash to use the chat.", "OK", nullptr, 300)->show();
            return false;
        }

        cargarPrioridad();
        std::string Channel = "World";

        if (preOpenContact) {
            Channel = preOpenContact->username;
        }

        auto bgSize = m_mainLayer->getContentSize();

        auto titleContainer = CCNode::create();
        titleContainer->setPosition({ bgSize.width / 2, bgSize.height - 15.0f });
        m_mainLayer->addChild(titleContainer);

        auto logoSpr = CCSprite::create("logo.png"_spr);
        if (!logoSpr) {
            logoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        logoSpr->setScale(0.15f);

        auto kristalLabel = CCLabelBMFont::create("Kristal", "bigFont.fnt");
        kristalLabel->setScale(0.6f);

        float logoW = logoSpr->getScaledContentSize().width;
        float textW = kristalLabel->getScaledContentSize().width;
        float spacing = 8.0f;
        float totalW = logoW + spacing + textW;

        logoSpr->setPosition({ -totalW / 2 + logoW / 2, -1.0f });
        kristalLabel->setPosition({ totalW / 2 - textW / 2, 0.0f });

        titleContainer->addChild(logoSpr);
        titleContainer->addChild(kristalLabel);

        m_chatSubtitle = CCLabelBMFont::create(("Chat: " + Channel).c_str(), "chatFont.fnt");
        m_chatSubtitle->setScale(0.45f);
        m_chatSubtitle->setColor({ 200, 200, 200 });
        m_chatSubtitle->setPosition({ bgSize.width / 2, bgSize.height - 30.0f });
        m_mainLayer->addChild(m_chatSubtitle);

        ChatBubble::sharedBubble()->hide();

        ChatBubble::sharedBubble()->setupCallback([](ContactInfo info) {
            auto popup = info.accountId.empty() ? ChatPopup::create() : ChatPopup::create(&info);
            if (popup) {
                popup->show();
            }
            });

        m_network = ChatNetwork::create();
        m_network->retain();
        m_network->setOnMessagesLoaded([this](const std::vector<ChatMessage>& mensajes) {
            if (mensajes.empty()) {
                if (m_lastMessageCount > 0) {
                    this->limpiarChat(true);
                }
                return;
            }

            std::string currentLastText = mensajes.back().texto;
            bool isSameSize = (mensajes.size() == m_lastMessageCount);
            bool isSameLastMsg = (currentLastText == m_lastMessageText);

            // Bug Fix: Only skip redrawing if the UI actually has children rendered.
            bool uiIsEmpty = m_scrollLayer->m_contentLayer->getChildrenCount() == 0;

            if (isSameSize && isSameLastMsg && !uiIsEmpty) {
                return;
            }

            bool isInitialLoad = (m_lastMessageCount == 0);
            bool isNewMessage = !isSameLastMsg || mensajes.size() > m_lastMessageCount;
            m_lastMessageCount = mensajes.size();
            m_lastMessageText = currentLastText;

            auto am = GJAccountManager::sharedState();
            std::string myId = std::to_string(am->m_accountID);

            this->limpiarChat(false);

            for (size_t idx = 0; idx < mensajes.size(); ++idx) {
                const auto& msg = mensajes[idx];
                bool isMe = (msg.senderId == myId);
                bool isLast = (idx == mensajes.size() - 1);
                bool animateThis = isLast && (m_animateLastSent || (isNewMessage && !isMe && !isInitialLoad));
                this->addMessage(msg, isMe, animateThis);
            }
            m_animateLastSent = false;
            });

        m_contactsNetwork = ContactsNetwork::create();
        m_contactsNetwork->retain();
        m_contactsNetwork->setOnContactsLoaded([this](const std::vector<ContactInfo>& contactos) {
            m_contactList = contactos;
            aplicarOrdenPrioridad();
            this->mostrarContactos(m_contactList);
            });

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

        auto InfoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        auto InfoBtn = CCMenuItemSpriteExtra::create(InfoSpr, this, menu_selector(ChatPopup::onInfo));
        InfoBtn->setPosition({ bgSize.width - 15.0f, 15.0f });
        InfoBtn->setZOrder(10);
        fixedMenu->addChild(InfoBtn);

        auto addMemberSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        m_addMemberBtn = CCMenuItemSpriteExtra::create(addMemberSpr, this, menu_selector(ChatPopup::onAddCommunityMember));
        m_addMemberBtn->setPosition({ bgSize.width - 45.0f, 15.0f });
        addMemberSpr->setScale(0.3f);
        m_addMemberBtn->setZOrder(10);
        m_addMemberBtn->setVisible(false);
        fixedMenu->addChild(m_addMemberBtn);
 
        auto leaveSpr = CCSprite::createWithSpriteFrameName("exit_group_btn.png"_spr);     
        leaveSpr->setScale(0.52f);
        m_leaveCommunityBtn = CCMenuItemSpriteExtra::create(leaveSpr, this, menu_selector(ChatPopup::onLeaveCommunity));
        m_leaveCommunityBtn->setPosition({ bgSize.width - 45.0f, 15.0f });
        m_leaveCommunityBtn->setZOrder(10);
        m_leaveCommunityBtn->setVisible(false);
        fixedMenu->addChild(m_leaveCommunityBtn);

        auto membersSpr = CCSprite::createWithSpriteFrameName("GJ_longBtn05_001.png");
        membersSpr->setScale(0.45f);
        m_membersBtn = CCMenuItemSpriteExtra::create(membersSpr, this, menu_selector(ChatPopup::onShowMembers));
        m_membersBtn->setPosition({ bgSize.width - 75.0f, 15.0f });
        m_membersBtn->setZOrder(10);
        m_membersBtn->setVisible(false);
        fixedMenu->addChild(m_membersBtn);

        auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
        editSpr->setScale(0.2f);
        m_editCommunityBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(ChatPopup::onEditCommunity));
        m_editCommunityBtn->setPosition({ bgSize.width - 105.0f, 15.0f });
        m_editCommunityBtn->setZOrder(10);
        m_editCommunityBtn->setVisible(false);
        fixedMenu->addChild(m_editCommunityBtn);

        auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        deleteSpr->setScale(0.4f);
        m_deleteCommunityBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(ChatPopup::onDeleteCommunity));
        m_deleteCommunityBtn->setPosition({ bgSize.width - 135.0f, 15.0f });
        m_deleteCommunityBtn->setZOrder(10);
        m_deleteCommunityBtn->setVisible(false);
        fixedMenu->addChild(m_deleteCommunityBtn);

        auto joinReqSpr = CCSprite::createWithSpriteFrameName("accountBtn_requests_001.png");
        joinReqSpr->setScale(0.40f);
        m_joinRequestsBtn = CCMenuItemSpriteExtra::create(joinReqSpr, this, menu_selector(ChatPopup::onShowJoinRequests));
        m_joinRequestsBtn->setPosition({ bgSize.width - 165.0f, 15.0f });
        m_joinRequestsBtn->setZOrder(10);
        m_joinRequestsBtn->setVisible(false);
        fixedMenu->addChild(m_joinRequestsBtn);

        auto sideMenu = CCMenu::create();
        sideMenu->setPosition({ bgSize.width + 25.0f, bgSize.height / 2 + 10.0f });
        m_mainLayer->addChild(sideMenu, 2);

        auto bubbleSprite = CCSprite::createWithSpriteFrameName("BubbleChatSpr.png"_spr);
        bubbleSprite->setScale(0.55f);
        auto bubbleBtn = CCMenuItemSpriteExtra::create(bubbleSprite, this, menu_selector(ChatPopup::onToggleBubble));
        sideMenu->addChild(bubbleBtn);

        auto addContactSprite = CCSprite::createWithSpriteFrameName("AddContanctSpr.png"_spr);
        addContactSprite->setScale(0.55f);
        auto addContactBtn = CCMenuItemSpriteExtra::create(addContactSprite, this, menu_selector(ChatPopup::onOpenAddContact));
        sideMenu->addChild(addContactBtn);

        auto communitySprite = CCSprite::createWithSpriteFrameName("new_group_btn.png"_spr);
        communitySprite->setScale(0.6f);
        auto communityBtn = CCMenuItemSpriteExtra::create(communitySprite, this, menu_selector(ChatPopup::onOpenCommunity));
        sideMenu->addChild(communityBtn);

        auto browseSprite = CCSprite::createWithSpriteFrameName("search_group_btn.png"_spr);
        browseSprite->setScale(0.6f);
        auto browseBtn = CCMenuItemSpriteExtra::create(browseSprite, this, menu_selector(ChatPopup::onBrowseCommunities));
        sideMenu->addChild(browseBtn);

        // INVITATIONS INBOX BUTTON
        auto invitesSprite = CCSprite::createWithSpriteFrameName("accountBtn_messages_001.png");
        invitesSprite->setScale(0.6f);
        auto invitesBtn = CCMenuItemSpriteExtra::create(invitesSprite, this, menu_selector(ChatPopup::onOpenInvites));
        sideMenu->addChild(invitesBtn);

        sideMenu->alignItemsVerticallyWithPadding(12.0f);

        float contactsScrollH = m_chatHeight - 20.0f;
        m_contactsScroll = ScrollLayer::create({ contactsWidth, contactsScrollH });
        m_contactsScroll->setPosition({ contactsBG->getPositionX() - contactsWidth / 2, contactsBG->getPositionY() - contactsScrollH / 2 });
        m_mainLayer->addChild(m_contactsScroll, 1);

        auto contactsScrollbar = Scrollbar::create(m_contactsScroll);
        contactsScrollbar->setPosition({ contactsBG->getPositionX() - (contactsWidth / 2) - 10.0f, contactsBG->getPositionY() });
        m_mainLayer->addChild(contactsScrollbar);

        auto chatBG = CCScale9Sprite::create("square02b_001.png");
        chatBG->setColor({ 0, 0, 0 });
        chatBG->setOpacity(100);
        chatBG->setContentSize({ m_chatWidth, m_chatHeight });
        chatBG->setPosition({ bgSize.width / 2 + 15.0f, bgSize.height / 2 - 5.0f });
        m_mainLayer->addChild(chatBG);

        m_scrollLayer = ScrollLayer::create({ m_chatWidth, m_scrollHeight });
        m_scrollLayer->setPosition({ chatBG->getPositionX() - m_chatWidth / 2, chatBG->getPositionY() - m_chatHeight / 2 + 45.0f });
        m_mainLayer->addChild(m_scrollLayer);

        auto scrollbar = Scrollbar::create(m_scrollLayer);
        scrollbar->setPosition({ chatBG->getPositionX() + m_chatWidth / 2 + 12.0f, chatBG->getPositionY() + 20.0f });
        m_mainLayer->addChild(scrollbar);

        auto inputMenu = CCMenu::create();
        inputMenu->setPosition({ 0, 0 });
        inputMenu->setZOrder(1);
        m_mainLayer->addChild(inputMenu);

        auto stickerBtnSprite = CCSprite::createWithSpriteFrameName("EmojiSpr.png"_spr);
        stickerBtnSprite->setScale(0.5f);
        auto stickerBtn = CCMenuItemSpriteExtra::create(stickerBtnSprite, this, menu_selector(ChatPopup::onAddSticker));
        stickerBtn->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 224.0f, chatBG->getPositionY() - m_chatHeight / 2 + 22.0f });
        inputMenu->addChild(stickerBtn);

        auto inputBg = CCScale9Sprite::create("square02_small.png", CCRectMake(0.0f, 0.0f, 40.0f, 40.0f));
        inputBg->setContentSize({ 235.0f, 30.0f });
        inputBg->setOpacity(90);
        inputBg->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 126.0f, chatBG->getPositionY() - m_chatHeight / 2 + 22.0f });
        m_mainLayer->addChild(inputBg);

        float anchoInput = m_chatWidth - 120.0f;
        m_input = TextInput::create(anchoInput, "Write a message...", "chatFont.fnt");
        m_input->setPosition({ chatBG->getPositionX() - (m_chatWidth / 2) + 110.0f, chatBG->getPositionY() - m_chatHeight / 2 + 22.0f });
        m_input->getBGSprite()->setVisible(false);
        m_input->setTextAlign(geode::TextInputAlign::Left);
        m_input->getInputNode()->setDelegate(this);
        m_input->getInputNode()->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%^&*()-_=+[]{}|;:',.<>?/`~\"\\");
        m_mainLayer->addChild(m_input);

        auto sendBtnSprite = CCSprite::createWithSpriteFrameName("SendSpr.png"_spr);
        sendBtnSprite->setScale(0.5f);
        auto sendBtn = CCMenuItemSpriteExtra::create(sendBtnSprite, this, menu_selector(ChatPopup::onSend));
        sendBtn->setPosition({ chatBG->getPositionX() + (m_chatWidth / 2) - 48.0f, m_input->getPositionY() });
        inputMenu->addChild(sendBtn);

        auto sharedSpr = CCSprite::createWithSpriteFrameName("Shared.png"_spr);
        sharedSpr->setScale(0.5f);
        auto sharedBtn = CCMenuItemSpriteExtra::create(sharedSpr, this, nullptr);
        sharedBtn->setPosition({ chatBG->getPositionX() + (m_chatWidth / 2) - 20.0f, m_input->getPositionY() });
        inputMenu->addChild(sharedBtn);

        this->schedule(schedule_selector(ChatPopup::onPollServer), 3.5f);
        this->schedule(schedule_selector(ChatPopup::onPollContacts), 15.0f);

        m_network->registrarJugador();
        m_contactsNetwork->cargarContactos();

        if (preOpenContact) {
            m_activeChatId = preOpenContact->accountId;
            m_activeChatName = preOpenContact->username;
            m_activeContact = *preOpenContact;
            m_network->cargarMensajes(std::to_string(am->m_accountID), m_activeChatId);
        }

#ifdef GEODE_IS_DESKTOP
        this->scheduleOnce(schedule_selector(ChatPopup::setupScrollMouseHandling), 0.0f);
#endif

        return true;
    }

    void onInfo(CCObject* sender) {
        std::string Info = "<cr>(1) Not be toxic</c>: Don't swear or insult people\n<co>No Spam</c>: No mentioning promos or invitations to other servers, etc....\n<cg>Use common sense</c>: There are many rules that everyone should already know; if I haven't listed one here, you should use common sense and say, This wouldn't be allowed.";
        FLAlertLayer::create(nullptr, "Rules", Info.c_str(), "Okay", nullptr, 360)->show();
    }

    void onAddCommunityMember(CCObject* sender) {
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);
        auto popup = CommunityAddMemberPopup::create(m_activeChatId, myId);
        if (popup) {
            popup->show();
        }
    }

    void onShowMembers(CCObject* sender) {
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);
        auto popup = CommunityMembersPopup::create(m_activeChatId, myId, [this]() {
            if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
            });
        if (popup) popup->show();
    }

    void onEditCommunity(CCObject* sender) {
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);

        if (!m_communityNet) {
            m_communityNet = CommunityNetwork::create();
            m_communityNet->retain();
        }
        m_communityNet->setOnCommunityFound([this, myId](const CommunityInfo& info) {
            auto popup = CommunityEditPopup::create(info.communityId, myId, info, [this]() {
                if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
                });
            if (popup) popup->show();
            });
        m_communityNet->setOnError([](const std::string& err) {
            FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
            });
        m_communityNet->buscarComunidad(m_activeChatId);
    }

    void onDeleteCommunity(CCObject* sender) {
        auto confirm = FLAlertLayer::create(this, "Delete Community",
            "<cr>Are you sure?</c>\nThe community will be deleted for all members.\nThis action cannot be undone.",
            "Cancel", "Delete", 320);
        confirm->setTag(999);
        confirm->show();
    }

    void onLeaveCommunity(CCObject* sender) {
        auto confirm = FLAlertLayer::create(this, "Leave Community",
            "Are you sure you want to <cr>leave</c> this community?",
            "Cancel", "Leave", 320);
        confirm->setTag(998);
        confirm->show();
    }

    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        if (!btn2) return;

        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);

        if (alert->getTag() == 999) {
            if (!m_communityNet) {
                m_communityNet = CommunityNetwork::create();
                m_communityNet->retain();
            }
            m_communityNet->setOnCommunityDeleted([this]() {
                FLAlertLayer::create(nullptr, "Success", "Community deleted!", "OK", nullptr, 300)->show();
                this->resetChatView();
                });
            m_communityNet->setOnError([](const std::string& err) {
                FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
                });
            m_communityNet->eliminarComunidad(myId, m_activeChatId);
        }
        else if (alert->getTag() == 998) {
            if (!m_communityNet) {
                m_communityNet = CommunityNetwork::create();
                m_communityNet->retain();
            }
            m_communityNet->setOnLeftCommunity([this](const std::string& msg) {
                FLAlertLayer::create(nullptr, "Success", msg.c_str(), "OK", nullptr, 300)->show();
                this->resetChatView();
                });
            m_communityNet->setOnError([](const std::string& err) {
                FLAlertLayer::create(nullptr, "Error", err.c_str(), "OK", nullptr, 300)->show();
                });
            m_communityNet->salirComunidad(myId, m_activeChatId);
        }
    }

    void resetChatView() {
        m_activeChatId = "";
        m_activeChatName = "";
        limpiarChat(true);
        if (m_chatSubtitle) m_chatSubtitle->setString("Chat: World");
        if (m_addMemberBtn) m_addMemberBtn->setVisible(false);
        if (m_membersBtn) m_membersBtn->setVisible(false);
        if (m_editCommunityBtn) m_editCommunityBtn->setVisible(false);
        if (m_deleteCommunityBtn) m_deleteCommunityBtn->setVisible(false);
        if (m_joinRequestsBtn) m_joinRequestsBtn->setVisible(false);
        if (m_leaveCommunityBtn) m_leaveCommunityBtn->setVisible(false);
        if (m_contactIndicator) m_contactIndicator->setVisible(false);
        if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
    }

    void onBrowseCommunities(CCObject* sender) {
        auto popup = CommunityBrowserPopup::create([this]() {
            if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
            });
        if (popup) popup->show();
    }

    void onOpenInvites(CCObject* sender) {
        auto popup = CommunityInvitesPopup::create([this]() {
            if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
            });
        if (popup) popup->show();
    }

    void onShowJoinRequests(CCObject* sender) {
        auto am = GJAccountManager::sharedState();
        std::string myId = std::to_string(am->m_accountID);
        auto popup = CommunityJoinRequestsPopup::create(m_activeChatId, myId, [this]() {
            if (m_contactsNetwork) m_contactsNetwork->cargarContactos();
            });
        if (popup) popup->show();
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
        if (!am || am->m_accountID == 0) {
            return;
        }
        std::string myId = std::to_string(am->m_accountID);
        if (!m_activeChatId.empty()) {
            m_network->cargarMensajes(myId, m_activeChatId);
        }
    }

    void onPollContacts(float dt) {
        if (m_contactsNetwork) {
            m_contactsNetwork->cargarContactos();
        }
    }

    virtual void textChanged(CCTextInputNode* input) override {}

    void mostrarContactos(const std::vector<ContactInfo>& contactos) {
        m_contactsScroll->m_contentLayer->removeAllChildren();
        float contactsWidth = 40.0f;
        float spacing = 38.0f;
        float scrollH = m_chatHeight - 20.0f;
        size_t totalItems = contactos.size() + 1;
        float totalHeight = spacing * totalItems;

        if (totalHeight < scrollH) {
            totalHeight = scrollH;
        }

        m_contactsScroll->m_contentLayer->setContentSize({ contactsWidth, totalHeight });
        auto menu = CCMenu::create();
        menu->setContentSize({ contactsWidth, totalHeight }); 
        menu->setPosition({ 0, 0 });
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

            auto nameLabel = CCLabelBMFont::create("Announc...", "chatFont.fnt");
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
            if (contactos[i].glow) {
                contactSprite->setGlowOutline(gm->colorForIdx(contactos[i].glow));
            }
            contactSprite->setScale(0.55f);
            contactSprite->setPosition({ 17.5f, 17.5f });
            contactHitbox->addChild(contactSprite);

            if (contactos[i].unreadCount > 0) {
                std::string unreadText = std::to_string(contactos[i].unreadCount);
                if (contactos[i].unreadCount > 9) {
                    unreadText = "9+";
                }
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
        if (minY > 0.0f) {
            minY = 0.0f;
        }
        m_contactsScroll->m_contentLayer->setPositionY(minY);

        m_contactIndicator = CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
        m_contactIndicator->setContentSize({ 3.0f, 20.0f });
        m_contactIndicator->setInsetLeft(1);
        m_contactIndicator->setInsetRight(1);
        m_contactIndicator->setInsetTop(1);
        m_contactIndicator->setInsetBottom(1);
        m_contactIndicator->setColor({ 255, 255, 255 });
        m_contactIndicator->setOpacity(220);
        m_contactIndicator->setAnchorPoint({ 0.0f, 0.5f });
        m_contactIndicator->setVisible(false);
        m_contactsScroll->m_contentLayer->addChild(m_contactIndicator, 5);

        if (!m_activeChatId.empty()) {
            for (size_t i = 0; i < contactos.size(); i++) {
                if (contactos[i].accountId == m_activeChatId) {
                    float btnY = totalHeight - (spacing * (i + 1)) - spacing + spacing / 2;
                    m_contactIndicator->setPosition({ 0.0f, btnY });
                    m_contactIndicator->setVisible(true);
                    break;
                }
            }
        }

        if (this->isRunning()) {
            geode::cocos::handleTouchPriority(this);
        }
    }

    void onSelectContact(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto dataStr = static_cast<CCString*>(btn->getUserObject());
        if (!dataStr) {
            return;
        }

        if (m_input) {
            m_input->setString("");
        }

        m_activeChatId = dataStr->getCString();

        for (const auto& c : m_contactList) {
            if (c.accountId == m_activeChatId) {
                m_activeContact = c;
                m_activeChatName = c.username;
                break;
            }
        }

        if (m_contactIndicator) {
            float spacing = 38.0f;
            float scrollH = m_chatHeight - 20.0f;
            float totalHeight = spacing * (m_contactList.size() + 1);
            if (totalHeight < scrollH) {
                totalHeight = scrollH;
            }

            for (size_t i = 0; i < m_contactList.size(); i++) {
                if (m_contactList[i].accountId == m_activeChatId) {
                    float targetY = totalHeight - (i + 2) * spacing + spacing / 2.0f;
                    m_contactIndicator->stopAllActions();

                    if (!m_contactIndicator->isVisible()) {
                        m_contactIndicator->setPosition({ 0.0f, targetY });
                        m_contactIndicator->setVisible(true);
                        m_contactIndicator->setOpacity(0);
                        m_contactIndicator->runAction(CCFadeIn::create(0.2f));
                    }
                    else {
                        auto slideTo = CCEaseExponentialOut::create(CCMoveTo::create(0.25f, ccp(0.0f, targetY)));
                        m_contactIndicator->runAction(slideTo);
                    }
                    break;
                }
            }
        }

        limpiarChat(true);

        auto am = GJAccountManager::sharedState();
        if (am && am->m_accountID != 0) {
            std::string myId = std::to_string(am->m_accountID);
            m_network->cargarMensajes(myId, m_activeChatId);

            if (m_addMemberBtn) {
                bool isOwner = (m_activeContact.isCommunity && m_activeContact.ownerId == myId);
                bool isMemberOnly = (m_activeContact.isCommunity && m_activeContact.ownerId != myId);

                m_addMemberBtn->setVisible(isOwner);
                m_membersBtn->setVisible(isOwner);
                m_editCommunityBtn->setVisible(isOwner);
                m_deleteCommunityBtn->setVisible(isOwner);

                bool isPrivateOwner = (isOwner && m_activeContact.isCommunity);
                m_joinRequestsBtn->setVisible(isPrivateOwner);

                m_leaveCommunityBtn->setVisible(isMemberOnly);
            }
        }

        if (m_contactsNetwork) {
            m_contactsNetwork->cargarContactos();
        }

        if (m_chatSubtitle) {
            m_chatSubtitle->setString(("Chat: " + m_activeChatName).c_str());
        }
    }

    void limpiarChat(bool resetCounter) {
        if (resetCounter) {
            m_lastMessageCount = 0;
            m_lastMessageText = "";
        }
        m_messages.clear();
        m_scrollLayer->m_contentLayer->removeAllChildren();
        m_scrollLayer->m_contentLayer->setContentSize({ m_chatWidth, m_scrollHeight });
        m_scrollLayer->m_contentLayer->setPositionY(0.0f);
    }

    void onOpenAddContact(CCObject* sender) {
        auto popup = AddContactPopup::create([this]() {
            m_contactsNetwork->cargarContactos();
            });
        popup->show();
    }

    void onOpenCommunity(CCObject* sender) {
        auto popup = CommunityPopup::create([this]() {
            if (m_contactsNetwork) {
                m_contactsNetwork->cargarContactos();
            }
            });
        if (popup) {
            popup->show();
        }
    }

    void addMessage(const ChatMessage& chatMsg, bool isMe, bool animate = false) {
        const std::string& rawText = chatMsg.texto;
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
            if (msgHeightActual < 45.0f) {
                msgHeightActual = 45.0f;
            }

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
            if (anchoRealTexto > anchoMaxTexto) {
                anchoRealTexto = anchoMaxTexto;
            }

            msgHeightActual = alturaRealTexto + 20.0f;
            if (msgHeightActual < 45.0f) {
                msgHeightActual = 45.0f;
            }

            float bubblePadH = 10.0f;
            float bubblePadV = 8.0f;
            float bubbleW = anchoRealTexto + bubblePadH * 2;
            float bubbleH = alturaRealTexto + bubblePadV * 2;
            if (bubbleW < 30.0f) {
                bubbleW = 30.0f;
            }
            if (bubbleH < 26.0f) {
                bubbleH = 26.0f;
            }

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

            if (isMe) {
                label->setColor({ 100, 255, 100 });
            }
            else {
                label->setColor({ 255, 255, 255 });
            }

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
            if (gm->getPlayerGlow()) {
                player->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
            }
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
            else if (m_activeContact.isCommunity && !chatMsg.senderName.empty()) {
                auto player = SimplePlayer::create(chatMsg.senderIcon);
                player->setColor(gm->colorForIdx(chatMsg.senderCol1));
                player->setSecondColor(gm->colorForIdx(chatMsg.senderCol2));
                if (chatMsg.senderGlow) {
                    player->setGlowOutline(gm->colorForIdx(chatMsg.senderGlow));
                }
                player->setScale(0.65f);
                player->setPosition({ 20.0f, msgHeightActual / 2 });
                msgNode->addChild(player);

                auto nameLabel = CCLabelBMFont::create(chatMsg.senderName.c_str(), "chatFont.fnt");
                nameLabel->setScale(0.35f);
                nameLabel->setColor({ 200, 200, 200 });
                nameLabel->setAnchorPoint({ 0.0f, 0.0f });
                nameLabel->setPosition({ 45.0f, msgHeightActual - 12.0f });
                msgNode->addChild(nameLabel);
            }
            else {
                auto player = SimplePlayer::create(m_activeContact.icon);
                player->setColor(gm->colorForIdx(m_activeContact.col1));
                player->setSecondColor(gm->colorForIdx(m_activeContact.col2));
                if (m_activeContact.glow) {
                    player->setGlowOutline(gm->colorForIdx(m_activeContact.glow));
                }
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
        for (const auto& pair : m_messages) {
            totalHeightMessages += pair.second;
        }

        float totalHeight = totalHeightMessages + 15.0f;
        if (totalHeight < m_scrollHeight) {
            totalHeight = m_scrollHeight;
        }

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
        if (!s_contactPriority.empty()) {
            return;
        }
        auto path = Mod::get()->getSaveDir() / "contact_priority.json";
        std::ifstream file(path);
        if (!file.is_open()) {
            return;
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        if (content.empty()) {
            return;
        }
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
        if (s_contactPriority.empty()) {
            return;
        }
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
        for (const auto& c : rest) {
            ordered.push_back(c);
        }
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
                if (i == 0) {
                    return;
                }
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
        if (text.empty() || m_activeChatId.empty()) {
            return;
        }
        auto am = GJAccountManager::sharedState();
        if (!am || am->m_accountID == 0) {
            return;
        }
        m_input->setString("");
        m_animateLastSent = true;
        m_network->enviarMensaje(std::to_string(am->m_accountID), m_activeChatId, text);
        m_network->cargarMensajes(std::to_string(am->m_accountID), m_activeChatId);
        moverContactoAlInicio(m_activeChatId);
    }

    virtual void textInputReturn(CCTextInputNode* textInput) override {
        this->onSend(nullptr);
    }

    void onClose(CCObject* sender) override {
        if (!m_closedByBubble && ChatBubble::s_instance) {
            ChatBubble::s_instance->hide();
        }

        this->unschedule(schedule_selector(ChatPopup::onPollServer));
        this->unschedule(schedule_selector(ChatPopup::onPollContacts));

        if (m_network && !m_activeChatId.empty()) {
            auto am = GJAccountManager::sharedState();
            if (am && am->m_accountID != 0) {
                m_network->limpiarHistorial(std::to_string(am->m_accountID), m_activeChatId);
            }
        }

        if (m_network) {
            m_network->setLogCallback(nullptr);
            m_network->setOnMessagesLoaded(nullptr);
            m_network->release();
            m_network = nullptr;
        }
        if (m_contactsNetwork) {
            m_contactsNetwork->setOnContactsLoaded(nullptr);
            m_contactsNetwork->release();
            m_contactsNetwork = nullptr;
        }
        if (m_communityNet) {
            m_communityNet->release();
            m_communityNet = nullptr;
        }
#ifdef GEODE_IS_DESKTOP
        CCDirector::sharedDirector()->getMouseDispatcher()->removeDelegate(static_cast<CCMouseDelegate*>(this));
#endif
        Popup::onClose(sender);
    }

public:
    static ChatPopup* create(ContactInfo* preOpenContact = nullptr) {
        auto ret = new ChatPopup();
        if (ret && ret->init(preOpenContact)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};