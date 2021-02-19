//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <structures/Player.h>
#include <structures/Room.h>

namespace tech ::structures {
    class ChatManager : public drogon::Plugin<ChatManager> {
    public:
        ChatManager() {}

        virtual void initAndStart(const Json::Value &config) override;

        virtual void shutdown() override;

        std::string getID() const;

        drogon::SubscriberID joinChat(const tech::structures::Room::MessageHandler &handler, const std::shared_ptr<tech::structures::Player> &player);

        void quitChat(drogon::SubscriberID playerID);

        void chat(const std::string &message);

        uint64_t chatCount();

    private:
        std::string _roomID;
        std::unique_ptr<tech::structures::Room> _chattingRoom;
        mutable std::shared_mutex _sharedMutex;
    };
}

