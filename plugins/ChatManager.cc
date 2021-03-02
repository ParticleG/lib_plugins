//
// Created by Parti on 2021/2/4.
//

#include <plugins/ChatManager.h>
#include <structures/ChatRoom.h>
#include <utils/Utils.h>

using namespace tech::plugins;
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
                createRoom(
                        channel["id"].asString(),
                        move(ChatRoom(
                                channel["id"].asString(),
                                channel["capacity"].asUInt64(),
                                channel["historyCount"].asUInt64()))
                );
            } else {
                LOG_ERROR << R"(Requires string value "id" and UInt64 type "capacity" in config['channels'])";
                abort();
            }
        }
    } else {
        LOG_ERROR << R"(Requires array value "channels" in plugin ChatManager's config')";
        abort();
    }
}

void ChatManager::shutdown() {
    /// Shutdown the plugin
}

void ChatManager::subscribe(const string &id, WebSocketConnectionPtr connection) {
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto iter = _idsMap.find(id);
        if (iter != _idsMap.end()) {
            try {
                auto room = iter->second;
                room->subscribe(connection);

                Json::Value message, response;
                message["message"] = "Broadcast";
                message["action"] = 0;
                message["rid"] = id;
                message["data"] = _getPlayerInfo(connection);
                room->publish(move(message));

                response["message"] = "OK";
                response["action"] = 0;
                response["rid"] = id;
                response["data"] = room->getHistory(0, 20);
                connection->send(WebSocket::fromJson(response));
            } catch (range_error &error) {
                LOG_WARN << error.what();
                throw error;
            }
        }
    }
    unique_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(id);
    if (iter != _idsMap.end()) {
        try {
            auto room = iter->second;
            room->subscribe(connection);

            Json::Value message, response;
            message["message"] = "Broadcast";
            message["action"] = 0;
            message["rid"] = id;
            message["data"] = _getPlayerInfo(connection);
            room->publish(move(message));

            response["message"] = "OK";
            response["action"] = 0;
            response["rid"] = id;
            response["data"] = room->getHistory(0, 20);
            connection->send(WebSocket::fromJson(response));
        } catch (range_error &error) {
            LOG_WARN << error.what();
            throw error;
        }
    }
    throw out_of_range("Room not found");
}

void ChatManager::unsubscribe(const string &id, const WebSocketConnectionPtr &connection) {
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto iter = _idsMap.find(id);
        if (iter == _idsMap.end()) {
            throw out_of_range("Room not found");
        }
        auto room = iter->second;
        room->unsubscribe(connection);
        
        Json::Value message, response;
        message["message"] = "Broadcast";
        message["action"] = 1;
        message["rid"] = id;
        message["data"] = _getPlayerInfo(connection);
        room->publish(move(message));

        response["message"] = "OK";
        response["action"] = 0;
        response["rid"] = id;
        connection->send(WebSocket::fromJson(response));
        if (!iter->second->isEmpty())
            return;
    }
    unique_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(id);
    if (iter == _idsMap.end()) {
        throw out_of_range("Room not found");
    }
    if (iter->second->isEmpty())
        _idsMap.erase(iter);
}

void ChatManager::publish(const string &rid, const WebSocketConnectionPtr &connection, const string &message) {
    shared_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(rid);
    if (iter != _idsMap.end()) {
        auto room = iter->second;
        Json::Value response;
        response["message"] = "Broadcast";
        response["action"] = 2;
        response["rid"] = rid;
        response["data"] = _getPlayerInfo(connection, message);
        room->publish(message);
    }
    throw out_of_range("Channel not found");
}

Json::Value ChatManager::_getPlayerInfo(const WebSocketConnectionPtr &connection, const string &message) {
    auto info = connection->getContext<Chat>()->getInfo();
    Json::Value result;
    result["uid"] = info->getValueOfId();
    result["username"] = info->getValueOfId();
    result["time"] = Utils::fromDate();
    result["message"] = message;
    return result;
}

Json::Value ChatManager::_getPlayerInfo(const WebSocketConnectionPtr &connection) {
    auto info = connection->getContext<Chat>()->getInfo();
    Json::Value result;
    result["uid"] = info->getValueOfId();
    result["username"] = info->getValueOfId();
    return result;
}

