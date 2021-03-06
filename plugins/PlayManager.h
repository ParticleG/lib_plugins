//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <structures/BaseManager.h>
#include <structures/PlayRoom.h>

namespace tech::plugins {
    class PlayManager :
            public tech::structures::BaseManager<tech::structures::PlayRoom>,
            public drogon::Plugin<PlayManager> {
    public:
        virtual void initAndStart(const Json::Value &config) override;

        virtual void shutdown() override;

        uint64_t getCapacity(const std::string &type) const;

        void subscribe(
                const std::string &id,
                const std::string &password,
                const drogon::WebSocketConnectionPtr &connection
        );

        void unsubscribe(
                const std::string &id,
                const drogon::WebSocketConnectionPtr &connection
        ) override;

        void publish(
                const std::string &rid,
                const uint64_t &action,
                Json::Value &&data
        );

        void publish(
                const std::string &rid,
                const drogon::WebSocketConnectionPtr &connection,
                const uint64_t &action,
                Json::Value &&data
        );

        void publish(
                const std::string &rid,
                const drogon::WebSocketConnectionPtr &connection,
                const uint64_t &action,
                Json::Value &&data,
                const uint64_t &excluded
        );

        void changeConfig(
                const std::string &rid,
                std::string &&config,
                const drogon::WebSocketConnectionPtr &connection
        );

        void changeReady(
                const std::string &rid,
                const bool &ready,
                const drogon::WebSocketConnectionPtr &connection
        );

        Json::Value parseInfo(
                const std::string &type,
                const unsigned int &begin,
                const unsigned int &count
        ) const;

    private:
        std::unordered_map<std::string, uint64_t> _typesMap{};

        static Json::Value _parsePlayerInfo(
                const drogon::WebSocketConnectionPtr &connection,
                Json::Value &&data
        );

        void _checkReady(const std::string &rid);
    };
}
