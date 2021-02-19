//
// Created by Parti on 2021/2/19.
//

#include <plugins/HandlerManager.h>
#include <strategies/app/CheckAppVersion.h>
#include <strategies/app/GetNotice.h>
#include <strategies/app/GetAppVersion.h>
#include <strategies/app/NewLogin.h>

using namespace tech::plugin;
using namespace tech::strategies;
using namespace tech::structures;
using namespace drogon;
using namespace std;

void HandlerManager::initAndStart(const Json::Value &config) {
    static HandlerRegistrar<MessageHandler, CheckAppVersion> checkAppVersionRegistrar(00);
    static HandlerRegistrar<MessageHandler, GetAppVersion> getAppVersionRegistrar(10);
    static HandlerRegistrar<MessageHandler, GetNotice> getNoticeRegistrar(11);
    static HandlerRegistrar<MessageHandler, NewLogin> newLoginRegistrar(20);
}

void HandlerManager::shutdown() {

}

CloseCode HandlerManager::process(
        const WebSocketConnectionPtr &wsConnPtr,
        const unsigned int &action,
        const Json::Value &request,
        Json::Value &response
) {
    auto handler(_handlerFactory.getHandler(action));
    CloseCode code = handler->fromJson(wsConnPtr, request, response);
    return code;
}

HandlerManager::HandlerManager() : _handlerFactory(HandlerFactory<MessageHandler>::instance()) {}
