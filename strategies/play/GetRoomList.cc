//
// Created by Particle_G on 2021/3/04.
//

#include <plugins/Configurator.h>
#include <strategies/play/GetRoomList.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

GetRoomList::GetRoomList() : _playManager(app().getPlugin<PlayManager>()) {}

CloseCode GetRoomList::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    string type;
    unsigned int begin = 0, count = 10;
    if (request.isMember("data") && request["data"].isObject()) {
        if (request["data"].isMember("type") && request["data"]["type"].isString()) {
            type = request["data"]["type"].asString();
        }
        if (request["data"].isMember("begin") && request["data"]["begin"].isUInt()) {
            begin = request["data"]["begin"].asUInt();
        }
        if (request["data"].isMember("count") && request["data"]["count"].isUInt()) {
            count = request["data"]["count"].asUInt();
        }
    }
    response["message"] = "OK";
    response["action"] = 0;
    response["roomList"] = _playManager->parseInfo(type, begin, count);
    return CloseCode::kNone;
}