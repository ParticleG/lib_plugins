//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/StreamManager.h>
#include <strategies/actions.h>
#include <strategies/manage/GetStreamManagerStatus.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace drogon;
using namespace std;

GetStreamManagerStatus::GetStreamManagerStatus() = default;

drogon::CloseCode GetStreamManagerStatus::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        Json::Value data;
        auto streamManager = app().getPlugin<StreamManager>();
        for (const auto &rid: streamManager->getRidList()) {
            try {
                auto sharedRoom = streamManager->getSharedRoom(rid);
                Json::Value tempJson = sharedRoom.room.parseInfo();
                tempJson["players"] = sharedRoom.room.getPlayers();
                data.append(tempJson);
            } catch (...) {}
        }

        response["action"] = static_cast<int>(actions::Manage::getStreamManagerStatus);
        response["type"] = "Self";
        response["data"] = data;
        return CloseCode::kNormalClosure;
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
}
