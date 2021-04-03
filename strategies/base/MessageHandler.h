//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <drogon/WebSocketController.h>

namespace tech::strategies {
    class MessageHandler {
    public:
        virtual drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) = 0;

        virtual ~MessageHandler() = default;
    };
}