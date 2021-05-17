//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <shared_mutex>
#include <structures/User.h>
#include <utils/websocket.h>

namespace tech::plugins {
    class UserManager : public drogon::Plugin<UserManager> {
    public:
        enum class MapType {
            chat,
            play,
            stream,
            user,
            all
        };

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void disconnect(const int64_t &uid, MapType mapType, const Json::Value& message);

        void subscribe(const int64_t &uid, drogon::WebSocketConnectionPtr connection, MapType mapType);

        std::vector<drogon::WebSocketConnectionPtr> unsubscribe(const int64_t &uid, MapType mapType);

        Json::Value parseInfo();

        Json::Value getCount();

    private:
        std::unordered_map<int64_t, drogon::WebSocketConnectionPtr> _chatConnMap{}, _playConnMap{}, _streamConnMap{}, _userConnMap{};
        mutable std::shared_mutex _sharedMutex;
    };
}
