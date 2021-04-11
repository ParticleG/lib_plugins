//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/user/GetAccessToken.h>
#include <structures/User.h>
#include <utils/crypto.h>
#include <utils/misc.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

GetAccessToken::GetAccessToken() : _authMapper(app().getDbClient()) {}

CloseCode GetAccessToken::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        auto auth = wsConnPtr->getContext<User>()->getAuth();
        auto *configurator = app().getPlugin<Configurator>();
        auth->setAccessToken(crypto::keccak(drogon::utils::getUuid()));
        auth->setAccessTokenExpireTime(misc::fromDate(configurator->getAccessExpire()));
        _authMapper.update(*auth);
        response["action"] = static_cast<int>(actions::User::getAccessToken);
        response["type"] = "Self";
        response["accessToken"] = auth->getValueOfAccessToken();
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
    return CloseCode::kNormalClosure;
}