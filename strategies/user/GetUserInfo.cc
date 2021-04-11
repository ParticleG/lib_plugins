//
// Created by Particle_G on 2021/2/17.
//

#include <strategies/actions.h>
#include <strategies/user/GetUserInfo.h>
#include <structures/User.h>
#include <utils/crypto.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::strategies;
using namespace tech::structures;
using namespace tech::utils;
using namespace std;

GetUserInfo::GetUserInfo() : _infoMapper(app().getDbClient()) {}

drogon::CloseCode GetUserInfo::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    int64_t id = wsConnPtr->getContext<User>()->getInfo()->getValueOfId();
    std::string hash;
    if (request.isMember("data") && request["data"].isObject()) {
        if (request["data"].isMember("uid") && request["data"]["uid"].isInt()) {
            id = request["data"]["uid"].asInt64();
        }
        if (request["data"].isMember("hash") && request["data"]["hash"].isString()) {
            hash = request["data"]["hash"].asString();
        }
    }
    try {
        auto info = _infoMapper.findOne(Criteria(Techmino::Info::Cols::__id, CompareOperator::EQ, id));
        response["action"] = static_cast<int>(actions::User::getUserInfo);
        response["type"] = "Self";
        response["data"]["uid"] = info.getValueOfId();
        response["data"]["username"] = info.getValueOfUsername();
        response["data"]["motto"] = info.getValueOfMotto();

        if (!hash.empty() && hash != crypto::blake2b(info.getValueOfAvatar())) {
            response["data"]["avatar"] = crypto::blake2b(info.getValueOfAvatar());
        }
    } catch (const UnexpectedRows &e) {
        LOG_WARN << e.what();
        response["type"] = "Warn";
        response["reason"] = "ID not found";
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
    return CloseCode::kNormalClosure;
}