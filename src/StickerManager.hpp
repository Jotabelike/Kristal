#pragma once
#include <Geode/Geode.hpp>
#include <unordered_map>
#include <string>
#include <vector>

using namespace geode::prelude;

struct StickerInfo {
    std::string name; 
    bool isModSprite;
};

class StickerManager {
public:

    static std::vector<std::string> getStickerCommands() {
        return {
            ":w1:", ":w2:", ":w3:", ":w4:", ":w5:", ":w6:", ":w7:", ":w8:", ":w9:", ":w10:",
            ":w11:", ":w12:", ":w13:", ":w14:", ":w15:", ":w16:", ":w17:", ":w18:", ":w19:", ":w20:",
            ":w21:", ":w22:", ":w23:", ":w24:", ":w25:", ":w26:", ":w27:", ":w28:", ":w29:", ":w30:",
            ":w31:", ":w32:", ":w33:", ":w34:", ":w35:", ":w36:", ":w37:", ":w38:", ":w39:", ":w40:",
            ":w41:", ":w42:", ":w43:", ":w44:", ":w45:", ":w46:", ":w47:", ":w48:", ":w49:", ":w50:",
            ":w51:", ":w52:", ":w53:", ":w54:", ":w55:", ":w56:", ":w57:", ":w58:", ":w59:", ":w60:",
            ":w61:", ":w62:", ":w63:", ":w64:", ":w65:", ":w66:", ":w67:", ":w68:", ":w69:", ":w70:",
			":w71:", ":w72:", ":w73:", ":w74:", ":w75:", ":w76:", ":w77:", ":w78:", ":w79:", ":w80:"
        };
    }

    static std::vector<std::string> getAllCommands() {
        return getStickerCommands();
    }

    static const std::unordered_map<std::string, StickerInfo>& getStickerMap() {
        static std::unordered_map<std::string, StickerInfo> stickers = {
            {":w1:",  {"w1.png",  true}},
            {":w2:",  {"w2.png",  true}},
            {":w3:",  {"w3.png",  true}},
            {":w4:",  {"w4.png",  true}},
            {":w5:",  {"w5.png",  true}},
            {":w6:",  {"w6.png",  true}},
            {":w7:",  {"w7.png",  true}},
            {":w8:",  {"w8.png",  true}},
            {":w9:",  {"w9.png",  true}},
            {":w10:", {"w10.png", true}},
            {":w11:", {"w11.png", true}},
            {":w12:", {"w12.png", true}},
            {":w13:", {"w13.png", true}},
            {":w14:", {"w14.png", true}},
            {":w15:", {"w15.png", true}},
            {":w16:", {"w16.png", true}},
            {":w17:", {"w17.png", true}},
            {":w18:", {"w18.png", true}},
            {":w19:", {"w19.png", true}},
            {":w20:", {"w20.png", true}},
            {":w21:", {"w21.png", true}},
            {":w22:", {"w22.png", true}},
            {":w23:", {"w23.png", true}},
            {":w24:", {"w24.png", true}},
            {":w25:", {"w25.png", true}},
            {":w26:", {"w26.png", true}},
            {":w27:", {"w27.png", true}},
            {":w28:", {"w28.png", true}},
            {":w29:", {"w29.png", true}},
            {":w30:", {"w30.png", true}},
            {":w31:", {"w31.png", true}},
            {":w32:", {"w32.png", true}},
            {":w33:", {"w33.png", true}},
            {":w34:", {"w34.png", true}},
            {":w35:", {"w35.png", true}},
            {":w36:", {"w36.png", true}},
            {":w37:", {"w37.png", true}},
            {":w38:", {"w38.png", true}},
            {":w39:", {"w39.png", true}},
            {":w40:", {"w40.png", true}},
            {":w41:", {"w41.png", true}},
            {":w42:", {"w42.png", true}},
            {":w43:", {"w43.png", true}},
            {":w44:", {"w44.png", true}},
            {":w45:", {"w45.png", true}},
            {":w46:", {"w46.png", true}},
            {":w47:", {"w47.png", true}},
            {":w48:", {"w48.png", true}},
            {":w49:", {"w49.png", true}},
            {":w50:", {"w50.png", true}},
            {":w51:", {"w51.png", true}},
            {":w52:", {"w52.png", true}},
            {":w53:", {"w53.png", true}},
            {":w54:", {"w54.png", true}},
            {":w55:", {"w55.png", true}},
            {":w56:", {"w56.png", true}},
            {":w57:", {"w57.png", true}},
            {":w58:", {"w58.png", true}},
            {":w59:", {"w59.png", true}},
            {":w60:", {"w60.png", true}},
            {":w61:", {"w61.png", true}},
            {":w62:", {"w62.png", true}},
            {":w63:", {"w63.png", true}},
            {":w64:", {"w64.png", true}},
            {":w65:", {"w65.png", true}},
            {":w66:", {"w66.png", true}},
            {":w67:", {"w67.png", true}},
            {":w68:", {"w68.png", true}},
            {":w69:", {"w69.png", true}},
            {":w70:", {"w70.png", true}},
            {":w71:", {"w71.png", true}},
            {":w72:", {"w72.png", true}},
            {":w73:", {"w73.png", true}},
            {":w74:", {"w74.png", true}},
            {":w75:", {"w75.png", true}},
            {":w76:", {"w76.png", true}},
            {":w77:", {"w77.png", true}},
            {":w78:", {"w78.png", true}},
            {":w79:", {"w79.png", true}},
			{":w80:", {"w80.png", true}}
        };
        return stickers;
    }

    static void applyMaxScale(CCSprite* spr, float maxSize = 40.0f) {
        if (!spr) return;
        auto sprSize = spr->getContentSize();
        float scale = 1.0f;
        if (sprSize.width > 0 && sprSize.height > 0) {
            float scaleX = maxSize / sprSize.width;
            float scaleY = maxSize / sprSize.height;
            scale = std::min(scaleX, scaleY);
        }
        spr->setScale(scale);
    }

    static CCSprite* createStickerPreview(const std::string& command) {
        auto& stickers = getStickerMap();
        auto it = stickers.find(command);
        if (it == stickers.end()) return nullptr;

        CCSprite* spr = nullptr;
        auto& info = it->second;

        if (info.isModSprite) {
            auto path = Mod::get()->getResourcesDir() / info.name;
            spr = CCSprite::create(path.string().c_str());
        }
        else {
            spr = CCSprite::createWithSpriteFrameName(info.name.c_str());
        }

        if (!spr) {
            spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            log::warn("No se pudo cargar preview (asegúrate de tener el .png): {}", info.name);
        }

        applyMaxScale(spr);
        return spr;
    }

    static CCSprite* createSticker(const std::string& command) {
        auto& stickers = getStickerMap();
        auto it = stickers.find(command);
        if (it == stickers.end()) return nullptr;

        CCSprite* spr = nullptr;
        auto& info = it->second;

        if (info.isModSprite) {
            auto path = Mod::get()->getResourcesDir() / info.name;
            spr = CCSprite::create(path.string().c_str());
        }
        else {
            spr = CCSprite::createWithSpriteFrameName(info.name.c_str());
        }

        if (!spr) {
            spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            log::warn("No se pudo cargar el sticker final: {}", info.name);
        }

        applyMaxScale(spr);
        return spr;
    }
};