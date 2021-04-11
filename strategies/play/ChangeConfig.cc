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
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires string type 'config' in 'data'";
    } else {
        auto rid = get<string>(wsConnPtr->getContext<Play>()->getRid());
        try {
            _playManager->changeConfig(rid, request["data"]["config"].asString(), wsConnPtr);
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}