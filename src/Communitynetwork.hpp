#pragma once
#include <Geode/Geode.hpp>
#include <cocos-ext.h>
#include <functional>
#include <vector>
#include <string>

using namespace geode::prelude;
using namespace cocos2d::extension;

struct CommunityInfo {
    std::string communityId;
    std::string name;
    std::string description;
    int icon = 1;
    int col1 = 0;
    int col2 = 3;
    int glow = 0;
    bool isPublic = true;
    std::string ownerId;
    int memberCount = 0;
};

class CommunityNetwork : public CCObject {
protected:
    std::string m_baseUrl = "https://kristal-chat-api.onrender.com";

    std::function<void(const CommunityInfo&)> m_onCommunityCreated = nullptr;
    std::function<void(const std::vector<CommunityInfo>&)> m_onCommunitiesLoaded = nullptr;
    std::function<void(const CommunityInfo&)> m_onCommunityFound = nullptr;
    std::function<void(const std::string&)> m_onError = nullptr;
    std::function<void(const std::string&)> m_logCallback = nullptr;

    void log(const std::string& msg) {
        if (m_logCallback) m_logCallback(msg);
    }

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

    CommunityInfo parseCommunityFromJson(const std::string& obj) {
        CommunityInfo info;
        info.communityId = extractJsonValue(obj, "communityId");
        info.name = extractJsonValue(obj, "name");
        info.description = extractJsonValue(obj, "description");
        info.ownerId = extractJsonValue(obj, "ownerId");

        std::string iconStr = extractJsonValue(obj, "icon");
        if (!iconStr.empty()) info.icon = std::stoi(iconStr);
        std::string col1Str = extractJsonValue(obj, "col1");
        if (!col1Str.empty()) info.col1 = std::stoi(col1Str);
        std::string col2Str = extractJsonValue(obj, "col2");
        if (!col2Str.empty()) info.col2 = std::stoi(col2Str);
        std::string glowStr = extractJsonValue(obj, "glow");
        if (!glowStr.empty()) info.glow = std::stoi(glowStr);
        std::string publicStr = extractJsonValue(obj, "isPublic");
        info.isPublic = (publicStr != "false" && publicStr != "0");
        std::string memberStr = extractJsonValue(obj, "memberCount");
        if (!memberStr.empty()) info.memberCount = std::stoi(memberStr);

        return info;
    }

    std::vector<CommunityInfo> parseCommunityArray(const std::string& json) {
        std::vector<CommunityInfo> result;
        size_t pos = 0;
        while ((pos = json.find('{', pos)) != std::string::npos) {
            size_t end = json.find('}', pos);
            if (end == std::string::npos) break;
            std::string obj = json.substr(pos, end - pos + 1);
            auto info = parseCommunityFromJson(obj);
            if (!info.communityId.empty()) result.push_back(info);
            pos = end + 1;
        }
        return result;
    }

public:
    static CommunityNetwork* create() {
        auto ret = new CommunityNetwork();
        if (ret) { ret->autorelease(); return ret; }
        return nullptr;
    }

    
    void setOnCommunityCreated(std::function<void(const CommunityInfo&)> cb) { m_onCommunityCreated = cb; }
    void setOnCommunitiesLoaded(std::function<void(const std::vector<CommunityInfo>&)> cb) { m_onCommunitiesLoaded = cb; }
    void setOnCommunityFound(std::function<void(const CommunityInfo&)> cb) { m_onCommunityFound = cb; }
    void setOnError(std::function<void(const std::string&)> cb) { m_onError = cb; }
    void setLogCallback(std::function<void(const std::string&)> cb) { m_logCallback = cb; }

    
    void crearComunidad(const std::string& ownerId, const std::string& name, const std::string& description,
        int icon, int col1, int col2, int glow, bool isPublic) {
        log("[Community] Creando: " + name);

        std::string postData = "ownerId=" + ownerId
            + "&name=" + urlEncode(name)
            + "&description=" + urlEncode(description)
            + "&icon=" + std::to_string(icon)
            + "&col1=" + std::to_string(col1)
            + "&col2=" + std::to_string(col2)
            + "&glow=" + std::to_string(glow)
            + "&isPublic=" + (isPublic ? "1" : "0");

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/crear").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        std::vector<std::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onCrearResponse));
        request->setTag("crearComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onCrearResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) {
            log("[Community] Sin respuesta del servidor");
            if (m_onError) m_onError("Sin respuesta del servidor");
            return;
        }

        long statusCode = response->getResponseCode();
        std::string body = "";
        if (response->getResponseData()) {
            std::vector<char>* data = response->getResponseData();
            body = std::string(data->begin(), data->end());
        }

        log("[Community] HTTP " + std::to_string(statusCode) + " | Body: " + body);

        if (!response->isSucceed()) {
            std::string errorMsg = "HTTP " + std::to_string(statusCode);
           
            std::string serverError = extractJsonValue(body, "error");
            if (!serverError.empty()) {
                errorMsg = serverError;
            }
            else if (statusCode == 404) {
                errorMsg = "Endpoint no encontrado. Actualiza tu servidor!";
            }
            else if (statusCode == 0) {
                errorMsg = "No se pudo conectar al servidor";
            }
            if (m_onError) m_onError(errorMsg);
            return;
        }

        auto info = parseCommunityFromJson(body);
        if (!info.communityId.empty()) {
            log("[Community] Creada: " + info.communityId);
            if (m_onCommunityCreated) m_onCommunityCreated(info);
        }
        else {
            std::string errorMsg = extractJsonValue(body, "error");
            if (errorMsg.empty()) errorMsg = "Respuesta invalida: " + body.substr(0, 100);
            if (m_onError) m_onError(errorMsg);
        }
    }

   
    void cargarComunidades(const std::string& userId) {
        log("[Community] Cargando comunidades...");

        std::string postData = "accountId=" + userId;

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidades").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        std::vector<std::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onComunidadesResponse));
        request->setTag("cargarComunidades");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onComunidadesResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            if (m_onCommunitiesLoaded) m_onCommunitiesLoaded({});
            return;
        }
        std::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto list = parseCommunityArray(body);
        log("[Community] Cargadas: " + std::to_string(list.size()));
        if (m_onCommunitiesLoaded) m_onCommunitiesLoaded(list);
    }

     
    void buscarComunidad(const std::string& communityId) {
        log("[Community] Buscando: " + communityId);

        std::string postData = "communityId=" + urlEncode(communityId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/buscar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        std::vector<std::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onBuscarResponse));
        request->setTag("buscarComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onBuscarResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            if (m_onError) m_onError("Comunidad no encontrada");
            return;
        }
        std::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto info = parseCommunityFromJson(body);
        if (!info.communityId.empty()) {
            log("[Community] Encontrada: " + info.name);
            if (m_onCommunityFound) m_onCommunityFound(info);
        }
        else {
            if (m_onError) m_onError("Comunidad no encontrada");
        }
    }

    
    void unirseComunidad(const std::string& userId, const std::string& communityId) {
        log("[Community] Uniendose a: " + communityId);

        std::string postData = "accountId=" + userId + "&communityId=" + urlEncode(communityId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/unirse").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        std::vector<std::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onUnirseResponse));
        request->setTag("unirseComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onUnirseResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            if (m_onError) m_onError("Error al unirse");
            return;
        }
        log("[Community] Unido exitosamente");
    }
};