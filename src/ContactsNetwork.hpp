#pragma once
#include <Geode/Geode.hpp>
#include <cocos-ext.h>
#include <functional>
#include <vector>
#include <string>

using namespace geode::prelude;
using namespace cocos2d::extension;

struct ContactInfo {
    std::string accountId;
    std::string username;
    int icon = 0;
    int col1 = 0;
    int col2 = 3;
    int glow = 0;
    int unreadCount = 0;
    bool isCommunity = false;
    std::string ownerId = "";
};

class ContactsNetwork : public CCObject {
protected:
    std::string m_baseUrl = "https://kristal-backend-9aow.onrender.com";
    std::string m_lastContactsJson = "";

    std::function<void(const std::string&)> m_logCallback;
    std::function<void(const std::vector<ContactInfo>&)> m_onContactsLoaded;
    std::function<void(const std::vector<ContactInfo>&)> m_onRequestsLoaded;
    std::function<void()> m_onRequestAccepted;
    std::function<void(bool, const std::string&)> m_onSolicitudSent;

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

    std::vector<ContactInfo> parseContactArray(const std::string& json) {
        std::vector<ContactInfo> result;
        size_t pos = 0;
        while ((pos = json.find('{', pos)) != std::string::npos) {
            size_t end = json.find('}', pos);
            if (end == std::string::npos) break;
            std::string obj = json.substr(pos, end - pos + 1);

            ContactInfo info;
            info.accountId = extractJsonValue(obj, "accountId");
            info.username = extractJsonValue(obj, "username");

            std::string iconStr = extractJsonValue(obj, "icon");
            if (!iconStr.empty()) info.icon = std::stoi(iconStr);
            std::string col1Str = extractJsonValue(obj, "col1");
            if (!col1Str.empty()) info.col1 = std::stoi(col1Str);
            std::string col2Str = extractJsonValue(obj, "col2");
            if (!col2Str.empty()) info.col2 = std::stoi(col2Str);
            std::string glowStr = extractJsonValue(obj, "glow");
            if (!glowStr.empty()) info.glow = std::stoi(glowStr);


            std::string unreadStr = extractJsonValue(obj, "unreadCount");
            if (!unreadStr.empty()) info.unreadCount = std::stoi(unreadStr);

            std::string communityStr = extractJsonValue(obj, "isCommunity");
            info.isCommunity = (communityStr == "true" || communityStr == "1");

            info.ownerId = extractJsonValue(obj, "ownerId");

            if (!info.accountId.empty()) result.push_back(info);
            pos = end + 1;
        }
        return result;
    }

public:
    static ContactsNetwork* create() {
        auto ret = new ContactsNetwork();
        if (ret) { ret->autorelease(); return ret; }
        return nullptr;
    }

    void setLogCallback(std::function<void(const std::string&)> cb) { m_logCallback = cb; }
    void setOnContactsLoaded(std::function<void(const std::vector<ContactInfo>&)> cb) { m_onContactsLoaded = cb; }
    void setOnRequestsLoaded(std::function<void(const std::vector<ContactInfo>&)> cb) { m_onRequestsLoaded = cb; }
    void setOnRequestAccepted(std::function<void()> cb) { m_onRequestAccepted = cb; }
    void setOnSolicitudSent(std::function<void(bool, const std::string&)> cb) { m_onSolicitudSent = cb; }


    void enviarSolicitud(const std::string& toAccountId) {
        auto am = GJAccountManager::sharedState();
        int myId = am->m_accountID;
        std::string myUsername = am->m_username;
        if (myId == 0) return;
        if (myUsername.empty()) myUsername = "Desconocido";

        std::string postData = "fromId=" + std::to_string(myId) + "&fromUsername=" + urlEncode(myUsername) + "&toId=" + urlEncode(toAccountId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/solicitud/enviar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ContactsNetwork::onEnviarSolicitudResponse));
        request->setTag("enviarSolicitud");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onEnviarSolicitudResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) { if (m_onSolicitudSent) m_onSolicitudSent(false, "Sin respuesta."); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        if (response->isSucceed()) { if (m_onSolicitudSent) m_onSolicitudSent(true, "Solicitud enviada!"); }
        else { std::string errorMsg = extractJsonValue(body, "error"); if (errorMsg.empty()) errorMsg = body; if (m_onSolicitudSent) m_onSolicitudSent(false, errorMsg); }
    }

    void cargarSolicitudes() {
        auto am = GJAccountManager::sharedState();
        if (am->m_accountID == 0) return;
        std::string postData = "accountId=" + std::to_string(am->m_accountID);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/solicitudes").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ContactsNetwork::onSolicitudesResponse));
        request->setTag("cargarSolicitudes");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onSolicitudesResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) { if (m_onRequestsLoaded) m_onRequestsLoaded({}); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto solicitudes = parseContactArray(body);
        if (m_onRequestsLoaded) m_onRequestsLoaded(solicitudes);
    }

    void aceptarSolicitud(const std::string& fromId) {
        auto am = GJAccountManager::sharedState();
        std::string myUsername = am->m_username.empty() ? "Desconocido" : am->m_username;
        std::string postData = "fromId=" + urlEncode(fromId) + "&toId=" + std::to_string(am->m_accountID) + "&toUsername=" + urlEncode(myUsername);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/solicitud/aceptar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded"); request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ContactsNetwork::onAceptarResponse));
        request->setTag("aceptar");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }
    void onAceptarResponse(CCHttpClient* sender, CCHttpResponse* response) { if (response && response->isSucceed() && m_onRequestAccepted) m_onRequestAccepted(); }

    void rechazarSolicitud(const std::string& fromId) {
        auto am = GJAccountManager::sharedState();
        std::string postData = "fromId=" + urlEncode(fromId) + "&toId=" + std::to_string(am->m_accountID);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/solicitud/rechazar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded"); request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ContactsNetwork::onRechazarResponse));
        request->setTag("rechazar");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }
    void onRechazarResponse(CCHttpClient* sender, CCHttpResponse* response) { if (response && response->isSucceed() && m_onRequestAccepted) m_onRequestAccepted(); }

    void cargarContactos() {
        auto am = GJAccountManager::sharedState();
        if (am->m_accountID == 0) return;
        std::string postData = "accountId=" + std::to_string(am->m_accountID);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/contactos").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers; headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(ContactsNetwork::onContactosResponse));
        request->setTag("cargarContactos");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onContactosResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) { if (m_onContactsLoaded) m_onContactsLoaded({}); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());


        if (body == m_lastContactsJson) {
            return;
        }
        m_lastContactsJson = body;

        auto contactos = parseContactArray(body);
        if (m_onContactsLoaded) m_onContactsLoaded(contactos);
    }
};