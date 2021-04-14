//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <models/Message.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class GetNotice : public MessageHandler {
    public:
        GetNotice();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;

    private:
        drogon::orm::Mapper<drogon_model::Techmino::Message> _messageMapper;
    };
}
