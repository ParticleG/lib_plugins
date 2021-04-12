//
// Created by Particle_G on 2021/3/04.
//

#pragma once

#include <plugins/StreamManager.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class PublishStreamData : public MessageHandler {
    public:
        PublishStreamData();
        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    };
}
