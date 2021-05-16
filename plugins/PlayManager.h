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
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void subscribe(const std::string &rid, drogon::WebSocketConnectionPtr connection) override {}

        void subscribe(
                const std::string &rid,
                const std::string &password,
                const drogon::WebSocketConnectionPtr &connection
        );

        void unsubscribe(
                const std::string &rid,
                const drogon::WebSocketConnectionPtr &connection
        ) override;

        void publish(
                const std::string &rid,
                const uint64_t &action,
                Json::Value &&data,
                const drogon::WebSocketConnectionPtr &connection = nullptr,
                const uint64_t &excluded = 0
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
                const unsigned int &begin,
                const unsigned int &count
        ) const;

    private:
        static std::shared_ptr<tech::structures::Play> _getPlay(const drogon::WebSocketConnectionPtr &connection);

        void _checkReady(const std::string &rid);
    };
}
