//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <shared_mutex>
#include <structures/App.h>
#include <utils/websocket.h>

namespace tech::plugins {
    class AppManager : public drogon::Plugin<AppManager> {
    public:
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void publish(const Json::Value& message);

        void subscribe(drogon::WebSocketConnectionPtr connection);

        void unsubscribe(const drogon::WebSocketConnectionPtr &connection);

        Json::Value parseInfo();

        Json::Value getCount();

    private:
        std::unordered_set<drogon::WebSocketConnectionPtr> _appConnSet{};
        mutable std::shared_mutex _sharedMutex;
    };
}
