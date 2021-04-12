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

LeaveRoom::LeaveRoom()= default;

CloseCode LeaveRoom::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        app().getPlugin<PlayManager>()->unsubscribe(get<string>(wsConnPtr->getContext<Play>()->getRid()), wsConnPtr);
        return CloseCode::kNone;
    } catch (const exception &error) {
        response["type"] = "Warn";
        response["reason"] = error.what();
    }
    return CloseCode::kNormalClosure;
}