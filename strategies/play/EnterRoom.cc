//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/EnterRoom.h>
#include <structures/Play.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

EnterRoom::EnterRoom() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode EnterRoom::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("rid") && request["data"]["rid"].isString() &&
            request["data"].isMember("config") && request["data"]["config"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'rid' and 'config' in 'data'";
    } else {
        string rid, password;
        rid = request["data"]["rid"].asString();
        wsConnPtr->getContext<Play>()->setConfig(request["data"]["config"].asString());
        if (request["data"].isMember("password") && request["data"]["password"].isString()) {
            password = request["data"]["password"].asString();
        }
        try {
            _playManager->subscribe(rid, password, wsConnPtr);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}