//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/CreateRoom.h>
#include <utils/crypto.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

CreateRoom::CreateRoom() = default;

CloseCode CreateRoom::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("capacity") && request["data"]["capacity"].isUInt64() &&
            request["data"].isMember("config") && request["data"]["config"].isString() &&
            request["data"].isMember("roomData") && request["data"]["roomData"].isObject()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires UInt64 type 'capacity', string type 'rid', 'config' and object type 'roomData' in 'data'";
    } else {
        string name, password, config;

        if (request["data"].isMember("name") && request["data"]["name"].isString()) {
            name = request["data"]["name"].asString();
        } else {
            name = "A " + to_string(request["data"]["capacity"].asUInt64()) + " room";
        }

        if (request["data"].isMember("password") && request["data"]["password"].isString()) {
            password = request["data"]["password"].asString();
        }

        wsConnPtr->getContext<Play>()->setConfig(request["data"]["config"].asString());

        try {
            auto playManager = app().getPlugin<PlayManager>();
            auto rid = crypto::blake2b(drogon::utils::getUuid());
            auto room = PlayRoom(
                    rid,
                    name,
                    password,
                    request["data"]["capacity"].asUInt64(),
                    request["data"]["roomData"]
            );
            playManager->createRoom(move(room));
            playManager->subscribe(rid, password, wsConnPtr);
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }

    return CloseCode::kNormalClosure;
}