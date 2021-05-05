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
    misc::logger(typeid(*this).name(), "Try get room capacity: " + type);
    shared_lock<shared_mutex> lock(_sharedMutex);
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
    response["data"]["sid"] = play->getSid();
    response["data"]["ready"] = play->getReady();
    response["data"]["histories"] = sharedRoom.room.getHistory(0, 10);
    response["data"]["players"] = sharedRoom.room.getPlayers();
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

void PlayManager::publish(const string &rid, const uint64_t &action, Json::Value &&data) {
    auto sharedRoom = getSharedRoom(rid);
    if (action == static_cast<int>(actions::Play::publishPlayMessage)) {
        data["time"] = misc::fromDate();
    }
    Json::Value response;
    response["type"] = "Server";
    response["action"] = action;
    response["data"] = move(data);
    misc::logger(typeid(*this).name(), "Server: " + websocket::fromJson(response));
    sharedRoom.room.publish(action, move(response));
}

void PlayManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data
) {
    auto sharedRoom = getSharedRoom(rid);
    data["time"] = misc::fromDate();
    Json::Value response;
    response["type"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getPlay(connection)->parsePlayerInfo(move(data));
    misc::logger(typeid(*this).name(), "Broadcast: " + websocket::fromJson(response));
    sharedRoom.room.publish(action, move(response));
}

void PlayManager::publish(
        const string &rid,
        const WebSocketConnectionPtr &connection,
        const uint64_t &action,
        Json::Value &&data,
        const uint64_t &excluded
) {
    auto sharedRoom = getSharedRoom(rid);
    Json::Value response;
    response["type"] = "Broadcast";
    response["action"] = action;
    response["data"] = _getPlay(connection)->parsePlayerInfo(move(data)); // TODO: Remove unnecessary items.
    misc::logger(typeid(*this).name(), "Exclude broadcast: " + websocket::fromJson(response));
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
    publish(rid, connection, static_cast<int>(actions::Play::changeConfig), move(data), play->getSid());
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
    publish(rid, connection, static_cast<int>(actions::Play::changeReady), move(data));
    _checkReady(rid);
}

Json::Value PlayManager::parseInfo(
        const string &type,
        const unsigned int &begin,
        const unsigned int &count
) const {
    misc::logger(typeid(*this).name(), "Try parsing manager info: (" + type + ") " + to_string(count));
    shared_lock<shared_mutex> lock(_sharedMutex);
    Json::Value info(Json::arrayValue);
    if (begin < _idsMap.size()) {
        unsigned int counter = 0;
        for (const auto &[id, room_with_mutex] : _idsMap) {
            shared_lock<shared_mutex> roomLock(*room_with_mutex.sharedMutex);
            if (counter < begin) {
                ++counter;
                continue;
            }
            if (counter >= begin + count) {
                break;
            }
            if (type.empty() || type == room_with_mutex.room.getType()) {
                info.append(room_with_mutex.room.parseInfo());
            }
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
