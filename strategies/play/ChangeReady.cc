//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/ChangeReady.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

ChangeReady::ChangeReady() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode ChangeReady::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("ready") && request["data"]["ready"].isBool()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires bool type 'ready' in 'data'";
    } else {
        auto rid = wsConnPtr->getContext<Play>()->getSidsMap()->begin()->first;
        try {
            _playManager->changeReady(rid, request["data"]["ready"].asBool(), wsConnPtr);
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}