//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class GetPlayManagerStatus : public MessageHandler {
    public:
        GetPlayManagerStatus();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    };
}
