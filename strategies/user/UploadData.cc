//
// Created by Particle_G on 2021/2/17.
//

#include <plugins/Configurator.h>
#include <strategies/actions.h>
#include <strategies/user/UploadData.h>
#include <structures/User.h>

using namespace drogon;
using namespace drogon_model;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::structures;
using namespace std;

UploadData::UploadData() : _dataMapper(app().getDbClient()) {}

CloseCode UploadData::fromJson(
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
            auto data = _dataMapper.findOne(
                    Criteria(
                            Techmino::Info::Cols::__id,
                            CompareOperator::EQ,
                            wsConnPtr->getContext<User>()->getAuth().getValueOfId()
                    )
            );
            for (const auto &item : request["data"]["sections"]) {
                _uploadToSection(data, item["section"].asUInt(), item["data"].asString());
            }
            _dataMapper.update(data);
            response["action"] = static_cast<int>(actions::User::uploadData);
            response["type"] = "Self";
        } catch (const orm::DrogonDbException &e) {
            LOG_ERROR << "error:" << e.base().what();
            response["type"] = "Error";
            response["reason"] = "Internal error";
            return CloseCode::kUnexpectedCondition;
        }

    }
    return CloseCode::kNormalClosure;
}

void UploadData::_uploadToSection(Techmino::Data &data, const unsigned int &section, string dataString) {
    switch (section) {
        case 0:
            data.setSectionA(move(dataString));
            break;
        case 1:
            data.setSectionB(move(dataString));
            break;
        case 2:
            data.setSectionC(move(dataString));
            break;
        case 3:
            data.setSectionD(move(dataString));
            break;
        case 4:
            data.setSectionE(move(dataString));
            break;
        case 5:
            data.setSectionF(move(dataString));
            break;
        case 6:
            data.setSectionG(move(dataString));
            break;
        case 7:
            data.setSectionH(move(dataString));
            break;
        case 8:
            data.setSectionI(move(dataString));
            break;
        case 9:
            data.setSectionJ(move(dataString));
            break;
        case 10:
            data.setSectionK(move(dataString));
            break;
        case 11:
            data.setSectionL(move(dataString));
            break;
        case 12:
            data.setSectionM(move(dataString));
            break;
        case 13:
            data.setSectionN(move(dataString));
            break;
        case 14:
            data.setSectionO(move(dataString));
            break;
        case 15:
            data.setSectionP(move(dataString));
            break;
        default:
            break;
    }
}
