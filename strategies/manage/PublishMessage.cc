//
// Created by Parti on 2021/6/3.
//

#include <plugins/AppManager.h>
#include <strategies/actions.h>
#include <strategies/manage/PublishMessage.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace drogon;
using namespace std;

PublishMessage::PublishMessage() = default;

drogon::CloseCode PublishMessage::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("message") && request["data"]["message"].isString()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires String type 'message' in 'data'";
    } else {
        response["type"] = "Server";
        response["action"] = static_cast<int>(actions::App::publishMessage);
        response["data"]["message"] = request["data"]["message"].asString();
        app().getPlugin<AppManager>()->publish(response);

        response["type"] = "Self";
        response["action"] = static_cast<int>(actions::Manage::publishMessage);
    }
    return CloseCode::kNormalClosure;
}