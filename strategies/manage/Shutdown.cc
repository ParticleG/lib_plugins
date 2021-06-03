//
// Created by Parti on 2021/6/3.
//

#include <plugins/AppManager.h>
#include <strategies/actions.h>
#include <strategies/manage/Shutdown.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace drogon;
using namespace std;

Shutdown::Shutdown() = default;

drogon::CloseCode Shutdown::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("countDown") && request["data"]["countDown"].isUInt64()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires UInt64 type 'countDown' in 'data'";
    } else {
        response["type"] = "Server";
        response["action"] = static_cast<int>(actions::App::publishMessage);
        response["data"]["message"] = "<shutdown>" + to_string(request["data"]["countDown"].asUInt64());
        app().getPlugin<AppManager>()->publish(response);

        response["type"] = "Self";
        response["action"] = static_cast<int>(actions::Manage::shutdown);
        thread([request]() {
            this_thread::sleep_for(chrono::seconds(request["data"]["countDown"].asUInt64()));
            app().quit();
        }).detach();
    }
    return CloseCode::kNormalClosure;
}