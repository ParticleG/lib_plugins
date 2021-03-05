//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/LeaveRoom.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

LeaveRoom::LeaveRoom() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode LeaveRoom::fromJson(
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
            _playManager->unsubscribe(rid, wsConnPtr);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}