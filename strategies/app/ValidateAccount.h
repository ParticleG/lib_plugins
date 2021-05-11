//
// Created by Parti on 2021/4/10.
//

#pragma once

#include <models/Auth.h>
#include <models/Data.h>
#include <models/Info.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class ValidateAccount : public MessageHandler {
    public:
        ValidateAccount();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;

    private:
        drogon::orm::Mapper<drogon_model::Techmino::Auth> _authMapper;
        drogon::orm::Mapper<drogon_model::Techmino::Data> _dataMapper;
        drogon::orm::Mapper<drogon_model::Techmino::Info> _infoMapper;
    };
}
