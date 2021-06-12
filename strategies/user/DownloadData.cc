//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/user/DownloadData.h>
#include <structures/User.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace std;

DownloadData::DownloadData() : _dataMapper(app().getDbClient()) {}

CloseCode DownloadData::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("sections") && request["data"]["sections"].isArray()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires array type 'sections' in 'data'";
    } else {
        try {
            Json::Value result(Json::arrayValue);
            auto data = _dataMapper.findOne(
                    Criteria(
                            Techmino::Info::Cols::__id,
                            CompareOperator::EQ,
                            wsConnPtr->getContext<User>()->getAuth().getValueOfId()
                    )
            );
            for (const auto &item : request["data"]["sections"]) {
                _downloadFromSection(data, item.asUInt(), result);
            }
            response["action"] = static_cast<int>(actions::User::downloadData);
            response["type"] = "Self";
            response["data"]["sections"] = result;
        } catch (const orm::DrogonDbException &e) {
            LOG_ERROR << "error:" << e.base().what();
            response["type"] = "Error";
            response["reason"] = "Internal error";
            return CloseCode::kUnexpectedCondition;
        }

    }
    return CloseCode::kNormalClosure;
}

void DownloadData::_downloadFromSection(const drogon_model::Techmino::Data &data, const unsigned int &section, Json::Value &result) {
    bool isValid = true;
    Json::Value tempObject;
    tempObject["section"] = section;
    switch (section) {
        case 0:
            tempObject["data"] = data.getValueOfSectionA();
            break;
        case 1:
            tempObject["data"] = data.getValueOfSectionB();
            break;
        case 2:
            tempObject["data"] = data.getValueOfSectionC();
            break;
        case 3:
            tempObject["data"] = data.getValueOfSectionD();
            break;
        case 4:
            tempObject["data"] = data.getValueOfSectionE();
            break;
        case 5:
            tempObject["data"] = data.getValueOfSectionF();
            break;
        case 6:
            tempObject["data"] = data.getValueOfSectionG();
            break;
        case 7:
            tempObject["data"] = data.getValueOfSectionH();
            break;
        case 8:
            tempObject["data"] = data.getValueOfSectionI();
            break;
        case 9:
            tempObject["data"] = data.getValueOfSectionJ();
            break;
        case 10:
            tempObject["data"] = data.getValueOfSectionK();
            break;
        case 11:
            tempObject["data"] = data.getValueOfSectionL();
            break;
        case 12:
            tempObject["data"] = data.getValueOfSectionM();
            break;
        case 13:
            tempObject["data"] = data.getValueOfSectionN();
            break;
        case 14:
            tempObject["data"] = data.getValueOfSectionO();
            break;
        case 15:
            tempObject["data"] = data.getValueOfSectionP();
            break;
        default:
            isValid = false;
            break;
    }
    if (isValid) {
        result.append(tempObject);
    }
}
