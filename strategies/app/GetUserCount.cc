//
// Created by Particle_G on 2021/2/17.
//

#include <strategies/actions.h>
#include <strategies/app/GetUserCount.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace drogon_model;
using namespace drogon;
using namespace std;

drogon::CloseCode GetUserCount::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    response["action"] = static_cast<int>(actions::App::getUserCount);
    response["type"] = "Self";
    response["data"] = app().getPlugin<UserManager>()->getCount();
    return CloseCode::kNormalClosure;
}
