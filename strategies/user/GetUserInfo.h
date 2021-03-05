//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <models/Auth.h>
#include <models/Info.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class GetUserInfo : public MessageHandler {
    public:
        GetUserInfo();
        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    private:
        drogon::orm::Mapper<drogon_model::Techmino::Info> _infoMapper;
    };
}
