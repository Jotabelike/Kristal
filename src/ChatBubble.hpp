#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include "ContactsNetwork.hpp"

using namespace geode::prelude;

class ChatBubble : public CCNode, public CCTargetedTouchDelegate {
protected:
    SimplePlayer* m_playerIcon = nullptr;
    CCLabelBMFont* m_badgeLabel = nullptr;
    CCScale9Sprite* m_bg = nullptr;

    ContactInfo m_contact;
    std::function<void(ContactInfo)> m_openChatCallback;
    ContactsNetwork* m_contactsNetwork = nullptr;

    bool m_isDragging = false;
    CCPoint m_touchStartPos;
    CCPoint m_nodeStartPos;

    bool init() {
        if (!CCNode::init()) return false;

        this->setContentSize({ 45.f, 45.f });
        this->setAnchorPoint({ 0.5f, 0.5f });

        this->ignoreAnchorPointForPosition(false);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setPosition({ winSize.width - 30.f, winSize.height / 2 });

        m_bg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        m_bg->setContentSize({ 55.f, 55.f });
        m_bg->setPosition({ 22.5f, 22.5f });
        m_bg->setColor({ 0, 0, 0 });
        m_bg->setOpacity(0);
        this->addChild(m_bg, -1);

        m_badgeLabel = CCLabelBMFont::create("1", "bigFont.fnt");
        m_badgeLabel->setScale(0.40f);
        m_badgeLabel->setColor({ 255, 60, 60 });
        m_badgeLabel->setPosition({ 45.0f, 45.0f });
        this->addChild(m_badgeLabel, 10);
        m_badgeLabel->setVisible(false);

        m_contactsNetwork = ContactsNetwork::create();
        m_contactsNetwork->retain();
        m_contactsNetwork->setOnContactsLoaded([this](const std::vector<ContactInfo>& contactos) {
            for (const auto& c : contactos) {
                if (c.accountId == m_contact.accountId) {
                    m_contact = c;
                    if (c.unreadCount > 0) {
                        m_badgeLabel->setVisible(true);
                        std::string t = std::to_string(c.unreadCount);
                        if (c.unreadCount > 9) t = "9+";
                        m_badgeLabel->setString(t.c_str());
                    }
                    else {
                        m_badgeLabel->setVisible(false);
                    }
                    break;
                }
            }
            });

        this->schedule(schedule_selector(ChatBubble::onPoll), 3.0f);

        return true;
    }

    void onPoll(float dt) {
        if (this->isVisible() && !m_contact.accountId.empty()) {
            auto am = GJAccountManager::sharedState();
            if (am && am->m_accountID != 0) {
                m_contactsNetwork->cargarContactos();
            }
        }
    }

    ~ChatBubble() {
        if (m_contactsNetwork) m_contactsNetwork->release();
    }

    void onEnter() override {
        CCNode::onEnter();
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -5000, true);
    }

    void onExit() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        CCNode::onExit();
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (!this->isVisible() || !this->getParent()) return false;

        auto pos = this->getParent()->convertTouchToNodeSpace(touch);

        CCRect bbox = this->boundingBox();
        bbox.origin.x -= 10; bbox.origin.y -= 10;
        bbox.size.width += 20; bbox.size.height += 20;

        if (bbox.containsPoint(pos)) {
            m_isDragging = false;
            m_touchStartPos = touch->getLocation();
            m_nodeStartPos = this->getPosition();

            this->stopAllActions();
            this->runAction(CCScaleTo::create(0.1f, 0.9f));
            return true;
        }
        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        auto currentPos = touch->getLocation();
        if (ccpDistance(currentPos, m_touchStartPos) > 5.0f) {
            m_isDragging = true;
        }
        if (m_isDragging) {
            CCPoint delta = ccpSub(currentPos, m_touchStartPos);
            this->setPosition(ccpAdd(m_nodeStartPos, delta));
        }
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        this->runAction(CCScaleTo::create(0.1f, 1.0f));

        if (!m_isDragging) {
            if (m_openChatCallback) m_openChatCallback(m_contact);
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto pos = this->getPosition();
        if (pos.x < 30.f) pos.x = 30.f;
        if (pos.x > winSize.width - 30.f) pos.x = winSize.width - 30.f;
        if (pos.y < 30.f) pos.y = 30.f;
        if (pos.y > winSize.height - 30.f) pos.y = winSize.height - 30.f;

        if (pos.x != this->getPositionX() || pos.y != this->getPositionY()) {
            this->runAction(CCEaseElasticOut::create(CCMoveTo::create(0.3f, pos), 0.6f));
        }
    }

public:
    inline static ChatBubble* s_instance = nullptr;

    static ChatBubble* sharedBubble() {
        if (!s_instance) {
            auto director = CCDirector::sharedDirector();
            if (!director->getNotificationNode()) {
                director->setNotificationNode(CCNode::create());
            }

            if (auto old = director->getNotificationNode()->getChildByID("kristal-chat-bubble")) {
                old->removeFromParent();
            }

            s_instance = new ChatBubble();
            if (s_instance && s_instance->init()) {
                s_instance->setID("kristal-chat-bubble");
                director->getNotificationNode()->addChild(s_instance, 99999);
            }
            else {
                CC_SAFE_DELETE(s_instance);
            }
        }
        return s_instance;
    }

    static void destroyBubble() {
        if (s_instance) {
            s_instance->removeFromParent();
            s_instance = nullptr;
        }
    }

    void setupCallback(std::function<void(ContactInfo)> cb) {
        m_openChatCallback = cb;
    }

    void updateInfo(const ContactInfo& contact) {
        m_contact = contact;
        auto gm = GameManager::sharedState();

        if (m_playerIcon) {
            m_playerIcon->removeFromParent();
            m_playerIcon = nullptr;
        }

        m_playerIcon = SimplePlayer::create(contact.icon);
        m_playerIcon->setColor(gm->colorForIdx(contact.col1));
        m_playerIcon->setSecondColor(gm->colorForIdx(contact.col2));
        if (contact.glow) m_playerIcon->setGlowOutline(gm->colorForIdx(contact.glow));

        m_playerIcon->setScale(0.85f);
        m_playerIcon->setPosition({ 22.5f, 22.5f });
        this->addChild(m_playerIcon, 0);

        if (contact.unreadCount > 0) {
            m_badgeLabel->setVisible(true);
            std::string t = std::to_string(contact.unreadCount);
            if (contact.unreadCount > 9) t = "9+";
            m_badgeLabel->setString(t.c_str());
        }
        else {
            m_badgeLabel->setVisible(false);
        }

        this->setVisible(true);

        this->stopAllActions();
        this->setScale(0.0f);
        this->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.5f, 1.0f), 0.6f));
    }

    void hide() {
        this->setVisible(false);
    }
};