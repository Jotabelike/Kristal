#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ChatPopup.hpp" 

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {

    bool init() {
        
        if (!MenuLayer::init()) return false;

     
      
        auto bottomMenu = this->getChildByID("bottom-menu");

        if (bottomMenu) {
            
            auto chatIcon = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");

          
            auto chatBtn = CCMenuItemSpriteExtra::create(
                chatIcon,
                this,
                menu_selector(MyMenuLayer::onOpenChat)  
            );

          
            chatBtn->setID("kristal-chat-button"_spr);

           
            bottomMenu->addChild(chatBtn);

       
            bottomMenu->updateLayout();
        }

        return true;
    }

 
    void onOpenChat(CCObject * sender) {
         
        ChatPopup::create()->show();
    }
};