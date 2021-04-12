//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/stream/PublishDeathData.h>

using namespace drogon;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

PublishDeathData::PublishDeathData() = default;

CloseCode PublishDeathData::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("score") && request["data"]["score"].isUInt64() &&
            request["data"].isMember("survivalTime") && request["data"]["survivalTime"].isUInt64()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires UInt64 type 'score' and 'survivalTime' in 'data'";
    } else {
        auto stream = wsConnPtr->getContext<Stream>();
        stream->setScore(request["data"]["score"].asUInt64());
        stream->setSurvivalTime(request["data"]["survivalTime"].asUInt64());
        stream->setDead(true);

        auto rid = get<string>(stream->getRid());
        auto data = request["data"];
        try {
            app().getPlugin<StreamManager>()->publish(rid, wsConnPtr, static_cast<int>(actions::Stream::publishDeathData), move(data));
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}