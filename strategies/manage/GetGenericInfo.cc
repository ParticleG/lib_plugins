//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/UserManager.h>
#include <strategies/actions.h>
#include <strategies/manage/GetGenericInfo.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace drogon;
using namespace std;

GetGenericInfo::GetGenericInfo() = default;

drogon::CloseCode GetGenericInfo::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        auto userManager = app().getPlugin<UserManager>();

        response["action"] = static_cast<int>(actions::Manage::getGenericInfo);
        response["type"] = "Self";
        response["data"] = userManager->parseInfo();;
        return CloseCode::kNormalClosure;
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
}
