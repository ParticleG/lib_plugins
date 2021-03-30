//
// Created by Particle_G on 2021/2/17.
//

#include <strategies/user/GetUserInfo.h>
#include <structures/User.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::strategies;
using namespace tech::structures;
using namespace std;

GetUserInfo::GetUserInfo() : _infoMapper(app().getDbClient()) {}

drogon::CloseCode GetUserInfo::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    int64_t id = wsConnPtr->getContext<User>()->getInfo()->getValueOfId();
    bool detailed = false;
    if (request.isMember("data") && request["data"].isObject()) {
        if (request["data"].isMember("id") && request["data"]["id"].isInt()) {
            id = request["data"]["id"].asInt64();
        }
        if (request["data"].isMember("detailed") && request["data"]["detailed"].isBool()) {
            detailed = request["data"]["detailed"].asBool();
        }
    }
    try {
        auto info = _infoMapper.findOne(Criteria(Techmino::Info::Cols::__id, CompareOperator::EQ, id));
        response["action"] = 1;
        response["message"] = "OK";
        response["id"] = info.getValueOfId();
        response["email"] = info.getValueOfEmail();
        response["username"] = info.getValueOfUsername();

        if (detailed) {
            response["motto"] = info.getValueOfMotto();
            response["avatar"] = info.getValueOfAvatar();
        }
        return CloseCode::kNone;
    } catch (const UnexpectedRows &e) {
        response["message"] = "ID not found";
        response["reason"] = e.what();
        return CloseCode::kNone;
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["message"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
}