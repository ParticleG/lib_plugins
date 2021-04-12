//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <plugins/ChatManager.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class EnterChannel : public MessageHandler {
    public:
        EnterChannel();
        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    };
}
