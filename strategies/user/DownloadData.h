//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <models/Data.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class DownloadData : public MessageHandler {
    public:
        DownloadData();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;

    private:
        drogon::orm::Mapper<drogon_model::Techmino::Data> _dataMapper;

        static void _downloadFromSection(const drogon_model::Techmino::Data &data, const unsigned int &section, Json::Value &result);
    };
}
