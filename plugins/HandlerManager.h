//
// Created by Parti on 2021/2/19.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <strategies/base/MessageHandler.h>
#include <structures/HandlerFactory.h>
#include <utils/WebSocket.h>

namespace tech::plugins {
    class HandlerManager : public drogon::Plugin<HandlerManager> {
    public:
        HandlerManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        drogon::CloseCode process(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const tech::utils::WebSocket::Type &type,
                int action,
                const Json::Value &request,
                Json::Value &response
        );

    private:
        tech::structures::HandlerFactory<tech::strategies::MessageHandler> &_handlerFactory;
    };
}