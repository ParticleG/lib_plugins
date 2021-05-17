//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/play/GetRoomList.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

GetRoomList::GetRoomList() = default;

CloseCode GetRoomList::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    unsigned int begin = 0, count = 10;
    if (request.isMember("data") && request["data"].isObject()) {
        if (request["data"].isMember("begin") && request["data"]["begin"].isUInt()) {
            begin = request["data"]["begin"].asUInt();
        }
        if (request["data"].isMember("count") && request["data"]["count"].isUInt()) {
            count = request["data"]["count"].asUInt();
        }
    }
    response["type"] = "Self";
    response["action"] = static_cast<int>(actions::Play::getRoomList);
    response["roomList"] = app().getPlugin<PlayManager>()->parseInfo(begin, count);
    return CloseCode::kNormalClosure;
}