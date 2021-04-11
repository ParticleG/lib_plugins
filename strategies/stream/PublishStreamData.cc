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
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires string type 'stream' in 'data'";
    } else {
        auto stream = wsConnPtr->getContext<Stream>();
        auto data = request["data"];
        try {
            _streamManager->publish(
                    get<string>(stream->getRid()),
                    wsConnPtr,
                    static_cast<int>(actions::Stream::publishStreamData),
                    move(data),
                    stream->getSid());
            return CloseCode::kNone;
        } catch (const exception &error) {
            response["type"] = "Warn";
            response["reason"] = error.what();
        }
    }
    return CloseCode::kNormalClosure;
}