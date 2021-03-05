//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/Configurator.h>
#include <strategies/chat/EnterChannel.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

EnterChannel::EnterChannel() : _chatManager(app().getPlugin<ChatManager>()) {}

CloseCode EnterChannel::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("rid") && request["data"]["rid"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'rid' in 'data'";
    } else {
        auto rid = request["data"]["rid"].asString();
        try {
            _chatManager->subscribe(rid, wsConnPtr);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}