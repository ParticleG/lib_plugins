//
// Created by Parti on 2021/2/4.
//

#include <plugins/Configurator.h>

using namespace tech::plugins;

void Configurator::initAndStart(const Json::Value &config) {
    if (config.isMember("version") && config["version"].isObject() &&
        config["version"].isMember("name") && config["version"]["name"].isString()) {
        LOG_INFO << "Running Version: " << config["versionName"].asString();
    }
    if (config.isMember("tokenExpireTime") && config["tokenExpireTime"].isObject() &&
        config["tokenExpireTime"].isMember("auth") && config["tokenExpireTime"]["auth"].isUInt() &&
        config["tokenExpireTime"].isMember("access") && config["tokenExpireTime"]["access"].isUInt() &&
        config["tokenExpireTime"].isMember("email") && config["tokenExpireTime"]["email"].isUInt()) {
        _authTokenExpireTime = config["tokenExpireTime"]["auth"].asUInt();
        _accessTokenExpireTime = config["tokenExpireTime"]["access"].asUInt();
        _emailExpireTime = config["tokenExpireTime"]["email"].asUInt();
    } else {
        LOG_ERROR << R"(Requires unsigned int value "auth", "access" and "email" in plugin Configurator's config item "tokenExpireTime")";
        abort();
    }
    if (config.isMember("smtp") && config["smtp"].isObject() &&
        config["smtp"].isMember("username") && config["smtp"]["username"].isString() &&
        config["smtp"].isMember("password") && config["smtp"]["password"].isString() &&
        config["smtp"].isMember("mailAddress") && config["smtp"]["mailAddress"].isString() &&
        config["smtp"].isMember("mailName") && config["smtp"]["mailName"].isString() &&
        config["smtp"].isMember("port") && config["smtp"]["port"].isUInt() &&
        config["smtp"].isMember("hostName") && config["smtp"]["hostName"].isString()) {
        _username = config["smtp"]["username"].asString();
        _password = config["smtp"]["password"].asString();
        _mailAddress = config["smtp"]["mailAddress"].asString();
        _mailName = config["smtp"]["mailName"].asString();
        _port = config["smtp"]["port"].asUInt();
        _hostname = config["smtp"]["hostName"].asString();
    } else {
        LOG_ERROR << R"(Requires string value "username", "password", "mailAddress", "mailName", "hostName" and unsigned int value "port" in plugin Configurator's config item "smtp")";
        abort();
    }
    LOG_INFO << "Configurator loaded.";
}

void Configurator::shutdown() { LOG_INFO << "Configurator shutdown."; }

uint64_t Configurator::getAuthExpire() const { return _authTokenExpireTime; }

uint64_t Configurator::getAccessExpire() const { return _accessTokenExpireTime; }

uint64_t Configurator::getEmailExpire() const { return _emailExpireTime; }

std::string Configurator::getUsername() const { return _username; }

std::string Configurator::getPassword() const { return _password; }

std::string Configurator::getMailAddress() const { return _mailAddress; }

std::string Configurator::getMailName() const { return _mailName; }

uint32_t Configurator::getPort() const { return _port; }

std::string Configurator::getHostName() const { return _hostname; }
