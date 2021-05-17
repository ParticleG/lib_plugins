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
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void subscribe(const std::string &id, drogon::WebSocketConnectionPtr connection);

        void unsubscribe(const std::string &id, const drogon::WebSocketConnectionPtr &connection);

        void publish(const std::string &rid, const drogon::WebSocketConnectionPtr &connection, const std::string &message);

        Json::Value parseInfo() const;

    private:
        static std::shared_ptr<tech::structures::Chat> _getChat(const drogon::WebSocketConnectionPtr &connection);
    };
}

