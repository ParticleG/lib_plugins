//
// Created by Parti on 2021/2/4.
//

#include <mutex>
#include <plugins/UserManager.h>
#include <utils/misc.h>

using namespace tech::plugins;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void UserManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "UserManager loaded.";
}

void UserManager::shutdown() {
    LOG_INFO << "UserManager shutdown.";
}

void UserManager::disconnect(const int64_t &uid, UserManager::MapType mapType, const Json::Value &message) {
    auto oldConnections = unsubscribe(uid, mapType);
    for (const auto &oldConnection : oldConnections) {
        if (oldConnection) {
            websocket::close(
                    oldConnection,
                    CloseCode::kViolation,
                    tech::utils::websocket::fromJson(message)
            );
        }
    }
}

void UserManager::subscribe(const int64_t &uid, WebSocketConnectionPtr connection, MapType mapType) {
    misc::logger(typeid(*this).name(), "Try login connection");
    bool hasConflict = false;
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        switch (mapType) {
            case MapType::chat:
                if (_chatConnMap.find(uid) != _chatConnMap.end()) {
                    hasConflict = true;
                }
                break;
            case MapType::play:
                if (_playConnMap.find(uid) != _playConnMap.end()) {
                    hasConflict = true;
                }
                break;
            case MapType::stream:
                if (_streamConnMap.find(uid) != _streamConnMap.end()) {
                    hasConflict = true;
                }
                break;
            case MapType::user:
                if (_userConnMap.find(uid) != _userConnMap.end()) {
                    hasConflict = true;
                }
                break;
            case MapType::all:
                break;
        }
    }
    if (hasConflict) {
        Json::Value response;
        response["type"] = "Error";
        response["reason"] = "Account logged in at another place";
        disconnect(uid, MapType::all, response);
    }
    {
        unique_lock<shared_mutex> lock(_sharedMutex);
        switch (mapType) {
            case MapType::chat:
                _chatConnMap[uid] = move(connection);
                break;
            case MapType::play:
                _playConnMap[uid] = move(connection);
                break;
            case MapType::stream:
                _streamConnMap[uid] = move(connection);
                break;
            case MapType::user:
                _userConnMap[uid] = move(connection);
                break;
            case MapType::all:
                break;
        }
    }
    misc::logger(typeid(*this).name(), "Login: " + to_string(uid));
}

vector<WebSocketConnectionPtr> UserManager::unsubscribe(const int64_t &uid, MapType mapType) {
    misc::logger(typeid(*this).name(), "Try unsubscribing connection");
    unique_lock<shared_mutex> lock(_sharedMutex);
    vector<WebSocketConnectionPtr> oldConnections;
    switch (mapType) {
        case MapType::chat:
            oldConnections.push_back(_chatConnMap[uid] ? _chatConnMap[uid] : nullptr);
            _chatConnMap.erase(uid);
            break;
        case MapType::play:
            oldConnections.push_back(_playConnMap[uid] ? _playConnMap[uid] : nullptr);
            _playConnMap.erase(uid);
            break;
        case MapType::stream:
            oldConnections.push_back(_streamConnMap[uid] ? _streamConnMap[uid] : nullptr);
            _streamConnMap.erase(uid);
            break;
        case MapType::user:
            oldConnections.push_back(_userConnMap[uid] ? _userConnMap[uid] : nullptr);
            _userConnMap.erase(uid);
            break;
        case MapType::all:
            oldConnections.push_back(_chatConnMap[uid] ? _chatConnMap[uid] : nullptr);
            oldConnections.push_back(_playConnMap[uid] ? _playConnMap[uid] : nullptr);
            oldConnections.push_back(_streamConnMap[uid] ? _streamConnMap[uid] : nullptr);
            oldConnections.push_back(_userConnMap[uid] ? _userConnMap[uid] : nullptr);
            _chatConnMap.erase(uid);
            _playConnMap.erase(uid);
            _streamConnMap.erase(uid);
            _userConnMap.erase(uid);
            break;
    }
    return oldConnections;
}

Json::Value UserManager::parseInfo() {
    Json::Value result;
    result["chat"] = Json::arrayValue;
    result["play"] = Json::arrayValue;
    result["stream"] = Json::arrayValue;
    result["user"] = Json::arrayValue;
    for (const auto &[uid, connPtr] : _chatConnMap) {
        Json::Value tempItem;
        tempItem["uid"] = uid;
        tempItem["connected"] = connPtr->connected();
        tempItem["hasContext"] = connPtr->hasContext();
        tempItem["localAddr"] = connPtr->localAddr().toIpPort();
        tempItem["peerAddr"] = connPtr->peerAddr().toIpPort();
        result["chat"].append(tempItem);
    }
    for (const auto &[uid, connPtr] : _playConnMap) {
        Json::Value tempItem;
        tempItem["uid"] = uid;
        tempItem["connected"] = connPtr->connected();
        tempItem["hasContext"] = connPtr->hasContext();
        tempItem["localAddr"] = connPtr->localAddr().toIpPort();
        tempItem["peerAddr"] = connPtr->peerAddr().toIpPort();
        result["play"].append(tempItem);
    }
    for (const auto &[uid, connPtr] : _streamConnMap) {
        Json::Value tempItem;
        tempItem["uid"] = uid;
        tempItem["connected"] = connPtr->connected();
        tempItem["hasContext"] = connPtr->hasContext();
        tempItem["localAddr"] = connPtr->localAddr().toIpPort();
        tempItem["peerAddr"] = connPtr->peerAddr().toIpPort();
        result["stream"].append(tempItem);
    }
    for (const auto &[uid, connPtr] : _userConnMap) {
        Json::Value tempItem;
        tempItem["uid"] = uid;
        tempItem["connected"] = connPtr->connected();
        tempItem["hasContext"] = connPtr->hasContext();
        tempItem["localAddr"] = connPtr->localAddr().toIpPort();
        tempItem["peerAddr"] = connPtr->peerAddr().toIpPort();
        result["user"].append(tempItem);
    }
    return result;
}

Json::Value UserManager::getCount() {
    Json::Value result;
    misc::logger(typeid(*this).name(), "Try getting count");
    shared_lock<shared_mutex> lock(_sharedMutex);
    result["Chat"] = _chatConnMap.size();
    result["Play"] = _playConnMap.size();
    result["Stream"] = _streamConnMap.size();
    result["User"] = _userConnMap.size();
    return result;
}
