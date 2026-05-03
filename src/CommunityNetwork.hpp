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
    bool isMember = false;
    bool hasPending = false;
};



struct MemberInfo {
    std::string accountId;
    std::string username;
    int icon = 0;
    int col1 = 0;
    int col2 = 3;
    int glow = 0;
    std::string role;
};

struct JoinRequestInfo {
    std::string accountId;
    std::string username;
    int icon = 0;
    int col1 = 0;
    int col2 = 3;
    int glow = 0;
};

struct CommunityInviteInfo {
    std::string communityId;
    std::string name;
    int icon = 1;
    int col1 = 0;
    int col2 = 3;
    int glow = 0;
};

class CommunityNetwork : public CCObject {
protected:
    std::string m_baseUrl = "https://kristal-backend-9aow.onrender.com";

    std::function<void(const CommunityInfo&)> m_onCommunityCreated = nullptr;
    std::function<void(const std::vector<CommunityInfo>&)> m_onCommunitiesLoaded = nullptr;
    std::function<void(const CommunityInfo&)> m_onCommunityFound = nullptr;
    std::function<void(const std::string&)> m_onError = nullptr;
    std::function<void(const std::string&)> m_logCallback = nullptr;
    std::function<void(const std::string&)> m_onInviteSent = nullptr;
    std::function<void(const std::vector<MemberInfo>&)> m_onMembersLoaded = nullptr;
    std::function<void(const std::string&)> m_onMemberRemoved = nullptr;
    std::function<void()> m_onCommunityDeleted = nullptr;
    std::function<void()> m_onCommunityEdited = nullptr;
    std::function<void(const std::vector<CommunityInfo>&)> m_onAllCommunitiesLoaded = nullptr;
    std::function<void(const std::string&)> m_onJoinResult = nullptr;
    std::function<void(const std::string&)> m_onJoinRequestSent = nullptr;
    std::function<void(const std::vector<JoinRequestInfo>&)> m_onJoinRequestsLoaded = nullptr;
    std::function<void(const std::string&)> m_onJoinRequestHandled = nullptr;
    std::function<void(const std::vector<CommunityInviteInfo>&)> m_onInvitesLoaded = nullptr;
    std::function<void(const std::string&)> m_onInviteHandled = nullptr;
    std::function<void(const std::string&)> m_onLeftCommunity = nullptr;

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
        std::string isMemberStr = extractJsonValue(obj, "isMember");
        info.isMember = (isMemberStr == "true" || isMemberStr == "1");
        std::string hasPendingStr = extractJsonValue(obj, "hasPending");
        info.hasPending = (hasPendingStr == "true" || hasPendingStr == "1");

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

    std::vector<MemberInfo> parseMemberArray(const std::string& json) {
        std::vector<MemberInfo> result;
        size_t pos = 0;
        while ((pos = json.find('{', pos)) != std::string::npos) {
            size_t end = json.find('}', pos);
            if (end == std::string::npos) break;
            std::string obj = json.substr(pos, end - pos + 1);

            MemberInfo m;
            m.accountId = extractJsonValue(obj, "accountId");
            m.username = extractJsonValue(obj, "username");
            m.role = extractJsonValue(obj, "role");

            std::string iconStr = extractJsonValue(obj, "icon");
            if (!iconStr.empty()) m.icon = std::stoi(iconStr);
            std::string col1Str = extractJsonValue(obj, "col1");
            if (!col1Str.empty()) m.col1 = std::stoi(col1Str);
            std::string col2Str = extractJsonValue(obj, "col2");
            if (!col2Str.empty()) m.col2 = std::stoi(col2Str);
            std::string glowStr = extractJsonValue(obj, "glow");
            if (!glowStr.empty()) m.glow = std::stoi(glowStr);

            if (!m.accountId.empty()) result.push_back(m);
            pos = end + 1;
        }
        return result;
    }

    std::vector<JoinRequestInfo> parseJoinRequestArray(const std::string& json) {
        std::vector<JoinRequestInfo> result;
        size_t pos = 0;
        while ((pos = json.find('{', pos)) != std::string::npos) {
            size_t end = json.find('}', pos);
            if (end == std::string::npos) break;
            std::string obj = json.substr(pos, end - pos + 1);
            JoinRequestInfo r;
            r.accountId = extractJsonValue(obj, "accountId");
            r.username = extractJsonValue(obj, "username");
            std::string iconStr = extractJsonValue(obj, "icon");
            if (!iconStr.empty()) r.icon = std::stoi(iconStr);
            std::string col1Str = extractJsonValue(obj, "col1");
            if (!col1Str.empty()) r.col1 = std::stoi(col1Str);
            std::string col2Str = extractJsonValue(obj, "col2");
            if (!col2Str.empty()) r.col2 = std::stoi(col2Str);
            std::string glowStr = extractJsonValue(obj, "glow");
            if (!glowStr.empty()) r.glow = std::stoi(glowStr);
            if (!r.accountId.empty()) result.push_back(r);
            pos = end + 1;
        }
        return result;
    }

    std::vector<CommunityInviteInfo> parseInviteArray(const std::string& json) {
        std::vector<CommunityInviteInfo> result;
        size_t pos = 0;
        while ((pos = json.find('{', pos)) != std::string::npos) {
            size_t end = json.find('}', pos);
            if (end == std::string::npos) break;
            std::string obj = json.substr(pos, end - pos + 1);
            CommunityInviteInfo r;
            r.communityId = extractJsonValue(obj, "communityId");
            r.name = extractJsonValue(obj, "name");

            std::string iconStr = extractJsonValue(obj, "icon");
            if (!iconStr.empty()) r.icon = std::stoi(iconStr);

            std::string col1Str = extractJsonValue(obj, "col1");
            if (!col1Str.empty()) r.col1 = std::stoi(col1Str);

            std::string col2Str = extractJsonValue(obj, "col2");
            if (!col2Str.empty()) r.col2 = std::stoi(col2Str);

            std::string glowStr = extractJsonValue(obj, "glow");
            if (!glowStr.empty()) r.glow = std::stoi(glowStr);

            if (!r.communityId.empty()) result.push_back(r);
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
    void setOnInviteSent(std::function<void(const std::string&)> cb) { m_onInviteSent = cb; }
    void setOnMembersLoaded(std::function<void(const std::vector<MemberInfo>&)> cb) { m_onMembersLoaded = cb; }
    void setOnMemberRemoved(std::function<void(const std::string&)> cb) { m_onMemberRemoved = cb; }
    void setOnCommunityDeleted(std::function<void()> cb) { m_onCommunityDeleted = cb; }
    void setOnCommunityEdited(std::function<void()> cb) { m_onCommunityEdited = cb; }
    void setOnAllCommunitiesLoaded(std::function<void(const std::vector<CommunityInfo>&)> cb) { m_onAllCommunitiesLoaded = cb; }
    void setOnJoinResult(std::function<void(const std::string&)> cb) { m_onJoinResult = cb; }
    void setOnJoinRequestSent(std::function<void(const std::string&)> cb) { m_onJoinRequestSent = cb; }
    void setOnJoinRequestsLoaded(std::function<void(const std::vector<JoinRequestInfo>&)> cb) { m_onJoinRequestsLoaded = cb; }
    void setOnJoinRequestHandled(std::function<void(const std::string&)> cb) { m_onJoinRequestHandled = cb; }
    void setOnInvitesLoaded(std::function<void(const std::vector<CommunityInviteInfo>&)> cb) { m_onInvitesLoaded = cb; }
    void setOnInviteHandled(std::function<void(const std::string&)> cb) { m_onInviteHandled = cb; }
    void setOnLeftCommunity(std::function<void(const std::string&)> cb) { m_onLeftCommunity = cb; }

    void crearComunidad(const std::string& ownerId, const std::string& name, const std::string& description,
        int icon, int col1, int col2, int glow, bool isPublic) {
        log("[Community] Creating: " + name);

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
        gd::vector<gd::string> headers;
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
            log("[Community] No response from server");
            if (m_onError) m_onError("No response from server");
            return;
        }

        long statusCode = response->getResponseCode();
        std::string body = "";
        if (response->getResponseData()) {
            gd::vector<char>* data = response->getResponseData();
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
                errorMsg = "Endpoint not found. Update your server!";
            }
            else if (statusCode == 0) {
                errorMsg = "Could not connect to server";
            }
            if (m_onError) m_onError(errorMsg);
            return;
        }

        auto info = parseCommunityFromJson(body);
        if (!info.communityId.empty()) {
            log("[Community] Created: " + info.communityId);
            if (m_onCommunityCreated) m_onCommunityCreated(info);
        }
        else {
            std::string errorMsg = extractJsonValue(body, "error");
            if (errorMsg.empty()) errorMsg = "Invalid response: " + body.substr(0, 100);
            if (m_onError) m_onError(errorMsg);
        }
    }

    void cargarComunidades(const std::string& userId) {
        log("[Community] Loading communities...");

        std::string postData = "accountId=" + userId;

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidades").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
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
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto list = parseCommunityArray(body);
        log("[Community] Loaded: " + std::to_string(list.size()));
        if (m_onCommunitiesLoaded) m_onCommunitiesLoaded(list);
    }

    void buscarComunidad(const std::string& communityId) {
        log("[Community] Searching: " + communityId);

        std::string postData = "communityId=" + urlEncode(communityId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/buscar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
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
            if (m_onError) m_onError("Community not found");
            return;
        }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto info = parseCommunityFromJson(body);
        if (!info.communityId.empty()) {
            log("[Community] Found: " + info.name);
            if (m_onCommunityFound) m_onCommunityFound(info);
        }
        else {
            if (m_onError) m_onError("Community not found");
        }
    }

    void unirseComunidad(const std::string& userId, const std::string& communityId) {
        log("[Community] Joining: " + communityId);

        std::string postData = "accountId=" + userId + "&communityId=" + urlEncode(communityId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/unirse").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onUnirseResponse));
        request->setTag("unirseComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onUnirseResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) { if (m_onError) m_onError("No response"); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        if (response->isSucceed()) {
            std::string msg = extractJsonValue(body, "message");
            if (msg.empty()) msg = "You joined!";
            if (m_onJoinResult) m_onJoinResult(msg);
        }
        else {
            std::string err = extractJsonValue(body, "error");
            if (err.empty()) err = "Error joining";
            if (m_onError) m_onError(err);
        }
    }

    void invitarMiembroComunidad(const std::string& ownerId, const std::string& communityId, const std::string& targetAccountId) {
        log("[Community] Inviting member " + targetAccountId + " to community: " + communityId);

        std::string postData = "ownerId=" + ownerId
            + "&communityId=" + urlEncode(communityId)
            + "&targetAccountId=" + urlEncode(targetAccountId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/invitar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onInvitarMiembroResponse));
        request->setTag("invitarMiembroComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onInvitarMiembroResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            if (m_onError) m_onError("Connection error sending invite");
            return;
        }

        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        std::string success = extractJsonValue(body, "success");

        if (success == "true") {
            log("[Community] Invite sent correctly");
            if (m_onInviteSent) m_onInviteSent("Invite sent successfully!");
        }
        else {
            std::string errorMsg = extractJsonValue(body, "error");
            if (errorMsg.empty()) errorMsg = "Invalid server response";
            if (m_onError) m_onError(errorMsg);
        }
    }

    void listarMiembros(const std::string& communityId) {
        log("[Community] Listing members of: " + communityId);
        std::string postData = "communityId=" + urlEncode(communityId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/miembros").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onMiembrosResponse));
        request->setTag("listarMiembros");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onMiembrosResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            if (m_onMembersLoaded) m_onMembersLoaded({});
            return;
        }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto members = parseMemberArray(body);
        log("[Community] Members loaded: " + std::to_string(members.size()));
        if (m_onMembersLoaded) m_onMembersLoaded(members);
    }

    void expulsarMiembro(const std::string& ownerId, const std::string& communityId, const std::string& targetAccountId) {
        log("[Community] Kicking " + targetAccountId + " from: " + communityId);
        std::string postData = "ownerId=" + ownerId
            + "&communityId=" + urlEncode(communityId)
            + "&targetAccountId=" + urlEncode(targetAccountId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/expulsar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onExpulsarResponse));
        request->setTag("expulsarMiembro");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onExpulsarResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            std::string errMsg = "Connection error";
            if (response && response->getResponseData()) {
                std::string body(response->getResponseData()->begin(), response->getResponseData()->end());
                std::string serverErr = extractJsonValue(body, "error");
                if (!serverErr.empty()) errMsg = serverErr;
            }
            if (m_onError) m_onError(errMsg);
            return;
        }
        log("[Community] Member kicked");
        if (m_onMemberRemoved) m_onMemberRemoved("Member kicked successfully!");
    }

    void eliminarComunidad(const std::string& ownerId, const std::string& communityId) {
        log("[Community] Deleting community: " + communityId);
        std::string postData = "ownerId=" + ownerId + "&communityId=" + urlEncode(communityId);

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/eliminar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onEliminarResponse));
        request->setTag("eliminarComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onEliminarResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            std::string errMsg = "Connection error";
            if (response && response->getResponseData()) {
                std::string body(response->getResponseData()->begin(), response->getResponseData()->end());
                std::string serverErr = extractJsonValue(body, "error");
                if (!serverErr.empty()) errMsg = serverErr;
            }
            if (m_onError) m_onError(errMsg);
            return;
        }
        log("[Community] Community deleted");
        if (m_onCommunityDeleted) m_onCommunityDeleted();
    }

    void editarComunidad(const std::string& ownerId, const std::string& communityId,
        const std::string& name, const std::string& description,
        int icon, int col1, int col2, int glow, bool isPublic) {
        log("[Community] Editing community: " + communityId);

        std::string postData = "ownerId=" + ownerId
            + "&communityId=" + urlEncode(communityId)
            + "&name=" + urlEncode(name)
            + "&description=" + urlEncode(description)
            + "&icon=" + std::to_string(icon)
            + "&col1=" + std::to_string(col1)
            + "&col2=" + std::to_string(col2)
            + "&glow=" + std::to_string(glow)
            + "&isPublic=" + (isPublic ? "1" : "0");

        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/editar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onEditarResponse));
        request->setTag("editarComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onEditarResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) {
            std::string errMsg = "Connection error";
            if (response && response->getResponseData()) {
                std::string body(response->getResponseData()->begin(), response->getResponseData()->end());
                std::string serverErr = extractJsonValue(body, "error");
                if (!serverErr.empty()) errMsg = serverErr;
            }
            if (m_onError) m_onError(errMsg);
            return;
        }
        log("[Community] Community edited");
        if (m_onCommunityEdited) m_onCommunityEdited();
    }

    void cargarTodasComunidades(const std::string& accountId) {
        std::string postData = "accountId=" + accountId;
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/todas").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onTodasResponse));
        request->setTag("todasComunidades");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onTodasResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) { if (m_onAllCommunitiesLoaded) m_onAllCommunitiesLoaded({}); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto list = parseCommunityArray(body);
        if (m_onAllCommunitiesLoaded) m_onAllCommunitiesLoaded(list);
    }

    void enviarSolicitudUnion(const std::string& accountId, const std::string& communityId) {
        std::string postData = "accountId=" + accountId + "&communityId=" + urlEncode(communityId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/solicitud/enviar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onSolicitudUnionResponse));
        request->setTag("solicitudUnion");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onSolicitudUnionResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) { if (m_onError) m_onError("No response"); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        if (response->isSucceed()) {
            std::string msg = extractJsonValue(body, "message");
            if (msg.empty()) msg = "Request sent!";
            if (m_onJoinRequestSent) m_onJoinRequestSent(msg);
        }
        else {
            std::string err = extractJsonValue(body, "error");
            if (err.empty()) err = "Error sending request";
            if (m_onError) m_onError(err);
        }
    }

    void cargarSolicitudesUnion(const std::string& ownerId, const std::string& communityId) {
        std::string postData = "ownerId=" + ownerId + "&communityId=" + urlEncode(communityId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/solicitudes").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onSolicitudesUnionResponse));
        request->setTag("solicitudesUnion");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onSolicitudesUnionResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) { if (m_onJoinRequestsLoaded) m_onJoinRequestsLoaded({}); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto list = parseJoinRequestArray(body);
        if (m_onJoinRequestsLoaded) m_onJoinRequestsLoaded(list);
    }

    void aceptarSolicitudUnion(const std::string& ownerId, const std::string& communityId, const std::string& targetAccountId) {
        std::string postData = "ownerId=" + ownerId + "&communityId=" + urlEncode(communityId) + "&targetAccountId=" + urlEncode(targetAccountId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/solicitud/aceptar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onHandleSolicitudResponse));
        request->setTag("aceptarSolicitudUnion");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void rechazarSolicitudUnion(const std::string& ownerId, const std::string& communityId, const std::string& targetAccountId) {
        std::string postData = "ownerId=" + ownerId + "&communityId=" + urlEncode(communityId) + "&targetAccountId=" + urlEncode(targetAccountId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/solicitud/rechazar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onHandleSolicitudResponse));
        request->setTag("rechazarSolicitudUnion");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onHandleSolicitudResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) { if (m_onError) m_onError("No response"); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        if (response->isSucceed()) {
            std::string msg = extractJsonValue(body, "message");
            if (msg.empty()) msg = "Done!";
            if (m_onJoinRequestHandled) m_onJoinRequestHandled(msg);
        }
        else {
            std::string err = extractJsonValue(body, "error");
            if (err.empty()) err = "Error";
            if (m_onError) m_onError(err);
        }
    }

    void cargarInvitaciones(const std::string& accountId) {
        std::string postData = "accountId=" + accountId;
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/invitaciones").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onInvitacionesResponse));
        request->setTag("cargarInvitaciones");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onInvitacionesResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response || !response->isSucceed()) { if (m_onInvitesLoaded) m_onInvitesLoaded({}); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        auto list = parseInviteArray(body);
        if (m_onInvitesLoaded) m_onInvitesLoaded(list);
    }

    void aceptarInvitacion(const std::string& accountId, const std::string& communityId) {
        std::string postData = "accountId=" + accountId + "&communityId=" + urlEncode(communityId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/invitacion/aceptar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onHandleInviteResponse));
        request->setTag("aceptarInvitacion");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void rechazarInvitacion(const std::string& accountId, const std::string& communityId) {
        std::string postData = "accountId=" + accountId + "&communityId=" + urlEncode(communityId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/invitacion/rechazar").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onHandleInviteResponse));
        request->setTag("rechazarInvitacion");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onHandleInviteResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) { if (m_onError) m_onError("No response"); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        if (response->isSucceed()) {
            std::string msg = extractJsonValue(body, "message");
            if (msg.empty()) msg = "Done!";
            if (m_onInviteHandled) m_onInviteHandled(msg);
        }
        else {
            std::string err = extractJsonValue(body, "error");
            if (err.empty()) err = "Error";
            if (m_onError) m_onError(err);
        }
    }

    void salirComunidad(const std::string& accountId, const std::string& communityId) {
        std::string postData = "accountId=" + accountId + "&communityId=" + urlEncode(communityId);
        auto request = new CCHttpRequest();
        request->setUrl((m_baseUrl + "/comunidad/salir").c_str());
        request->setRequestType(CCHttpRequest::kHttpPost);
        gd::vector<gd::string> headers;
        headers.push_back("Content-Type: application/x-www-form-urlencoded");
        request->setHeaders(headers);
        request->setRequestData(postData.c_str(), postData.length());
        request->setResponseCallback(this, httpresponse_selector(CommunityNetwork::onSalirResponse));
        request->setTag("salirComunidad");
        CCHttpClient::getInstance()->send(request);
        request->release();
    }

    void onSalirResponse(CCHttpClient* sender, CCHttpResponse* response) {
        if (!response) { if (m_onError) m_onError("No response"); return; }
        gd::vector<char>* data = response->getResponseData();
        std::string body(data->begin(), data->end());
        if (response->isSucceed()) {
            std::string msg = extractJsonValue(body, "message");
            if (msg.empty()) msg = "You left the community!";
            if (m_onLeftCommunity) m_onLeftCommunity(msg);
        }
        else {
            std::string err = extractJsonValue(body, "error");
            if (err.empty()) err = "Error leaving community";
            if (m_onError) m_onError(err);
        }
    }
};