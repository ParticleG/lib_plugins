//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <plugins/StreamManager.h>
#include <strategies/actions.h>

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

void StreamManager::subscribe(const string &srid, WebSocketConnectionPtr connection) {
    auto sharedRoom = getSharedRoom(srid);
    auto stream = _getStream(connection);
    if (sharedRoom.room.getStart() || !sharedRoom.room.checkIfPlaying(stream->getUid())) {
        stream->setSpectate(true);
    }
    Json::Value broadcast, self;
    self["type"] = "Self";
    self["action"] = static_cast<int>(actions::Stream::enterRoom);
    self["data"] = stream->parsePlayerInfo(stream->getSpectate() ? sharedRoom.room.parseHistories() : Json::Value());
    self["data"]["start"] = sharedRoom.room.getStart();
    self["data"]["seed"] = sharedRoom.room.getSeed();
    self["data"]["connected"] = sharedRoom.room.getPlayers();
    connection->send(websocket::fromJson(self));

    sharedRoom.room.subscribe(move(connection));
    broadcast["type"] = "Broadcast";
    broadcast["action"] = static_cast<int>(actions::Stream::enterRoom);
    broadcast["data"] = stream->parsePlayerInfo(Json::objectValue);
    sharedRoom.room.publish(move(broadcast), stream->getSid());
    _checkReady(srid);
}

void StreamManager::unsubscribe(const string &srid, const WebSocketConnectionPtr &connection) {
    {
        auto stream = _getStream(connection);
        auto sharedRoom = getSharedRoom(srid);
        sharedRoom.room.unsubscribe(connection);
        if (!sharedRoom.room.isEmpty()) {
            _getStream(connection)->setPlace(sharedRoom.room.generatePlace());

            Json::Value message;
            message["type"] = "Broadcast";
            message["action"] = static_cast<int>(actions::Stream::leaveRoom);
            message["data"] = stream->parsePlayerInfo(Json::objectValue);
            sharedRoom.room.publish(move(message));
        }
        if (!stream->getSpectate()) {
            _checkFinished(srid);
        }
    }
    if (connection->connected()) {
        Json::Value response;
        response["type"] = "Self";
        response["action"] = static_cast<int>(actions::Stream::leaveRoom);
        response["data"] = Json::objectValue;
        connection->send(websocket::fromJson(response));
    }
}

void StreamManager::startCountDown(const string &rid) {
    thread([this, rid]() {
        try {   // TODO: Use more accurate try catch
            this_thread::sleep_for(chrono::seconds(10));
            bool noConnections = false;
            {
                auto sharedRoom = getSharedRoom(rid);
                if (!sharedRoom.room.hasConnection()) {
                    try {
                        Json::Value result;
                        auto playManager = app().getPlugin<PlayManager>();
                        result["start"] = false;
                        result["result"] = sharedRoom.room.getDeaths();
                        playManager->publish(sharedRoom.room.getPlayRid(), static_cast<int>(actions::Play::endGame),
                                             move(result));
                        auto sharedPlayRoom = playManager->getSharedRoom(sharedRoom.room.getPlayRid());
                        sharedPlayRoom.room.setRelatedStreamRid("");
                        sharedPlayRoom.room.setStart(false);
                    } catch (const exception &error) {
                        LOG_WARN << error.what();
                    }
                    noConnections = true;
                } else if (!sharedRoom.room.getStart()) {
                    sharedRoom.room.setStart(true);
                    this_thread::sleep_for(chrono::seconds(1));
                    Json::Value response;
                    response["type"] = "Server";
                    response["action"] = static_cast<int>(actions::Stream::startStreaming);
                    sharedRoom.room.publish(move(response));
                }
            }
            if (noConnections) {
                removeRoom(rid);
            }
        } catch (const exception &error) {
            LOG_FATAL << error.what();
        }
    }).detach();
}

void StreamManager::publish(
        const string &rid,
        const uint64_t &action,
        Json::Value &&data,
        const WebSocketConnectionPtr &connection,
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
    for (const auto &pair : _idsMap) {
        shared_lock<shared_mutex> roomLock(*pair.second.sharedMutex);
        info.append(pair.second.room.parseInfo());
    }
    return info;
}

shared_ptr<Stream> StreamManager::_getStream(const drogon::WebSocketConnectionPtr &connection) {
    return connection->getContext<Stream>();
}

void StreamManager::_checkReady(const string &rid) {
    if (getSharedRoom(rid).room.getStart()) {
        return;
    }
    if (getSharedRoom(rid).room.checkReady()) {
        thread([this, rid]() {
            try {
                auto sharedRoom = getSharedRoom(rid);
                sharedRoom.room.setStart(true);
                this_thread::sleep_for(chrono::seconds(1));
                Json::Value response;
                response["type"] = "Server";
                response["action"] = static_cast<int>(actions::Stream::startStreaming);
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
                            sharedPlayRoom.room.setRelatedStreamRid("");
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
