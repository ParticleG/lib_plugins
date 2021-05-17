//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class GetStreamManagerStatus : public MessageHandler {
    public:
        GetStreamManagerStatus();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    };
}
