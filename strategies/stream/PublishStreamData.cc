//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/stream/PublishStreamData.h>

using namespace drogon;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

PublishStreamData::PublishStreamData() : _streamManager(app().getPlugin<StreamManager>()) {}

CloseCode PublishStreamData::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("stream") && request["data"]["stream"].isString()
    )) {
        response["message"] = "Wrong format";
        response["reason"] = "Requires string type 'stream' in 'data'";
    } else {
        auto rid = wsConnPtr->getContext<Stream>()->getSidsMap()->begin()->first;
        auto data = request["data"];
        try {
            _streamManager->publish(rid, wsConnPtr, 2, move(data));
        } catch (const exception &error) {
            response["message"] = error.what();
        }
    }
    return CloseCode::kNone;
}