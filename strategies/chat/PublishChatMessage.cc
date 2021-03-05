//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/chat/PublishChatMessage.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

PublishChatMessage::PublishChatMessage() : _chatManager(app().getPlugin<ChatManager>()) {}

CloseCode PublishChatMessage::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("rid") && request["data"]["rid"].isString() &&
            request["data"].isMember("message") && request["data"]["message"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'rid' and 'message' in 'data'";
    } else {
        auto rid = request["data"]["rid"].asString();
        auto message = request["data"]["message"].asString();
        try {
            _chatManager->publish(rid, wsConnPtr, message);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}