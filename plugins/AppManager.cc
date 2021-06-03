//
// Created by Parti on 2021/2/4.
//

#include <mutex>
#include <plugins/AppManager.h>
#include <utils/misc.h>

using namespace tech::plugins;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void AppManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "AppManager loaded.";
}

void AppManager::shutdown() {
    LOG_INFO << "AppManager shutdown.";
}

void AppManager::publish(const Json::Value &message) {
    for (const auto &appConnection : _appConnSet) {
        if (appConnection) {
            appConnection->send(websocket::fromJson(message));
        }
    }
}

void AppManager::subscribe(WebSocketConnectionPtr connection) {
    misc::logger(typeid(*this).name(), "Try login connection");
    unique_lock<shared_mutex> lock(_sharedMutex);
    auto matchedConn = _appConnSet.find(connection);
    if (matchedConn != _appConnSet.end()) {
        Json::Value response;
        response["type"] = "Error";
        response["reason"] = "Account logged in at another place";
        tech::utils::websocket::close(*matchedConn, CloseCode::kViolation, tech::utils::websocket::fromJson(response));
        _appConnSet.erase(matchedConn);
    }
    _appConnSet.insert(move(connection));
}

void AppManager::unsubscribe(const drogon::WebSocketConnectionPtr &connection) {
    misc::logger(typeid(*this).name(), "Try unsubscribing connection");
    unique_lock<shared_mutex> lock(_sharedMutex);
    auto matchedConn = _appConnSet.find(connection);
    if (matchedConn != _appConnSet.end()) {
        _appConnSet.erase(matchedConn);
    }
}

Json::Value AppManager::parseInfo() {
    Json::Value result(Json::arrayValue);
    for (const auto &connPtr : _appConnSet) {
        Json::Value tempItem;
        tempItem["connected"] = connPtr->connected();
        tempItem["hasContext"] = connPtr->hasContext();
        tempItem["localAddr"] = connPtr->localAddr().toIpPort();
        tempItem["peerAddr"] = connPtr->peerAddr().toIpPort();
        result.append(tempItem);
    }
    return result;
}

Json::Value AppManager::getCount() {
    Json::Value result;
    misc::logger(typeid(*this).name(), "Try getting count");
    shared_lock<shared_mutex> lock(_sharedMutex);
    result["App"] = _appConnSet.size();
    return result;
}
