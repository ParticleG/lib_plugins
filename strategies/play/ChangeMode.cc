//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/ChangeMode.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

ChangeMode::ChangeMode() = default;

CloseCode ChangeMode::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("mode") && request["data"]["mode"].isInt64()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires Int64 type 'mode' in 'data'";
    } else {
        auto rid = get<string>(wsConnPtr->getContext<Play>()->getRid());
        try {
            app().getPlugin<PlayManager>()->changeMode(rid, request["data"]["mode"].asInt64(), wsConnPtr);
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}