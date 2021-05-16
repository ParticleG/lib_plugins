//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/play/PublishPlayMessage.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

PublishPlayMessage::PublishPlayMessage() = default;

CloseCode PublishPlayMessage::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("message") && request["data"]["message"].isString()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires string type 'message' in 'data'";
    } else {
        auto rid = get<string>(wsConnPtr->getContext<Play>()->getRid());
        auto data = request["data"];
        try {
            app().getPlugin<PlayManager>()->publish(rid, static_cast<int>(actions::Play::publishPlayMessage), move(data), wsConnPtr);
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}