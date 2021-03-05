//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/ChangeConfig.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

ChangeConfig::ChangeConfig() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode ChangeConfig::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("config") && request["data"]["config"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'config' in 'data'";
    } else {
        auto rid = wsConnPtr->getContext<Play>()->getSidsMap()->begin()->first;
        try {
            _playManager->changeConfig(rid, request["data"]["config"].asString(), wsConnPtr);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}