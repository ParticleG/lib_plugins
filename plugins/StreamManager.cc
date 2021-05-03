//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <plugins/StreamManager.h>
#include <strategies/actions.h>
#include <utils/misc.h>

using namespace tech::plugins;
using namespace tech::strategies;
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
    auto stream = _getStream(connection);
    if (sharedRoom.room.getStart()) {
        stream->setWatch(true);
    }
    sharedRoom.room.subscribe(connection);

    Json::Value message;
    message["type"] = "Broadcast";
    message["action"] = static_cast<int>(actions::Stream::enterRoom);
    message["data"] = stream->parsePlayerInfo(Json::objectValue);
    sharedRoom.room.publish(move(message), stream->getSid());
    _checkReady(rid);
}

void StreamManager::unsubscribe(const string &rid, const WebSocketConnectionPtr &connection) {
    {
        auto playerInfo = _getStream(connection)->parsePlayerInfo(Json::objectValue);
        auto sharedRoom = getSharedRoom(rid);
        sharedRoom.room.unsubscribe(connection);
        if (!sharedRoom.room.isEmpty()) {
            _getStream(connection)->setPlace(sharedRoom.room.generatePlace());

            Json::Value message;
            message["type"] = "Broadcast";
            message["action"] = static_cast<int>(actions::Stream::leaveRoom);
            message["data"] = playerInfo;
            sharedRoom.room.publish(move(message));
        }
        _checkFinished(rid);    // TODO: Ensure this is working properly.
    }
    if (connection->connected()) {
        Json::Value response;
        response["type"] = "Self";
        response["action"] = static_cast<int>(actions::Stream::leaveRoom);
        response["data"] = Json::objectValue;
        connection->send(websocket::fromJson(response));
    }
}

void StreamManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    auto sharedRoom = getSharedRoom(rid);
    if (action == static_cast<int>(actions::Stream::publishDeathData)) {
        _getStream(connection)->setPlace(sharedRoom.room.generatePlace());
        _checkFinished(rid);
    }
    Json::Value response;
    response["type"] = "Broadcast";
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
    if (action == static_cast<int>(actions::Stream::publishDeathData)) {
        _getStream(connection)->setPlace(sharedRoom.room.generatePlace());
        _checkFinished(rid);
    }
    Json::Value response;
    response["type"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getStream(connection)->parsePlayerInfo(move(data)); // TODO: Remove unnecessary items.
    sharedRoom.room.publish(move(response), excluded);
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
    bool allReady;
    {
        auto sharedRoom = getSharedRoom(rid);
        allReady = sharedRoom.room.checkReady();
    }
    if (allReady) {
        thread([this, rid]() {
            try {
                auto sharedRoom = getSharedRoom(rid);
                sharedRoom.room.setStart(true);
                this_thread::sleep_for(chrono::seconds(1));
                Json::Value response;
                response["type"] = "Server";
                response["action"] = static_cast<int>(actions::Stream::startStreaming);
                response["data"]["seed"] = misc::uniform_random();
                sharedRoom.room.publish(move(response));
            } catch (const exception &error) {
                LOG_FATAL << error.what();
                abort();
            }
        }).detach();
    }
}

void StreamManager::_checkFinished(const string &rid) {
    try {
        auto sharedRoom = getSharedRoom(rid);
        if (!sharedRoom.room.getFinish() && sharedRoom.room.checkFinished()) {
            sharedRoom.room.setFinish(true);
            thread([this, rid]() {
                try {
                    {
                        auto sharedRoom = getSharedRoom(rid);
                        this_thread::sleep_for(chrono::seconds(3));
                        Json::Value response, result;
                        response["type"] = "Server";
                        response["action"] = static_cast<int>(actions::Stream::endStreaming);
                        sharedRoom.room.publish(move(response));

                        try {
                            auto playManager = app().getPlugin<PlayManager>();
                            result["start"] = false;
                            result["result"] = sharedRoom.room.getDeaths();
                            playManager->publish(sharedRoom.room.getPlayRid(), static_cast<int>(actions::Play::endGame),
                                                 move(result));
                            auto sharedPlayRoom = playManager->getSharedRoom(sharedRoom.room.getPlayRid());
                            sharedPlayRoom.room.setStart(false);
                        } catch (const exception &error) {
                            LOG_WARN << error.what();
                        }
                    }
                    this_thread::sleep_for(chrono::seconds(3));
                    removeRoom(rid);
                } catch (const exception &error) {
                    LOG_FATAL << error.what();
                }
            }).detach();
        }
    } catch (const exception &error) {
        LOG_WARN << error.what();
    }
}
