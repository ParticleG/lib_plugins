//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/PlayManager.h>
#include <strategies/actions.h>
#include <strategies/manage/GetPlayManagerStatus.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace drogon;
using namespace std;

GetPlayManagerStatus::GetPlayManagerStatus() = default;

drogon::CloseCode GetPlayManagerStatus::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        Json::Value data;
        auto playManager = app().getPlugin<PlayManager>();
        for (const auto &rid: playManager->getRidList()) {
            try {
                auto sharedRoom = playManager->getSharedRoom(rid);
                Json::Value tempJson = sharedRoom.room.parseInfo();
                tempJson["history"] = sharedRoom.room.getHistory(0, 100);
                tempJson["players"] = sharedRoom.room.getPlayers();
                data.append(tempJson);
            } catch (...) {}
        }

        response["action"] = static_cast<int>(actions::Manage::getPlayManagerStatus);
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
