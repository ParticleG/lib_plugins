//
// Created by Parti on 2021/2/19.
//

#include <plugins/HandlerManager.h>
#include <strategies/app/GetAppVersion.h>
#include <strategies/app/GetNotice.h>
#include <strategies/app/ValidateAccount.h>
#include <strategies/app/GetUserCount.h>
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

HandlerManager::HandlerManager() = default;

void HandlerManager::initAndStart(const Json::Value &config) {
    _handlerFactory.registerHandler<GetAppVersion>(actions::Prefix::app + actions::App::getAppVersion);
    _handlerFactory.registerHandler<GetNotice>(actions::Prefix::app + actions::App::getNotice);
    _handlerFactory.registerHandler<ValidateAccount>(actions::Prefix::app + actions::App::validateAccount);
    _handlerFactory.registerHandler<GetUserCount>(actions::Prefix::app + actions::App::getUserCount);

    _handlerFactory.registerHandler<GetAccessToken>(actions::Prefix::user + actions::User::getAccessToken);
    _handlerFactory.registerHandler<GetUserInfo>(actions::Prefix::user + actions::User::getUserInfo);

    _handlerFactory.registerHandler<GetChannelList>(actions::Prefix::chat + actions::Chat::getChannelList);
    _handlerFactory.registerHandler<EnterChannel>(actions::Prefix::chat + actions::Chat::enterChannel);
    _handlerFactory.registerHandler<LeaveChannel>(actions::Prefix::chat + actions::Chat::leaveChannel);
    _handlerFactory.registerHandler<PublishChatMessage>(actions::Prefix::chat + actions::Chat::publishChatMessage);

    _handlerFactory.registerHandler<GetRoomList>(actions::Prefix::play + actions::Play::getRoomList);
    _handlerFactory.registerHandler<CreateRoom>(actions::Prefix::play + actions::Play::createRoom);
    _handlerFactory.registerHandler<EnterRoom>(actions::Prefix::play + actions::Play::enterRoom);
    _handlerFactory.registerHandler<LeaveRoom>(actions::Prefix::play + actions::Play::leaveRoom);
    _handlerFactory.registerHandler<PublishPlayMessage>(actions::Prefix::play + actions::Play::publishPlayMessage);
    _handlerFactory.registerHandler<ChangeConfig>(actions::Prefix::play + actions::Play::changeConfig);
    _handlerFactory.registerHandler<ChangeReady>(actions::Prefix::play + actions::Play::changeReady);

    _handlerFactory.registerHandler<PublishDeathData>(actions::Prefix::stream + actions::Stream::publishDeathData);
    _handlerFactory.registerHandler<PublishStreamData>(actions::Prefix::stream + actions::Stream::publishStreamData);
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
        auto &handler = _handlerFactory.getHandler(prefix + action);
        CloseCode code = handler.fromJson(wsConnPtr, request, response);
        return code;
    } catch (const out_of_range &e) {
        response["type"] = "Warn";
        response["reason"] = e.what();
        return CloseCode::kNormalClosure;
    }
}
