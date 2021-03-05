//
// Created by Parti on 2021/2/19.
//

#include <plugins/HandlerManager.h>
#include <strategies/app/GetAppVersion.h>
#include <strategies/app/GetNotice.h>
#include <strategies/user/GetAccessToken.h>
#include <strategies/user/GetUserInfo.h>
#include <strategies/chat/GetChannelList.h>
#include <strategies/chat/EnterChannel.h>
#include <strategies/chat/LeaveChannel.h>
#include <strategies/chat/PublishChatMessage.h>
#include <strategies/play/GetRoomList.h>
#include <strategies/play/CreateRoom.h>
#include <strategies/play/EnterRoom.h>
#include <strategies/play/LeaveRoom.h>
#include <strategies/play/PublishPlayMessage.h>
#include <strategies/play/ChangeConfig.h>
#include <strategies/play/ChangeReady.h>
#include <strategies/stream/PublishStreamData.h>
#include <strategies/stream/PublishDeathData.h>

using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace drogon;
using namespace std;

HandlerManager::HandlerManager() : _handlerFactory(tech::structures::HandlerFactory<tech::strategies::MessageHandler>::instance()) {}

void HandlerManager::initAndStart(const Json::Value &config) {
    if (!(
            config.isMember("App") && config["App"].isInt() &&
            config.isMember("User") && config["User"].isInt() &&
            config.isMember("Chat") && config["Chat"].isInt() &&
            config.isMember("Play") && config["Play"].isInt() &&
            config.isMember("Stream") && config["Stream"].isInt()
    )) {
        LOG_ERROR << R"(Requires unsigned int value "App", "User", "Chat", "Play" and "Stream" in plugin HandlerManager's config')";
        abort();
    }
    auto appPrefix = static_cast<const unsigned int &>(config["App"].asInt() * 100);
    static HandlerRegistrar<MessageHandler, GetAppVersion> getAppVersionRegistrar(appPrefix + 0);
    static HandlerRegistrar<MessageHandler, GetNotice> getNoticeRegistrar(appPrefix + 1);

    auto userPrefix = config["User"].asInt() * 100;
    static HandlerRegistrar<MessageHandler, GetAccessToken> getAccessTokenRegistrar(userPrefix + 0);
    static HandlerRegistrar<MessageHandler, GetUserInfo> getUserInfoRegistrar(userPrefix + 1);

    auto chatPrefix = config["Chat"].asInt() * 100;
    static HandlerRegistrar<MessageHandler, GetChannelList> getChannelListRegistrar(chatPrefix + 0);
    static HandlerRegistrar<MessageHandler, EnterChannel> enterChannelRegistrar(chatPrefix + 1);
    static HandlerRegistrar<MessageHandler, LeaveChannel> leaveChannelRegistrar(chatPrefix + 2);
    static HandlerRegistrar<MessageHandler, PublishChatMessage> publishChatMessageRegistrar(chatPrefix + 3);

    auto playPrefix = config["Play"].asInt() * 10;
    static HandlerRegistrar<MessageHandler, GetRoomList> getRoomListRegistrar(playPrefix + 0);
    static HandlerRegistrar<MessageHandler, CreateRoom> createRoomRegistrar(playPrefix + 1);
    static HandlerRegistrar<MessageHandler, EnterRoom> enterRoomRegistrar(playPrefix + 2);
    static HandlerRegistrar<MessageHandler, LeaveRoom> leaveRoomRegistrar(playPrefix + 3);
    static HandlerRegistrar<MessageHandler, PublishPlayMessage> publishPlayMessageRegistrar(playPrefix + 4);
    static HandlerRegistrar<MessageHandler, ChangeConfig> changeConfigRegistrar(playPrefix + 5);
    static HandlerRegistrar<MessageHandler, ChangeReady> changeReadyRegistrar(playPrefix + 6);

    auto streamPrefix = config["Stream"].asInt() * 10;
    static HandlerRegistrar<MessageHandler, PublishStreamData> publishStreamDataRegistrar(streamPrefix + 2);
    static HandlerRegistrar<MessageHandler, PublishDeathData> publishDeathData(streamPrefix + 3);
}

void HandlerManager::shutdown() {}

CloseCode HandlerManager::process(
        const WebSocketConnectionPtr &wsConnPtr,
        const WebSocket::Type &type,
        int action,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        auto handler(_handlerFactory.getHandler(static_cast<int>(type) * 10 + action));
        CloseCode code = handler->fromJson(wsConnPtr, request, response);
        return code;
    } catch (const out_of_range &e) {
        response["message"] = e.what();
        return CloseCode::kNone;
    }

}
