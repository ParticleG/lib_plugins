//
// Created by Parti on 2021/2/19.
//

#include <plugins/HandlerManager.h>
#include <strategies/app/GetAppVersion.h>
#include <strategies/app/GetNotice.h>
#include <strategies/app/ValidateAccount.h>
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
    static HandlerRegistrar<MessageHandler, GetAppVersion> getAppVersionRegistrar(actions::Prefix::app + actions::App::getAppVersion);
    static HandlerRegistrar<MessageHandler, GetNotice> getNoticeRegistrar(actions::Prefix::app + actions::App::getNotice);
    static HandlerRegistrar<MessageHandler, ValidateAccount> validateAccountRegistrar(actions::Prefix::app + actions::App::validateAccount);

    static HandlerRegistrar<MessageHandler, GetAccessToken> getAccessTokenRegistrar(actions::Prefix::user + actions::User::getAccessToken);
    static HandlerRegistrar<MessageHandler, GetUserInfo> getUserInfoRegistrar(actions::Prefix::user + actions::User::getUserInfo);

    static HandlerRegistrar<MessageHandler, GetChannelList> getChannelListRegistrar(actions::Prefix::chat + actions::Chat::getChannelList);
    static HandlerRegistrar<MessageHandler, EnterChannel> enterChannelRegistrar(actions::Prefix::chat + actions::Chat::enterChannel);
    static HandlerRegistrar<MessageHandler, LeaveChannel> leaveChannelRegistrar(actions::Prefix::chat + actions::Chat::leaveChannel);
    static HandlerRegistrar<MessageHandler, PublishChatMessage> publishChatMessageRegistrar(actions::Prefix::chat + actions::Chat::publishChatMessage);

    static HandlerRegistrar<MessageHandler, GetRoomList> getRoomListRegistrar(actions::Prefix::play + actions::Play::getRoomList);
    static HandlerRegistrar<MessageHandler, CreateRoom> createRoomRegistrar(actions::Prefix::play + actions::Play::createRoom);
    static HandlerRegistrar<MessageHandler, EnterRoom> enterRoomRegistrar(actions::Prefix::play + actions::Play::enterRoom);
    static HandlerRegistrar<MessageHandler, LeaveRoom> leaveRoomRegistrar(actions::Prefix::play + actions::Play::leaveRoom);
    static HandlerRegistrar<MessageHandler, PublishPlayMessage> publishPlayMessageRegistrar(actions::Prefix::play + actions::Play::publishPlayMessage);
    static HandlerRegistrar<MessageHandler, ChangeConfig> changeConfigRegistrar(actions::Prefix::play + actions::Play::changeConfig);
    static HandlerRegistrar<MessageHandler, ChangeReady> changeReadyRegistrar(actions::Prefix::play + actions::Play::changeReady);

    static HandlerRegistrar<MessageHandler, PublishDeathData> publishDeathDataRegistrar(actions::Prefix::stream + actions::Stream::publishDeathData);
    static HandlerRegistrar<MessageHandler, PublishStreamData> publishStreamDataRegistrar(actions::Prefix::stream + actions::Stream::publishStreamData);
    LOG_INFO << "HandlerManager loaded.";
}

void HandlerManager::shutdown() {
    LOG_INFO << "HandlerManager shutdown.";
}

CloseCode HandlerManager::process(
        const WebSocketConnectionPtr &wsConnPtr,
        const actions::Prefix &prefix,
        int action,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        auto handler(_handlerFactory.getHandler(prefix + action));
        CloseCode code = handler->fromJson(wsConnPtr, request, response);
        return code;
    } catch (const out_of_range &e) {
        response["type"] = "Warn";
        response["reason"] = e.what();
        return CloseCode::kNormalClosure;
    }
}
