//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/stream/PublishStreamData.h>

using namespace drogon;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

PublishStreamData::PublishStreamData() = default;

CloseCode PublishStreamData::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("stream") && request["data"]["stream"].isString()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires string type 'stream' in 'data'";
    } else {
        auto stream = wsConnPtr->getContext<Stream>();
        if (!stream) {
            misc::logger(typeid(*this).name(), "Get 'Stream' failed");
            response["type"] = "Error";
            response["reason"] = "Get 'Stream' failed (nullptr)";
            return CloseCode::kUnexpectedCondition;
        }
        if (stream->getWatch()) {
            response["type"] = "Warn";
            response["reason"] = "You are in watch mode";
            return CloseCode::kNormalClosure;
        }
        auto data = request["data"];
        stream->addHistory(request["data"]["stream"].asString());
        try {
            app().getPlugin<StreamManager>()->publish(
                    get<string>(stream->getRid()),
                    static_cast<int>(actions::Stream::publishStreamData),
                    move(data),
                    wsConnPtr,
                    stream->getSid());
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}