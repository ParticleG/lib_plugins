//
// Created by Parti on 2021/2/4.
//

#include <plugins/PlayManager.h>
#include <utils/Crypto.h>
#include <utils/Utils.h>

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
        const string &id,
        const string &password,
        const WebSocketConnectionPtr &connection
) {
    auto room = getRoom(id);
    if (room->getStart()) {
        throw invalid_argument("Room already started");
    }
    if (!room->checkPassword(password)) {
        throw invalid_argument("Password is incorrect");
    }
    room->subscribe(connection);
    auto play = _getPlay(connection);

    play->setReady(false);

    Json::Value data, message, response;
    data["config"] = play->getConfig();
    data["ready"] = play->getReady();

    message["message"] = "Broadcast";
    message["action"] = 2;
    message["data"] = play->parsePlayerInfo(move(data));
    room->publish(2, move(message), play->getSidsMap().begin()->second);

    response["message"] = "OK";
    response["action"] = 2;
    response["data"]["ready"] = play->getReady();
    response["data"]["histories"] = room->getHistory(0, 10);
    response["data"]["players"] = room->getPlayers();
    connection->send(WebSocket::fromJson(response));
}

void PlayManager::unsubscribe(const string &id, const WebSocketConnectionPtr &connection) {
    {
        auto room = getRoom(id);
        room->unsubscribe(connection);
        if (!room->isEmpty()) {
            Json::Value message, response;
            message["message"] = "Broadcast";
            message["action"] = 3;
            message["data"] = _getPlay(connection)->parsePlayerInfo(Json::objectValue); // TODO: Remove unnecessary items.
            room->publish(3, move(message));

            if (connection->connected()) {
                response["message"] = "OK";
                response["action"] = 3;
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

void PlayManager::publish(const string &rid, const uint64_t &action, Json::Value &&data) {
    auto room = getRoom(rid);
    if (action == 4) {
        data["time"] = Utils::fromDate();
    }
    Json::Value response;
    response["message"] = "Server";
    response["action"] = action;
    response["data"] = move(data);
    room->publish(action, move(response));
}

void PlayManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    auto room = getRoom(rid);
    data["time"] = Utils::fromDate();
    Json::Value response;
    response["message"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getPlay(connection)->parsePlayerInfo(move(data));
    room->publish(action, move(response));
}

void PlayManager::publish(
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
    response["data"] = _getPlay(connection)->parsePlayerInfo(move(data)); // TODO: Remove unnecessary items.
    room->publish(action, move(response), excluded);
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
            if (counter < begin) {
                ++counter;
                continue;
            }
            if (counter >= begin + count) {
                break;
            }
            if (type.empty() || type == pair.second.getType()) {
                info.append(pair.second.parseInfo());
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
    auto room = getRoom(rid);
    bool allReady = true;
    if (room->getPendingStart()) {
        return;
    }
    auto players = room->getPlayers();
    for (auto player : players) {
        if (!player["ready"]) {
            allReady = false;
            break;
        }
    }
    if (allReady) {
        room->setPendingStart(true);
        Json::Value response;
        response["message"] = "Server";
        response["action"] = 7;
        room->publish(7, move(response));

        thread([room{move(room)}]() { // is room safe here?
            for (unsigned int milliseconds = 0; milliseconds < 300; ++milliseconds) {
                this_thread::sleep_for(chrono::milliseconds(10));
                auto players = room->getPlayers();
                bool allReady = true;
                for (auto player : players) {
                    if (!player["ready"]) {
                        allReady = false;
                        break;
                    }
                }
                if (!allReady) {
                    room->setPendingStart(false);
                    return;
                }
            }
            Json::Value response;
            response["message"] = "Server";
            response["action"] = 8;
            response["data"]["rid"] = Crypto::blake2b(drogon::utils::getUuid(), 1);
            room->publish(8, move(response));
            room->setPendingStart(false);
            room->setStart(true);
        }).detach();
    }
}
