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
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto iter = _idsMap.find(id);
        if (iter != _idsMap.end()) {
            auto room = iter->second;
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
            return;
        }
    }
    unique_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(id);
    if (iter != _idsMap.end()) {
        auto room = iter->second;
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
        return;
    }
    throw out_of_range("Room not found");
}

void StreamManager::unsubscribe(const string &id, const WebSocketConnectionPtr &connection) {
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto iter = _idsMap.find(id);
        if (iter == _idsMap.end()) {
            throw out_of_range("Room not found");
        }
        auto room = iter->second;
        room->unsubscribe(connection);
        if (!iter->second->isEmpty()) {
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
    if (iter->second->isEmpty()) {
        _idsMap.erase(iter);
    }
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    shared_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(rid);
    if (iter != _idsMap.end()) {
        auto room = iter->second;

        if (action == 3) {
            connection->getContext<Stream>()->setPlace(room->generatePlace());
        }

        Json::Value response;
        response["message"] = "Broadcast";
        response["action"] = action;
        response["data"] = _parsePlayerInfo(connection, move(data));
        room->publish(move(response));
    }
    throw out_of_range("Room not found");
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data,
        const uint64_t &excluded
) {
    shared_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(rid);
    if (iter != _idsMap.end()) {
        auto room = iter->second;
        Json::Value response;
        response["message"] = "Broadcast";
        response["action"] = action;
        response["data"] = _parsePlayerInfo(connection, move(data)); // TODO: Remove unnecessary items.
        room->publish(move(response), excluded);
        if (action == 3) {
            _checkFinished(rid);
        }
    }
    throw out_of_range("Room not found");
}

Json::Value StreamManager::parseInfo() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    for (const auto &pair : _idsMap) {
        info.append(pair.second->parseInfo());
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
    shared_lock<shared_mutex> lock(_sharedMutex);
    bool allReady = true;
    auto iter = _idsMap.find(rid);
    if (iter != _idsMap.end()) {
        auto room = iter->second;
        if (!room->isFull()) {
            allReady = false;
        }
        if (allReady) {
            thread([room]() {
                this_thread::sleep_for(chrono::seconds(1));
                room->setStart(true);
                Json::Value response;
                response["message"] = "Server";
                response["action"] = 1;
                response["data"]["seed"] = Utils::uniform_random();
                room->publish(move(response));
            }).detach();
        }
        return;
    }
    throw out_of_range("Channel not found");
}

void StreamManager::_checkFinished(const string &rid) {
    shared_lock<shared_mutex> lock(_sharedMutex);
    auto iter = _idsMap.find(rid);
    if (iter != _idsMap.end()) {
        if (iter->second->checkFinished()) {
            auto room = iter->second;
            thread([rid, room]() {
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
        return;
    }
    throw out_of_range("Channel not found");
}
