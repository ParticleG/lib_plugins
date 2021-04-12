//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/EnterRoom.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

EnterRoom::EnterRoom()  = default;

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
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires string type 'rid' and 'config' in 'data'";
    } else {
        string rid, password;
        rid = request["data"]["rid"].asString();
        wsConnPtr->getContext<Play>()->setConfig(request["data"]["config"].asString());
        if (request["data"].isMember("password") && request["data"]["password"].isString()) {
            password = request["data"]["password"].asString();
        }
        try {
            app().getPlugin<PlayManager>()->subscribe(rid, password, wsConnPtr);
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}