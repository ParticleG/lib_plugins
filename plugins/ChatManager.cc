//
// Created by Parti on 2021/2/4.
//

#include <plugins/ChatManager.h>

using namespace tech::structures;
using namespace drogon;
using namespace std;

void ChatManager::initAndStart(const Json::Value &config) {
    if (config.isMember("id") && config["id"].isString()) {
        _roomID = config["id"].asString();
        _chattingRoom = make_unique<Room>(
                config["id"].asString(),
                config["name"].asString(),
                "",
                "unlimited",
                2147483647
        );
    } else {
        LOG_ERROR << R"(Requires string value "id" in plugin ChatManager's config')";
        abort();
    }
}

void ChatManager::shutdown() {
    /// Shutdown the plugin
}

string ChatManager::getID() const {
    return _roomID;
}

SubscriberID ChatManager::joinChat(const Room::MessageHandler &handler, const shared_ptr<tech::structures::Player> &player) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    SubscriberID subscriberID = _chattingRoom->subscribe(handler);
    player->subscribe(subscriberID);
    _chattingRoom->join(subscriberID, player);
    return subscriberID;
}

void ChatManager::quitChat(SubscriberID playerID) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    _chattingRoom->unsubscribe(playerID);
    _chattingRoom->quit(playerID);
}

void ChatManager::chat(const string &message) {
    _chattingRoom->publish(message);
}

uint64_t ChatManager::chatCount() {
    return _chattingRoom->count();
}
