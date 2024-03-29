//
// Created by Parti on 2021/2/4.
//

#include <plugins/ChatManager.h>
#include <strategies/actions.h>
#include <utils/misc.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void ChatManager::initAndStart(const Json::Value &config) {
    if (config.isMember("channels") && config["channels"].isArray()) {
        for (auto &channel : config["channels"]) {
            if (
                    channel.isMember("id") && channel["id"].isString() &&
                    channel.isMember("capacity") && channel["capacity"].isUInt64() &&
                    channel.isMember("historyCount") && channel["historyCount"].isUInt64()
                    ) {
                try {
                    createRoom(move(ChatRoom(
                            channel["id"].asString(),
                            channel["capacity"].asUInt64(),
                            channel["historyCount"].asUInt())));
                } catch (const exception &error) {
                    LOG_ERROR << error.what();
                    abort();
                }
            } else {
                LOG_ERROR << R"(Requires string value "id", UInt64 type "capacity", UInt64 type "historyCount" in config['channels'])";
                abort();
            }
        }
    } else {
        LOG_ERROR << R"(Requires array value "channels" in plugin ChatManager's config')";
        abort();
    }
    LOG_INFO << "ChatManager loaded.";
}

void ChatManager::shutdown() {
    LOG_INFO << "ChatManager shutdown.";
}

void ChatManager::subscribe(const string &id, WebSocketConnectionPtr connection) {
    auto sharedRoom = getSharedRoom(id);
    sharedRoom.room.subscribe(connection);

    Json::Value message, response;
    message["type"] = "Broadcast";
    message["action"] = static_cast<int>(actions::Chat::enterChannel);
    message["rid"] = id;
    message["data"] = _getChat(connection)->getPlayerInfo();
    sharedRoom.room.publish(move(message));

    response["type"] = "Self";
    response["action"] = static_cast<int>(actions::Chat::enterChannel);
    response["rid"] = id;
    response["data"] = sharedRoom.room.getHistory(0, 20);
    connection->send(websocket::fromJson(response));
}

void ChatManager::unsubscribe(const string &id, const WebSocketConnectionPtr &connection) {
    auto playerInfo = _getChat(connection)->getPlayerInfo();
    auto sharedRoom = getSharedRoom(id);
    sharedRoom.room.unsubscribe(connection);

    Json::Value message, response;
    message["type"] = "Broadcast";
    message["action"] = static_cast<int>(actions::Chat::leaveChannel);
    message["rid"] = id;
    message["data"] = playerInfo;
    sharedRoom.room.publish(move(message));

    if (connection->connected()) {
        response["type"] = "Self";
        response["action"] = static_cast<int>(actions::Chat::leaveChannel);
        response["rid"] = id;
        connection->send(websocket::fromJson(response));
    }
}

void ChatManager::publish(const string &rid, const WebSocketConnectionPtr &connection, const string &message) {
    Json::Value response;
    response["type"] = "Broadcast";
    response["action"] = static_cast<int>(actions::Chat::publishChatMessage);
    response["rid"] = rid;
    response["data"]["histories"] = _getChat(connection)->getPlayerInfo(message);
    getSharedRoom(rid).room.publish(move(response));
}

Json::Value ChatManager::parseInfo() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    for (const auto &[id, room_with_mutex] : _idsMap) {
        shared_lock<shared_mutex> roomLock(*room_with_mutex.sharedMutex);
        info.append(room_with_mutex.room.parseInfo());
    }
    return info;
}

shared_ptr<Chat> ChatManager::_getChat(const drogon::WebSocketConnectionPtr &connection) {
    return connection->getContext<Chat>();
}
