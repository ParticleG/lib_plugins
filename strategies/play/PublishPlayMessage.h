//
// Created by Particle_G on 2021/3/04.
//

#pragma once

#include <plugins/PlayManager.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class PublishPlayMessage : public MessageHandler {
    public:
        PublishPlayMessage();
        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    private:
        std::shared_ptr<tech::plugins::PlayManager> _playManager;
    };
}
