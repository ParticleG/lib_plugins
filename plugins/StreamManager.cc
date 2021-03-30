//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <plugins/StreamManager.h>
#include <utils/misc.h>

using namespace tech::plugins;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void StreamManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "StreamManager loaded.";
}

void StreamManager::shutdown() {
    LOG_INFO << "StreamManager shutdown.";
}

void StreamManager::subscribe(const string &rid, WebSocketConnectionPtr connection) {
    auto sharedRoom = getSharedRoom(rid);
    if (sharedRoom.room.getStart()) {
        throw invalid_argument("Room already started");
    }
    sharedRoom.room.subscribe(connection);

    Json::Value message;
    message["message"] = "Broadcast";
    message["action"] = 0;
    message["data"] = _getStream(connection)->parsePlayerInfo(Json::objectValue);
    sharedRoom.room.publish(move(message));
    _checkReady(rid);
}

void StreamManager::unsubscribe(const string &rid, const WebSocketConnectionPtr &connection) {
    {
        auto sharedRoom = getSharedRoom(rid);
        sharedRoom.room.unsubscribe(connection);
        if (!sharedRoom.room.isEmpty()) {
            Json::Value message, response;
            message["message"] = "Broadcast";
            message["action"] = 5;
            message["data"] = _getStream(connection)->parsePlayerInfo(Json::objectValue);
            sharedRoom.room.publish(move(message));

            if (connection->connected()) {
                response["message"] = "OK";
                response["action"] = 5;
                connection->send(websocket::fromJson(response));
            }
            return;
        }
    }
    if (getSharedRoom(rid).room.isEmpty()) { // TODO: Check if thread safe
        removeRoom(rid);
    }
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    auto sharedRoom = getSharedRoom(rid);

    if (action == 3) {
        _getStream(connection)->setPlace(sharedRoom.room.generatePlace());
    }

    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getStream(connection)->parsePlayerInfo(move(data));
    sharedRoom.room.publish(move(response));
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data,
        const uint64_t &excluded
) {
    auto sharedRoom = getSharedRoom(rid);
    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getStream(connection)->parsePlayerInfo(move(data)); // TODO: Remove unnecessary items.
    sharedRoom.room.publish(move(response), excluded);
    if (action == 3) {
        _checkFinished(rid);
    }
}

Json::Value StreamManager::parseInfo() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    for (const auto &[id, room_with_mutex] : _idsMap) {
        shared_lock<shared_mutex> roomLock(*room_with_mutex.sharedMutex);
        info.append(room_with_mutex.room.parseInfo());
    }
    return info;
}

shared_ptr<Stream> StreamManager::_getStream(const drogon::WebSocketConnectionPtr &connection) {
    return connection->getContext<Stream>();
}

void StreamManager::_checkReady(const string &rid) {
    bool allReady = true;
    {
        auto sharedRoom = getSharedRoom(rid);
        if (!sharedRoom.room.isFull()) {
            allReady = false;
        }
    }
    if (allReady) {
        thread([this, &rid]() { // TODO: is room safe here?
            auto sharedRoom = getSharedRoom(rid);
            this_thread::sleep_for(chrono::seconds(1));
            sharedRoom.room.setStart(true);
            Json::Value response;
            response["message"] = "Server";
            response["action"] = 1;
            response["data"]["seed"] = misc::uniform_random();
            sharedRoom.room.publish(move(response));
        }).detach();
    }
}

void StreamManager::_checkFinished(const string &rid) {
    if (getSharedRoom(rid).room.checkFinished()) {
        thread([this, &rid]() { // TODO: is room safe here?
            auto sharedRoom = getSharedRoom(rid);
            this_thread::sleep_for(chrono::seconds(3));
            Json::Value response, result;
            response["message"] = "Server";
            response["action"] = 4;
            sharedRoom.room.publish(move(response));

            auto playManager = app().getPlugin<PlayManager>();
            result["start"] = false;
            result["result"] = sharedRoom.room.getDeaths();
            playManager->publish(rid, 9, move(result));

            this_thread::sleep_for(chrono::seconds(3));
            auto streamManager = app().getPlugin<StreamManager>();
            streamManager->removeRoom(rid); // TODO: Check if is websocket friendly.
        }).detach();
    }
}
