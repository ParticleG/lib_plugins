//
// Created by Parti on 2021/2/4.
//

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

void UserManager::subscribe(const int64_t &uid, WebSocketConnectionPtr connection, MapType mapType) {
    misc::logger(typeid(*this).name(), "Try login connection");
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        if (_connMapMap[mapType].find(uid) != _connMapMap[mapType].end()) {
            Json::Value response;
            response["type"] = "Error";
            response["reason"] = "Account logged in at another place";
            for (auto &connMap : _connMapMap) {
                if(connMap.second[uid]){
                    websocket::close(
                            connMap.second[uid],
                            CloseCode::kViolation,
                            tech::utils::websocket::fromJson(response)
                    );
                }
            }
        }
    }
    unique_lock<shared_mutex> lock(_sharedMutex);
    _connMapMap[mapType][uid] = move(connection);
    misc::logger(typeid(*this).name(), "Login: " + to_string(uid));
}

void UserManager::unsubscribe(const int64_t &uid, const WebSocketConnectionPtr &connection, MapType mapType) {
    misc::logger(typeid(*this).name(), "Try unsubscribing connection");
    unique_lock<shared_mutex> lock(_sharedMutex);
    if(_connMapMap[mapType][uid] == connection){
        auto node = _connMapMap[mapType].extract(uid);
        if (node.empty()) {
            LOG_INFO << "Already Logout at: " << static_cast<int>(mapType);
        }
    }
}

Json::Value UserManager::getCount() {
    Json::Value result;
    misc::logger(typeid(*this).name(), "Try getting count");
    shared_lock<shared_mutex> lock(_sharedMutex);
    result["Chat"] = _connMapMap[MapType::chat].size();
    result["Play"] = _connMapMap[MapType::play].size();
    result["Stream"] = _connMapMap[MapType::stream].size();
    result["User"] = _connMapMap[MapType::user].size();
    return result;
}
