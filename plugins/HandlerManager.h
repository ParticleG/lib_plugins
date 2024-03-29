//
// Created by Parti on 2021/2/19.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <strategies/actions.h>
#include <strategies/base/MessageHandler.h>
#include <structures/HandlerFactory.h>
#include <utils/websocket.h>

namespace tech::plugins {
    class HandlerManager : public drogon::Plugin<HandlerManager> {
    public:
        HandlerManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        drogon::CloseCode process(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const tech::strategies::actions::Prefix &prefix,
                int action,
                const Json::Value &request,
                Json::Value &response
        );

    private:
        tech::structures::HandlerFactory<tech::strategies::MessageHandler> _handlerFactory;
    };
}