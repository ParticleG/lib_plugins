//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/chat/GetChannelList.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

GetChannelList::GetChannelList() = default;

CloseCode GetChannelList::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    response["type"] = "Self";
    response["action"] = static_cast<int>(actions::Chat::getChannelList);
    response["roomList"] = app().getPlugin<ChatManager>()->parseInfo();
    return CloseCode::kNormalClosure;
}