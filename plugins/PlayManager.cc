//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <plugins/StreamManager.h>
#include <strategies/actions.h>
#include <utils/crypto.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void PlayManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "PlayManager loaded.";
}

void PlayManager::shutdown() {
    LOG_INFO << "PlayManager shutdown.";
}

void PlayManager::subscribe(
        const string &rid,
        const string &password,
        const WebSocketConnectionPtr &connection
) {
    misc::logger(typeid(*this).name(),
                 "Try subscribe a player: (" + rid + ") " +
                 websocket::fromJson(_getPlay(connection)->parsePlayerInfo(Json::objectValue)));
    auto sharedRoom = getSharedRoom(rid);
    if (!sharedRoom.room.checkPassword(password)) {
        throw invalid_argument("Password is incorrect");
    }
    sharedRoom.room.subscribe(connection);
    auto play = _getPlay(connection);

    play->setReady(false);

    Json::Value data, message, response;
    data["config"] = play->getConfig();
    data["ready"] = play->getReady();

    message["type"] = "Broadcast";
    message["action"] = static_cast<int>(actions::Play::enterRoom);
    message["data"] = play->parsePlayerInfo(move(data));
    sharedRoom.room.publish(static_cast<int>(actions::Play::enterRoom), move(message), play->getSid());

    response["type"] = "Self";
    response["action"] = static_cast<int>(actions::Play::enterRoom);
    response["data"] = sharedRoom.room.parseInfo();
    response["data"]["sid"] = play->getSid();
    response["data"]["ready"] = play->getReady();
    response["data"]["roomData"] = sharedRoom.room.getData();
    response["data"]["histories"] = sharedRoom.room.getHistory(0, 10);
    response["data"]["players"] = sharedRoom.room.getPlayers();
    if (!sharedRoom.room.getRelatedStreamRid().empty()) {
        response["data"]["srid"] = sharedRoom.room.getRelatedStreamRid();
    }
    connection->send(websocket::fromJson(response));
}

void PlayManager::unsubscribe(const string &rid, const WebSocketConnectionPtr &connection) {
    misc::logger(typeid(*this).name(),
                 "Try unsubscribe a player: (" + rid + ") " +
                 websocket::fromJson(_getPlay(connection)->parsePlayerInfo(Json::objectValue)));
    {
        auto playerInfo = _getPlay(connection)->parsePlayerInfo(Json::objectValue);
        auto sharedRoom = getSharedRoom(rid);
        sharedRoom.room.unsubscribe(connection);
        if (!sharedRoom.room.isEmpty()) {
            Json::Value message;
            message["type"] = "Broadcast";
            message["action"] = static_cast<int>(actions::Play::leaveRoom);
            message["data"] = playerInfo; // TODO: Remove unnecessary items.
            sharedRoom.room.publish(static_cast<int>(actions::Play::leaveRoom), move(message));
        }
    }
    if (connection->connected()) {
        Json::Value response;
        response["type"] = "Self";
        response["action"] = static_cast<int>(actions::Play::leaveRoom);
        response["data"] = Json::objectValue;
        connection->send(websocket::fromJson(response));
    }
    if (getSharedRoom(rid).room.isEmpty()) { // TODO: Check if thread safe
        misc::logger(typeid(*this).name(), "Try remove room when last player left: " + rid);
        removeRoom(rid);
    }
}

void PlayManager::publish(
        const string &rid,
        const uint64_t &action,
        Json::Value &&data,
        const WebSocketConnectionPtr &connection,
        const uint64_t &excluded
) {
    auto sharedRoom = getSharedRoom(rid);
    Json::Value response;
    response["type"] = connection ? "Broadcast" : "Server";
    response["action"] = action;
    response["data"] = connection ? _getPlay(connection)->parsePlayerInfo(move(data)) : move(data); // TODO: Remove unnecessary items.
    misc::logger(typeid(*this).name(), "Publish: " + websocket::fromJson(response));
    sharedRoom.room.publish(action, move(response), excluded);
}

void PlayManager::changeConfig(
        const string &rid,
        string &&config,
        const WebSocketConnectionPtr &connection
) {
    misc::logger(typeid(*this).name(), "Changing config: (" + rid + ") " + config);
    auto play = _getPlay(connection);
    Json::Value data;
    data["config"] = config;
    play->setConfig(move(config));
    publish(rid, static_cast<int>(actions::Play::changeConfig), move(data), connection, play->getSid());
}

void PlayManager::changeReady(
        const string &rid,
        const bool &ready,
        const WebSocketConnectionPtr &connection
) {
    misc::logger(typeid(*this).name(), "Changing config: (" + rid + ") " + to_string(ready));
    auto play = _getPlay(connection);
    Json::Value data;
    data["ready"] = ready;
    play->setReady(ready);
    publish(rid, static_cast<int>(actions::Play::changeReady), move(data), connection);
    _checkReady(rid);
}

Json::Value PlayManager::parseInfo(
        const unsigned int &begin,
        const unsigned int &count
) const {
    misc::logger(typeid(*this).name(), "Try parsing manager info: " + to_string(count));
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    if (begin < _idsMap.size()) {
        unsigned int counter = 0;
        for (const auto &pair : _idsMap) {
            shared_lock<shared_mutex> roomLock(*pair.second.sharedMutex);
            if (counter < begin) {
                ++counter;
                continue;
            }
            if (counter >= begin + count) {
                break;
            }
            info.append(pair.second.room.parseInfo());
            ++counter;
        }
    }
    misc::logger(typeid(*this).name(), "Parsed manager info: " + websocket::fromJson(info));
    return info;
}

shared_ptr<Play> PlayManager::_getPlay(const drogon::WebSocketConnectionPtr &connection) {
    return connection->getContext<Play>();
}

void PlayManager::_checkReady(const std::string &rid) {
    {
        auto sharedRoom = getSharedRoom(rid);
        if (sharedRoom.room.getStart()) {
            return;
        }
        if (sharedRoom.room.getPendingStart()) {
            return;
        }
        if (sharedRoom.room.checkReady()) {
            sharedRoom.room.setPendingStart(true);
            Json::Value response;
            response["type"] = "Server";
            response["action"] = static_cast<int>(actions::Play::pendingStart);
            sharedRoom.room.publish(static_cast<int>(actions::Play::pendingStart), move(response));
        }
    }
    if (getSharedRoom(rid).room.checkReady()) {
        thread([this, rid]() {
            try {
                misc::logger(typeid(*this).name(), "Try Check room ready within 2 seconds: " + rid);
                {
                    auto sharedRoom = getSharedRoom(rid);
                    for (unsigned int milliseconds = 0; milliseconds < 200; ++milliseconds) {
                        this_thread::sleep_for(chrono::milliseconds(10));
                        if (!sharedRoom.room.checkReady()) {
                            sharedRoom.room.setPendingStart(false);
                            return;
                        }
                    }
                }
                // TODO: Check if Unique lock works.
                auto uniqueRoom = getUniqueRoom(rid);
                misc::logger(typeid(*this).name(), "Try set room start: " + rid);
                uniqueRoom.room.setStart(true);

                auto streamManager = app().getPlugin<StreamManager>();
                auto srid = crypto::blake2b(drogon::utils::getUuid());
                try {
                    auto streamRoom = StreamRoom(
                            rid,
                            srid,
                            uniqueRoom.room.getCount(),
                            uniqueRoom.room.getCapacity()
                    );
                    misc::logger(typeid(*this).name(), "Try create stream room: " + srid);
                    streamManager->createRoom(move(streamRoom));
                    streamManager->startCountDown(srid);
                    uniqueRoom.room.setRelatedStreamRid(srid);
                } catch (const exception &error) {
                    LOG_FATAL << error.what();
                    abort();
                }
                Json::Value response;
                response["type"] = "Server";
                response["action"] = static_cast<int>(actions::Play::startGame);
                response["data"]["rid"] = srid;
                uniqueRoom.room.publish(static_cast<int>(actions::Play::startGame), move(response));
                uniqueRoom.room.setPendingStart(false);
                misc::logger(typeid(*this).name(), "Try reset room ready: " + rid);
                uniqueRoom.room.resetReady();
            } catch (const exception &error) {
                LOG_FATAL << error.what();
                abort();
            }
        }).detach();
    }
}
