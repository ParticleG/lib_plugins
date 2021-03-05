//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/CreateRoom.h>
#include <utils/Crypto.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

CreateRoom::CreateRoom() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode CreateRoom::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("config") && request["data"]["config"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'rid' and 'config' in 'data'";
    } else {
        string type = "classic", name, password, config;

        if (request["data"].isMember("type") && request["data"]["type"].isString()) {
            type = request["data"]["type"].asString();
        }

        if (request["data"].isMember("name") && request["data"]["name"].isString()) {
            name = request["data"]["name"].asString();
        } else {
            name = type + " room";
        }

        if (request["data"].isMember("password") && request["data"]["password"].isString()) {
            password = request["data"]["password"].asString();
        }

        wsConnPtr->getContext<Play>()->setConfig(request["data"]["config"].asString());

        try {
            auto capacity = _playManager->getCapacity(type);
            auto rid = Crypto::blake2b(drogon::utils::getUuid());
            auto room = PlayRoom(
                    rid,
                    type,
                    name,
                    password,
                    capacity
            );
            _playManager->createRoom(move(room));
            _playManager->subscribe(rid, password, wsConnPtr);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }

    return CloseCode::kNone;
}