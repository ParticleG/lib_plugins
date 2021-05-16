//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <structures/BaseManager.h>
#include <structures/StreamRoom.h>

namespace tech ::plugins {
    class StreamManager :
            public tech::structures::BaseManager<tech::structures::StreamRoom>,
            public drogon::Plugin<StreamManager> {
    public:
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;


        void subscribe(const std::string &id, drogon::WebSocketConnectionPtr connection) override;

        void unsubscribe(const std::string &rid, const drogon::WebSocketConnectionPtr &connection) override;

        void publish(
                const std::string &rid,
                const uint64_t &action,
                Json::Value &&data,
                const drogon::WebSocketConnectionPtr &connection,
                const uint64_t &excluded = 0
        );

        Json::Value parseInfo() const;

    private:
        static std::shared_ptr<tech::structures::Stream> _getStream(const drogon::WebSocketConnectionPtr &connection);

        void _checkReady(const std::string &rid);

        void _checkFinished(const std::string &rid);
    };
}

