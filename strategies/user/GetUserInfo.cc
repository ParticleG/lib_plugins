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
        response["type"] = "Self";
        response["id"] = info.getValueOfId();
        response["email"] = info.getValueOfEmail();
        response["username"] = info.getValueOfUsername();

        if (detailed) {
            response["motto"] = info.getValueOfMotto();
            response["avatar"] = info.getValueOfAvatar();
        }
        return CloseCode::kNormalClosure;
    } catch (const UnexpectedRows &e) {
        LOG_WARN << e.what();
        response["type"] = "Warn";
        response["reason"] = "ID not found";
        return CloseCode::kNormalClosure;
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
}