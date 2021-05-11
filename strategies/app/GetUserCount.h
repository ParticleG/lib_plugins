//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <plugins/UserManager.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class GetUserCount : public MessageHandler {
    public:
        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    };
}
