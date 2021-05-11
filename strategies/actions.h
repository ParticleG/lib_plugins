//
// Created by Parti on 2021/4/9.
//

#pragma once

namespace tech::strategies::actions {
    enum class Prefix {
        app = 100,
        chat = 200,
        play = 300,
        stream = 400,
        user = 500,
    };

    enum class App {
        getAppVersion = 0,
        getNotice = 1,
        validateAccount = 2,
        getUserCount = 3,
    };
    enum class Chat {
        getChannelList = 0,
        enterChannel = 1,
        leaveChannel = 2,
        publishChatMessage = 3,
    };
    enum class Play {
        getRoomList = 0,
        createRoom = 1,
        enterRoom = 2,
        leaveRoom = 3,
        publishPlayMessage = 4,
        changeConfig = 5,
        changeReady = 6,
        pendingStart = 7,
        startGame = 8,
        endGame = 9,
    };
    enum class Stream {
        startStreaming = 0,
        endStreaming = 1,
        enterRoom = 2,
        leaveRoom = 3,
        publishDeathData = 4,
        publishStreamData = 5,
    };
    enum class User {
        getAccessToken = 0,
        getUserInfo = 1,
    };

    static int operator+(Prefix prefix, int action) {
        return static_cast<int>(prefix) + action;
    }

    static int operator+(Prefix prefix, App action) {
        return static_cast<int>(prefix) + static_cast<int>(action);
    }

    static int operator+(Prefix prefix, Chat action) {
        return static_cast<int>(prefix) + static_cast<int>(action);
    }

    static int operator+(Prefix prefix, Play action) {
        return static_cast<int>(prefix) + static_cast<int>(action);
    }

    static int operator+(Prefix prefix, Stream action) {
        return static_cast<int>(prefix) + static_cast<int>(action);
    }

    static int operator+(Prefix prefix, User action) {
        return static_cast<int>(prefix) + static_cast<int>(action);
    }
}
