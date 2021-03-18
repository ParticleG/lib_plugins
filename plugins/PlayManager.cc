//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <utils/crypto.h>
#include <utils/misc.h>

using namespace tech::plugins;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

void PlayManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "Initializing PlayManager...";
    if (config.isMember("typeList") && config["typeList"].isArray()) {
        for (auto &type : config["typeList"]) {
            if (
                    type.isMember("type") && type["type"].isString() &&
                    type.isMember("capacity") && type["capacity"].isUInt64()
                    ) {
                _typesMap[type["type"].asString()] = type["capacity"].asUInt64();
            } else {
                LOG_ERROR << R"(Requires string value "type" and UInt64 value "capacity" in config['typeList'])";
                abort();
            }
        }
    } else {
        LOG_ERROR << R"(Requires array value "channels" in plugin ChatManager's config')";
        abort();
    }
    LOG_INFO << "PlayManager loaded.";
}

void PlayManager::shutdown() {
    LOG_INFO << "PlayManager shutdown.";
}

uint64_t PlayManager::getCapacity(const string &type) const {
    auto iter = _typesMap.find(type);
    if (iter != _typesMap.end()) {
        return iter->second;
    }
    throw out_of_range("Type not found");
}

void PlayManager::subscribe(
        const string &rid,
        const string &password,
        const WebSocketConnectionPtr &connection
) {
    auto &sharedRoom = getSharedRoom(rid);
    if (sharedRoom.room.getStart()) {
        throw invalid_argument("Room already started");
    }
    if (!sharedRoom.room.checkPassword(password)) {
        throw invalid_argument("Password is incorrect");
    }
    sharedRoom.room.subscribe(connection);
    auto play = _getPlay(connection);

    play->setReady(false);

    Json::Value data, message, response;
    data["config"] = play->getConfig();
    data["ready"] = play->getReady();

    message["message"] = "Broadcast";
    message["action"] = 2;
    message["data"] = play->parsePlayerInfo(move(data));
    sharedRoom.room.publish(2, move(message), play->getSidsMap().begin()->second);

    response["message"] = "OK";
    response["action"] = 2;
    response["data"]["ready"] = play->getReady();
    response["data"]["histories"] = sharedRoom.room.getHistory(0, 10);
    response["data"]["players"] = sharedRoom.room.getPlayers();
    connection->send(websocket::fromJson(response));
}

void PlayManager::unsubscribe(const string &rid, const WebSocketConnectionPtr &connection) {
    {
        auto &sharedRoom = getSharedRoom(rid);
        sharedRoom.room.unsubscribe(connection);
        if (!sharedRoom.room.isEmpty()) {
            Json::Value message, response;
            message["message"] = "Broadcast";
            message["action"] = 3;
            message["data"] = _getPlay(connection)->parsePlayerInfo(Json::objectValue); // TODO: Remove unnecessary items.
            sharedRoom.room.publish(3, move(message));

            if (connection->connected()) {
                response["message"] = "OK";
                response["action"] = 3;
                connection->send(websocket::fromJson(response));
            }
            return;
        }
    }
    if (getSharedRoom(rid).room.isEmpty()) { // TODO: Check if thread safe
        removeRoom(rid);
    }
}

void PlayManager::publish(const string &rid, const uint64_t &action, Json::Value &&data) {
    auto &sharedRoom = getSharedRoom(rid);
    if (action == 4) {
        data["time"] = misc::fromDate();
    }
    Json::Value response;
    response["message"] = "Server";
    response["action"] = action;
    response["data"] = move(data);
    sharedRoom.room.publish(action, move(response));
}

void PlayManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    auto &sharedRoom = getSharedRoom(rid);
    data["time"] = misc::fromDate();
    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getPlay(connection)->parsePlayerInfo(move(data));
    sharedRoom.room.publish(action, move(response));
}

void PlayManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data,
        const uint64_t &excluded
) {
    auto &sharedRoom = getSharedRoom(rid);
    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getPlay(connection)->parsePlayerInfo(move(data)); // TODO: Remove unnecessary items.
    sharedRoom.room.publish(action, move(response), excluded);
}

void PlayManager::changeConfig(
        const string &rid,
        string &&config,
        const WebSocketConnectionPtr &connection
) {
    auto play = _getPlay(connection);
    Json::Value data;
    data["config"] = config;
    play->setConfig(move(config));
    publish(rid, connection, 5, move(data), play->getSidsMap().begin()->second);
}

void PlayManager::changeReady(
        const string &rid,
        const bool &ready,
        const WebSocketConnectionPtr &connection
) {
    auto play = _getPlay(connection);
    Json::Value data;
    data["ready"] = ready;
    play->setReady(ready);
    publish(rid, connection, 6, move(data), play->getSidsMap().begin()->second);
    _checkReady(rid);
}

Json::Value PlayManager::parseInfo(
        const string &type,
        const unsigned int &begin,
        const unsigned int &count
) const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    if (begin < _idsMap.size()) {
        unsigned int counter = 0;
        for (const auto &pair : _idsMap) {
            shared_lock<shared_mutex> roomLock(pair.second.sharedMutex);
            if (counter < begin) {
                ++counter;
                continue;
            }
            if (counter >= begin + count) {
                break;
            }
            if (type.empty() || type == pair.second.room.getType()) {
                info.append(pair.second.room.parseInfo());
            }
            ++counter;
        }
    }
    return info;
}

shared_ptr<Play> PlayManager::_getPlay(const drogon::WebSocketConnectionPtr &connection) {
    return connection->getContext<Play>();
}

void PlayManager::_checkReady(const std::string &rid) {
    bool allReady = true;
    {
        auto &sharedRoom = getSharedRoom(rid);
        if (sharedRoom.room.getPendingStart()) {
            return;
        }
        auto players = sharedRoom.room.getPlayers();
        for (auto player : players) {
            if (!player["ready"]) {
                allReady = false;
                break;
            }
        }
        if (allReady) {
            sharedRoom.room.setPendingStart(true);
            Json::Value response;
            response["message"] = "Server";
            response["action"] = 7;
            sharedRoom.room.publish(7, move(response));
        }
    }
    if (allReady) {
        thread([this, &rid]() { // TODO: is room safe here?
            auto &sharedRoom = getSharedRoom(rid);
            for (unsigned int milliseconds = 0; milliseconds < 300; ++milliseconds) {
                this_thread::sleep_for(chrono::milliseconds(10));
                auto players = sharedRoom.room.getPlayers();
                bool allReady = true;
                for (auto player : players) {
                    if (!player["ready"]) {
                        allReady = false;
                        break;
                    }
                }
                if (!allReady) {
                    sharedRoom.room.setPendingStart(false);
                    return;
                }
            }
            Json::Value response;
            response["message"] = "Server";
            response["action"] = 8;
            response["data"]["rid"] = crypto::blake2b(drogon::utils::getUuid(), 1);
            sharedRoom.room.publish(8, move(response));
            sharedRoom.room.setPendingStart(false);
            sharedRoom.room.setStart(true);
        }).detach();
    }
}
