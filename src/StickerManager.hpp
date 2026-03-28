#pragma once
#include <Geode/Geode.hpp>
#include <unordered_map>
#include <string>
#include <vector>

using namespace geode::prelude;

enum class StickerType {
    Sticker,
    Gif
};

struct StickerInfo {
    std::string name; // "w1.png" o "a1"
    bool isModSprite;
    StickerType type;
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
            ":w71:"
        };
    }

  
    static std::vector<std::string> getGifCommands() {
        return {
            ":a1:", ":a2:", ":a3:", ":a4:",
            // Comentados para desactivar previas animadas pesadas:
            // ":a5:", ":a6:", ":a7:", ":a8:", ":a9:", ":a10:",
            // ":a11:", ":a12:", ":a13:", ":a14:", ":a15:", ":a16:", ":a17:", ":a18:", ":a19:", ":a20:",
            // ":a21:", ":a22:", ":a23:", ":a24:", ":a25:", ":a26:", ":a27:", ":a28:", ":a29:", ":a30:",
            // ":a31:", ":a32:", ":a33:", ":a34:", ":a35:", ":a36:", ":a37:", ":a38:"
        };
    }

    static std::vector<std::string> getAllCommands() {
        auto stickers = getStickerCommands();
        auto gifs = getGifCommands();
        stickers.insert(stickers.end(), gifs.begin(), gifs.end());
        return stickers;
    }

    static const std::unordered_map<std::string, StickerInfo>& getStickerMap() {
        static std::unordered_map<std::string, StickerInfo> stickers = {
       
            {":w1:",  {"w1.png",  true, StickerType::Sticker}},
            {":w2:",  {"w2.png",  true, StickerType::Sticker}},
            {":w3:",  {"w3.png",  true, StickerType::Sticker}},
            {":w4:",  {"w4.png",  true, StickerType::Sticker}},
            {":w5:",  {"w5.png",  true, StickerType::Sticker}},
            {":w6:",  {"w6.png",  true, StickerType::Sticker}},
            {":w7:",  {"w7.png",  true, StickerType::Sticker}},
            {":w8:",  {"w8.png",  true, StickerType::Sticker}},
            {":w9:",  {"w9.png",  true, StickerType::Sticker}},
            {":w10:", {"w10.png", true, StickerType::Sticker}},
            {":w11:", {"w11.png", true, StickerType::Sticker}},
            {":w12:", {"w12.png", true, StickerType::Sticker}},
            {":w13:", {"w13.png", true, StickerType::Sticker}},
            {":w14:", {"w14.png", true, StickerType::Sticker}},
            {":w15:", {"w15.png", true, StickerType::Sticker}},
            {":w16:", {"w16.png", true, StickerType::Sticker}},
            {":w17:", {"w17.png", true, StickerType::Sticker}},
            {":w18:", {"w18.png", true, StickerType::Sticker}},
            {":w19:", {"w19.png", true, StickerType::Sticker}},
            {":w20:", {"w20.png", true, StickerType::Sticker}},
            {":w21:", {"w21.png", true, StickerType::Sticker}},
            {":w22:", {"w22.png", true, StickerType::Sticker}},
            {":w23:", {"w23.png", true, StickerType::Sticker}},
            {":w24:", {"w24.png", true, StickerType::Sticker}},
            {":w25:", {"w25.png", true, StickerType::Sticker}},
            {":w26:", {"w26.png", true, StickerType::Sticker}},
            {":w27:", {"w27.png", true, StickerType::Sticker}},
            {":w28:", {"w28.png", true, StickerType::Sticker}},
            {":w29:", {"w29.png", true, StickerType::Sticker}},
            {":w30:", {"w30.png", true, StickerType::Sticker}},
            {":w31:", {"w31.png", true, StickerType::Sticker}},
            {":w32:", {"w32.png", true, StickerType::Sticker}},
            {":w33:", {"w33.png", true, StickerType::Sticker}},
            {":w34:", {"w34.png", true, StickerType::Sticker}},
            {":w35:", {"w35.png", true, StickerType::Sticker}},
            {":w36:", {"w36.png", true, StickerType::Sticker}},
            {":w37:", {"w37.png", true, StickerType::Sticker}},
            {":w38:", {"w38.png", true, StickerType::Sticker}},
            {":w39:", {"w39.png", true, StickerType::Sticker}},
            {":w40:", {"w40.png", true, StickerType::Sticker}},
            {":w41:", {"w41.png", true, StickerType::Sticker}},
            {":w42:", {"w42.png", true, StickerType::Sticker}},
            {":w43:", {"w43.png", true, StickerType::Sticker}},
            {":w44:", {"w44.png", true, StickerType::Sticker}},
            {":w45:", {"w45.png", true, StickerType::Sticker}},
            {":w46:", {"w46.png", true, StickerType::Sticker}},
            {":w47:", {"w47.png", true, StickerType::Sticker}},
            {":w48:", {"w48.png", true, StickerType::Sticker}},
            {":w49:", {"w49.png", true, StickerType::Sticker}},
            {":w50:", {"w50.png", true, StickerType::Sticker}},
            {":w51:", {"w51.png", true, StickerType::Sticker}},
            {":w52:", {"w52.png", true, StickerType::Sticker}},
            {":w53:", {"w53.png", true, StickerType::Sticker}},
            {":w54:", {"w54.png", true, StickerType::Sticker}},
            {":w55:", {"w55.png", true, StickerType::Sticker}},
            {":w56:", {"w56.png", true, StickerType::Sticker}},
            {":w57:", {"w57.png", true, StickerType::Sticker}},
            {":w58:", {"w58.png", true, StickerType::Sticker}},
            {":w59:", {"w59.png", true, StickerType::Sticker}},
            {":w60:", {"w60.png", true, StickerType::Sticker}},
            {":w61:", {"w61.png", true, StickerType::Sticker}},
            {":w62:", {"w62.png", true, StickerType::Sticker}},
            {":w63:", {"w63.png", true, StickerType::Sticker}},
            {":w64:", {"w64.png", true, StickerType::Sticker}},
            {":w65:", {"w65.png", true, StickerType::Sticker}},
            {":w66:", {"w66.png", true, StickerType::Sticker}},
            {":w67:", {"w67.png", true, StickerType::Sticker}},
            {":w68:", {"w68.png", true, StickerType::Sticker}},
            {":w69:", {"w69.png", true, StickerType::Sticker}},
            {":w70:", {"w70.png", true, StickerType::Sticker}},
            {":w71:", {"w71.png", true, StickerType::Sticker}},

            
            {":a1:",  {"a1",  true, StickerType::Gif}},
            {":a2:",  {"a2",  true, StickerType::Gif}},
            {":a3:",  {"a3",  true, StickerType::Gif}},
            {":a4:",  {"a4",  true, StickerType::Gif}},
           
        };
        return stickers;
    }

    static bool isGif(const std::string& command) {
        auto& stickers = getStickerMap();
        auto it = stickers.find(command);
        if (it == stickers.end()) return false;
        return it->second.type == StickerType::Gif;
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
            std::string fileName;
            if (info.type == StickerType::Gif) {
             
                fileName = info.name + ".gif";
            }
            else {
                fileName = info.name;  
            }

            auto path = Mod::get()->getResourcesDir() / fileName;
            spr = CCSprite::create(path.string().c_str());
        }
        else {
            spr = CCSprite::createWithSpriteFrameName(info.name.c_str());
        }

        if (!spr) {
            spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            log::warn("No se pudo cargar preview (asegúrate de tener el .gif/.png): {}", info.name);
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
            std::string fileName;
            if (info.type == StickerType::Gif) {
                fileName = info.name + ".gif";
            }
            else {
                fileName = info.name;
            }

            auto path = Mod::get()->getResourcesDir() / fileName;
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