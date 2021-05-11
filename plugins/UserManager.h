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
        typedef std::unordered_map<int64_t, drogon::WebSocketConnectionPtr> connMap;

        enum class MapType {
            chat,
            play,
            stream,
            user
        };

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void subscribe(drogon::WebSocketConnectionPtr connection, MapType mapType);

        void unsubscribe(const drogon::WebSocketConnectionPtr &connection, MapType mapType);

        Json::Value getCount();

    private:
        std::unordered_map<MapType,connMap>_connMapMap;
        mutable std::shared_mutex _sharedMutex;
    };
}
