//
// Created by Parti on 2021/2/4.
//

#include <plugins/Configurator.h>

using namespace tech::plugins;

void Configurator::initAndStart(const Json::Value &config) {
    if (config.isMember("auth_token_expire_time") && config["access_token_expire_time"].isUInt()) {
        _authTokenExpireTime = config["auth_token_expire_time"].asUInt();
    } else {
        LOG_ERROR << R"(Requires unsigned int value "auth_token_expire_time" in plugin Configurator's config')";
        abort();
    }
    if (config.isMember("access_token_expire_time") && config["access_token_expire_time"].isUInt()) {
        _accessTokenExpireTime = config["access_token_expire_time"].asUInt();
    } else {
        LOG_ERROR << R"(Requires unsigned int value "access_token_expire_time" in plugin Configurator's config')";
        abort();
    }
    LOG_INFO << "Configurator loaded.";
}

void Configurator::shutdown() {
    LOG_INFO << "Configurator shutdown.";
}

uint64_t Configurator::getAuthExpire() const {
    return _authTokenExpireTime;
}

uint64_t Configurator::getAccessExpire() const {
    return _accessTokenExpireTime;
}
