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
#include <strategies/chat/PublishMessage.h>

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
    auto appPrefix = config["App"].asInt() * 10;
    static HandlerRegistrar<MessageHandler, GetAppVersion> getAppVersionRegistrar(appPrefix + 0);
    static HandlerRegistrar<MessageHandler, GetNotice> getNoticeRegistrar(appPrefix + 1);

    auto userPrefix = config["User"].asInt() * 10;
    static HandlerRegistrar<MessageHandler, GetAccessToken> getAccessTokenRegistrar(userPrefix + 0);
    static HandlerRegistrar<MessageHandler, GetUserInfo> getUserInfoRegistrar(userPrefix + 1);

    auto chatPrefix = config["Chat"].asInt() * 10;
    static HandlerRegistrar<MessageHandler, GetChannelList> getChannelListRegistrar(chatPrefix + 0);
    static HandlerRegistrar<MessageHandler, EnterChannel> enterChannelRegistrar(chatPrefix + 1);
    static HandlerRegistrar<MessageHandler, LeaveChannel> leaveChannelRegistrar(chatPrefix + 2);
    static HandlerRegistrar<MessageHandler, PublishMessage> publishMessageRegistrar(chatPrefix + 3);

    auto playPrefix = config["Play"].asInt() * 10;
    auto streamPrefix = config["Stream"].asInt() * 10;
}

void HandlerManager::shutdown() {}

CloseCode HandlerManager::process(
        const WebSocketConnectionPtr &wsConnPtr,
        const WebSocket::Type &type,
        const unsigned int &action,
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
