#pragma once
#include <Geode/Geode.hpp>
#include <cocos-ext.h>
#include <functional>
#include <vector>
#include <string>

using namespace geode::prelude;
using namespace cocos2d::extension;

struct ChatMessage {
    std::string senderId;
    std::string texto;
    std::string senderName;
    int senderIcon = 0;
    int senderCol1 = 0;
    int senderCol2 = 3;
    int senderGlow = 0;
};

class ChatNetwork : public CCObject {
protected:
    std::string m_baseUrl = "https://kristal-backend-9aow.onrender.com";
    std::function<void(const std::string&)> m_logCallback;
    std::function<void(const std::vector<ChatMessage>&)> m_onMessagesLoaded;
    std::function<void(bool)> m_onTypingStatus;

    void log(const std::string& msg) { if (m_logCallback) m_logCallback(msg); }

    std::string urlEncode(const std::string& value) {
        std::string result;
        for (char c : value) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') result += c;
            else if (c == ' ') result += '+';
            else { char hex[4]; snprintf(hex, sizeof(hex), "%%%02X", (unsigned char)c); result += hex; }
        }
        return result;
    }

    std::string extractJsonValue(const std::string& obj, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = obj.find(searchKey);
        if (keyPos == std::string::npos) return "";
        size_t colonPos = obj.find(':', keyPos + searchKey.length());
        if (colonPos == std::string::npos) return "";
        size_t valStart = colonPos + 1;
        while (valStart < obj.size() && obj[valStart] == ' ') valStart++;
        if (valStart >= obj.size()) return "";
        if (obj[valStart] == '"') {
            valStart++;
            size_t valEnd = obj.find('"', valStart);
            if (valEnd == std::string::npos) return "";
            return obj.substr(valStart, valEnd - valStart);
        }
        else {
            size_t valEnd = valStart;
            while (valEnd < obj.size() && obj[valEnd] != ',' && obj[valEnd] != '}' && obj[valEnd] != ' ') valEnd++;
            return obj.substr(valStart, valEnd - valStart);
        }
    }

public:
    static ChatNetwork* create() {
        auto ret = new ChatNetwork();
        if (ret) { ret->autorelease(); return ret; }
        return nullptr;
    }

    void setLogCallback(std::function<void(const std::string&)> callback) { m_logCallback = callback; }
    void setOnMessagesLoaded(std::function<void(const std::vector<ChatMessage>&)> callback) { m_onMessagesLoaded = callback; }
    void setOnTypingStatus(std::function<void(bool)> callback) { m_onTypingStatus = callback; }

    void limpiarHistorial(const std::string& fromId, const std::string& toId) {
        if (fromId.empty() || toId.empty()) return;

        std::string postData = "fromId=" + fromId + "&toId=" + urlEncode(toId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/limpiar_historial").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);


        request->setRequestData(postData.c_str(), postData.length());


        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void registrarJugador() {
        auto am = GJAccountManager::sharedState();
        auto gm = GameManager::sharedState();
        int accountId = am->m_accountID;
        std::string username = am->m_username;

        if (accountId == 0) { log("Error: No hay cuenta de GD iniciada."); return; }
        if (username.empty()) username = "UsuarioDesconocido";


        int icon = gm->getPlayerFrame();
        int col1 = gm->getPlayerColor();
        int col2 = gm->getPlayerColor2();
        int glow = gm->getPlayerGlow() ? 1 : 0;

        std::string postData = "accountId=" + std::to_string(accountId)
            + "&username=" + urlEncode(username)
            + "&icon=" + std::to_string(icon)
            + "&col1=" + std::to_string(col1)
            + "&col2=" + std::to_string(col2)
            + "&glow=" + std::to_string(glow);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/registrar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);

        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ChatNetwork::onRegistroResponse));
        request->setTag("registro");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onRegistroResponse(CCHttpClient* sender, CCHttpResponse* response) {}


    void enviarMensaje(const std::string& fromId, const std::string& toId, const std::string& texto) {
        if (fromId.empty() || toId.empty() || texto.empty()) return;
        std::string postData = "fromId=" + fromId + "&toId=" + toId + "&texto=" + urlEncode(texto);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/enviar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded"); request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ChatNetwork::onEnviarResponse));
        request->setTag("enviar");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }
    void onEnviarResponse(CCHttpClient* sender, CCHttpResponse* response) {}

    void cargarMensajes(const std::string& fromId, const std::string& toId) {
        if (toId.empty()) return;
        std::string postData = "fromId=" + fromId + "&toId=" + toId;
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/mensajes").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded"); request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ChatNetwork::onMensajesResponse));
        request->setTag("cargarMensajes");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onMensajesResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) return;
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());

        if (m_onMessagesLoaded) {
            std::vector<ChatMessage> mensajes;
            size_t pos = 0;
            while ((pos = body.find('{', pos)) != std::string::npos) {
                size_t end = body.find('}', pos);
                if (end == std::string::npos) break;
                std::string obj = body.substr(pos, end - pos + 1);

                ChatMessage msg;
                msg.senderId = extractJsonValue(obj, "senderId");
                msg.texto = extractJsonValue(obj, "texto");
                msg.senderName = extractJsonValue(obj, "senderName");

                std::string iconStr = extractJsonValue(obj, "senderIcon");
                if (!iconStr.empty()) msg.senderIcon = std::stoi(iconStr);
                std::string col1Str = extractJsonValue(obj, "senderCol1");
                if (!col1Str.empty()) msg.senderCol1 = std::stoi(col1Str);
                std::string col2Str = extractJsonValue(obj, "senderCol2");
                if (!col2Str.empty()) msg.senderCol2 = std::stoi(col2Str);
                std::string glowStr = extractJsonValue(obj, "senderGlow");
                if (!glowStr.empty()) msg.senderGlow = std::stoi(glowStr);

                if (!msg.texto.empty()) mensajes.push_back(msg);
                pos = end + 1;
            }
            m_onMessagesLoaded(mensajes);
        }
    }

    void enviarEscribiendo(const std::string& fromId, const std::string& toId, bool isTyping) {
        if (fromId.empty() || toId.empty()) return;
        std::string postData = "fromId=" + fromId + "&toId=" + toId + "&isTyping=" + (isTyping ? "1" : "0");
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/escribiendo").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());



        request->setTag("escribiendo");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void checkEscribiendo(const std::string& fromId, const std::string& toId) {
        if (fromId.empty() || toId.empty()) return;
        std::string postData = "fromId=" + fromId + "&toId=" + toId;
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/estado_escribiendo").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded"); request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ChatNetwork::onCheckEscribiendo));
        request->setTag("checkEscribiendo");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onCheckEscribiendo(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) return;
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());

        std::string esc = extractJsonValue(body, "escribiendo");
        if (m_onTypingStatus) {
            m_onTypingStatus(esc == "1");
        }
    }
};