//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/PublishPlayMessage.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

PublishPlayMessage::PublishPlayMessage() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode PublishPlayMessage::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("message") && request["data"]["message"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'message' in 'data'";
    } else {
        auto rid = wsConnPtr->getContext<Play>()->getSidsMap()->begin()->first;
        auto data = request["data"];
        try {
            _playManager->publish(rid, wsConnPtr, 4, move(data));
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}