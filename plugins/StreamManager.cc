//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <plugins/StreamManager.h>
#include <utils/Utils.h>

using namespace tech::plugins;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void StreamManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "Initializing StreamManager...";
    LOG_INFO << "StreamManager loaded.";
}

void StreamManager::shutdown() {
    LOG_INFO << "StreamManager shutdown.";
}

void StreamManager::subscribe(const string &id, WebSocketConnectionPtr connection) {
    auto room = getRoom(id);
    if (room->getStart()) {
        throw invalid_argument("Room already started");
    }
    room->subscribe(connection);

    Json::Value message;
    message["message"] = "Broadcast";
    message["action"] = 0;
    message["data"] = _parsePlayerInfo(connection, Json::objectValue);
    room->publish(move(message));
    _checkReady(id);
}

void StreamManager::unsubscribe(const string &id, const WebSocketConnectionPtr &connection) {
    {
        auto room = getRoom(id);
        room->unsubscribe(connection);
        if (!room->isEmpty()) {
            Json::Value message, response;
            message["message"] = "Broadcast";
            message["action"] = 5;
            message["data"] = _parsePlayerInfo(connection, Json::objectValue);
            room->publish(move(message));

            if (connection->connected()) {
                response["message"] = "OK";
                response["action"] = 5;
                connection->send(WebSocket::fromJson(response));
            }
            return;
        }
    }
    unique_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(id);
    if (iter->second.isEmpty()) {
        _idsMap.erase(iter);
    }
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    auto room = getRoom(rid);

    if (action == 3) {
        connection->getContext<Stream>()->setPlace(room->generatePlace());
    }

    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _parsePlayerInfo(connection, move(data));
    room->publish(move(response));
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data,
        const uint64_t &excluded
) {
    auto room = getRoom(rid);
    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _parsePlayerInfo(connection, move(data)); // TODO: Remove unnecessary items.
    room->publish(move(response), excluded);
    if (action == 3) {
        _checkFinished(rid);
    }
}

Json::Value StreamManager::parseInfo() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    for (const auto &pair : _idsMap) {
        info.append(pair.second.parseInfo());
    }
    return info;
}

Json::Value StreamManager::_parsePlayerInfo(
        const WebSocketConnectionPtr &connection,
        Json::Value &&data
) {
    data["uid"] = connection->getContext<Stream>()->getUid();
    return data;
}

void StreamManager::_checkReady(const std::string &rid) {
    bool allReady = true;
    auto room = getRoom(rid);
    if (!room->isFull()) {
        allReady = false;
    }
    if (allReady) {
        thread([room{move(room)}]() { // TODO: is room safe here?
            this_thread::sleep_for(chrono::seconds(1));
            room->setStart(true);
            Json::Value response;
            response["message"] = "Server";
            response["action"] = 1;
            response["data"]["seed"] = Utils::uniform_random();
            room->publish(move(response));
        }).detach();
    }
}

void StreamManager::_checkFinished(const string &rid) {
    auto room = getRoom(rid);
    if (room->checkFinished()) {
        thread([rid, room{move(room)}]() { // TODO: is room safe here?
            this_thread::sleep_for(chrono::seconds(3));
            Json::Value response, result;
            response["message"] = "Server";
            response["action"] = 4;
            room->publish(move(response));

            auto playManager = app().getPlugin<PlayManager>();
            result["start"] = false;
            result["result"] = room->getDeaths();
            playManager->publish(rid, 9, move(result));

            this_thread::sleep_for(chrono::seconds(3));
            auto streamManager = app().getPlugin<StreamManager>();
            streamManager->removeRoom(rid); // TODO: Check if is websocket friendly.
        }).detach();
    }
}
