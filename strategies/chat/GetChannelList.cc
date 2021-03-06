//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/Configurator.h>
#include <strategies/chat/GetChannelList.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

GetChannelList::GetChannelList() : _chatManager(app().getPlugin<ChatManager>()) {}

CloseCode GetChannelList::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    response["message"] = "OK";
    response["action"] = 0;
    response["roomList"] = _chatManager->parseInfo();
    return CloseCode::kNone;
}