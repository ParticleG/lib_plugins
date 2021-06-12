//
// Created by Parti on 2021/6/3.
//

#pragma once

#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class PublishMessage : public MessageHandler {
    public:
        PublishMessage();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;
    };
}
