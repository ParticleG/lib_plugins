//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <models/Data.h>
#include <strategies/base/MessageHandler.h>

namespace tech::strategies {
    class UploadData : public MessageHandler {
    public:
        UploadData();

        drogon::CloseCode fromJson(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request,
                Json::Value &response
        ) override;

    private:
        drogon::orm::Mapper<drogon_model::Techmino::Data> _dataMapper;

        static void _uploadToSection(drogon_model::Techmino::Data &data, const unsigned int &section, std::string dataString);
    };
}
