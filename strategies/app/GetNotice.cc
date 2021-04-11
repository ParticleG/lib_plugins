//
// Created by Particle_G on 2021/2/17.
//

#include <strategies/actions.h>
#include <strategies/app/GetNotice.h>

using namespace tech::strategies;
using namespace drogon_model;
using namespace drogon;
using namespace std;

tech::strategies::GetNotice::GetNotice() : _messageMapper(app().getDbClient()) {}

drogon::CloseCode GetNotice::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    try {
        auto newestMessage = _messageMapper.orderBy(Techmino::Message::Cols::_id, SortOrder::DESC)
                .findBy(Criteria(Techmino::Message::Cols::_type, CompareOperator::EQ, "notice"))[0];
        response["action"] = static_cast<int>(actions::App::getNotice);
        response["type"] = "Self";
        response["notice"] = newestMessage.getValueOfContent();
        return CloseCode::kNormalClosure;
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "error:" << e.base().what();
        response["type"] = "Error";
        response["reason"] = "Internal error";
        return CloseCode::kUnexpectedCondition;
    }
}
