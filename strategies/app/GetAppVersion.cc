//
// Created by Particle_G on 2021/2/17.
//

#include <strategies/app/GetAppVersion.h>

using namespace tech::strategies;
using namespace drogon_model;
using namespace drogon;
using namespace std;

GetAppVersion::GetAppVersion() : _appMapper(app().getDbClient()) {}

drogon::CloseCode GetAppVersion::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    bool newestOnly = request["data"].isMember("newestOnly") && request["data"]["newestOnly"].asBool();
    try {
        auto newestApp = _appMapper.orderBy(Techmino::App::Cols::_version_code, SortOrder::DESC).limit(1).findAll()[0];
        response["action"] = 0;
        response["type"] = "Self";
        response["newest"]["code"] = newestApp.getValueOfVersionCode();
        response["newest"]["name"] = newestApp.getValueOfVersionName();
        response["newest"]["content"] = newestApp.getValueOfVersionContent();
        if (!newestOnly) {
            auto leastApp = _appMapper.orderBy(Techmino::App::Cols::_version_code, SortOrder::DESC).limit(1)
                    .findBy(Criteria(Techmino::App::Cols::_compatible, CompareOperator::EQ, false))[0];
            response["least"]["code"] = leastApp.getValueOfVersionCode();
            response["least"]["name"] = leastApp.getValueOfVersionName();
            response["least"]["content"] = leastApp.getValueOfVersionContent();
        }
        return CloseCode::kNormalClosure;
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
}
