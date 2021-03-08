//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>

namespace tech::plugins {
    class Configurator : public drogon::Plugin<Configurator> {
    public:
        Configurator() {}

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        uint64_t getAuthExpire() const;

        uint64_t getAccessExpire() const;

    private:
        uint64_t _authTokenExpireTime, _accessTokenExpireTime;
    };
}

