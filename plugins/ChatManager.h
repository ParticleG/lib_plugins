//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <structures/BaseManager.h>
#include <structures/ChatRoom.h>

namespace tech::plugins {
    class ChatManager :
            public tech::structures::BaseManager<tech::structures::ChatRoom>,
            public drogon::Plugin<ChatManager> {
    public:
        virtual void initAndStart(const Json::Value &config) override;

        virtual void shutdown() override;

        void subscribe(const std::string &id, drogon::WebSocketConnectionPtr connection) override;

        void unsubscribe(const std::string &id, const drogon::WebSocketConnectionPtr &connection) override;

        void publish(const std::string &rid, const drogon::WebSocketConnectionPtr &connection, const std::string &message);

        Json::Value parseInfo() const;

    private:
        static Json::Value _getPlayerInfo(const drogon::WebSocketConnectionPtr &connection, const std::string &message);

        static Json::Value _getPlayerInfo(const drogon::WebSocketConnectionPtr &connection);

    };
}

