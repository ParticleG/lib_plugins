//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <unordered_set>

namespace tech::plugins {
    class Configurator : public drogon::Plugin<Configurator> {
    public:
        Configurator() = default;

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        [[nodiscard]] bool isSuperusers(const int64_t &uid) const;

        [[nodiscard]] uint64_t getAuthExpire() const;

        [[nodiscard]] uint64_t getAccessExpire() const;

        [[nodiscard]] uint64_t getEmailExpire() const;

        [[nodiscard]] std::string getHtmlPath() const;

        [[nodiscard]] std::string getUsername() const;

        [[nodiscard]] std::string getPassword() const;

        [[nodiscard]] std::string getMailAddress() const;

        [[nodiscard]] std::string getMailName() const;

        [[nodiscard]] uint32_t getPort() const;

        [[nodiscard]] std::string getHostName() const;

    private:
        uint32_t _port{};
        uint64_t _authTokenExpireTime{}, _accessTokenExpireTime{}, _emailExpireTime{};
        std::string _htmlPath{}, _username{}, _password{}, _mailAddress{}, _mailName{}, _hostname{};
        std::unordered_set<int64_t> _superusers{};
    };
}

