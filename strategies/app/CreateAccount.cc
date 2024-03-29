//
// Created by Parti on 2021/4/10.
//

#include <mailio/message.hpp>
#include <mailio/smtp.hpp>
#include <plugins/Configurator.h>
#include <regex>
#include <strategies/actions.h>
#include <strategies/app/CreateAccount.h>
#include <utils/crypto.h>
#include <utils/misc.h>
#include <utils/websocket.h>

using namespace drogon;
using namespace drogon_model;
using namespace mailio;
using namespace tech::plugins;
using namespace tech::strategies;
using namespace tech::utils;
using namespace std;

CreateAccount::CreateAccount() :
        _authMapper(app().getDbClient()),
        _dataMapper(app().getDbClient()),
        _infoMapper(app().getDbClient()) {}

drogon::CloseCode CreateAccount::fromJson(
        const WebSocketConnectionPtr &wsConnPtr,
        const Json::Value &request,
        Json::Value &response
) {
    if (!(
            request.isMember("data") && request["data"].isObject() &&
            request["data"].isMember("email") && request["data"]["email"].isString() &&
            request["data"].isMember("password") && request["data"]["password"].isString() &&
            request["data"].isMember("username") && request["data"]["username"].isString()
    )) {
        response["type"] = "Warn";
        response["reason"] = "Wrong format: Requires string type 'email', 'password' and 'username' in 'data'";
    } else {
        bool sendSuccess = false;
        string email = request["data"]["email"].asString(),
                password = request["data"]["password"].asString(),
                username = request["data"]["username"].asString();
        LOG_DEBUG << websocket::fromJson(request);
        try {
            if (!_authMapper.findBy(Criteria(Techmino::Auth::Cols::_email, CompareOperator::EQ, email)).empty()) {
                response["type"] = "Warn";
                response["reason"] = "Email already registered";
            } else {
                Json::Value tempResponse;
                tempResponse["action"] = static_cast<int>(actions::App::validateAccount);
                tempResponse["type"] = "Self";
                tempResponse["data"]["message"] = "Successfully created account. Try sending email, please be patient...";
                wsConnPtr->send(websocket::fromJson(tempResponse));

                app().getDbClient()->execSqlSync(
                        "insert into auth (email, password, validated) "
                        "values ($1, crypt($2, gen_salt('bf', 10)), $3)",
                        email, password, false
                );
                auto configurator = app().getPlugin<Configurator>();
                auto tempAuth = _authMapper.findOne(Criteria(Techmino::Auth::Cols::_email, CompareOperator::EQ, email));
                tempAuth.setAuthToken(crypto::panama::generateKey());
                tempAuth.setAccessToken(crypto::panama::generateIV());
                _authMapper.update(tempAuth);
                Techmino::Info tempInfo;
                tempInfo.setId(tempAuth.getValueOfId());
                tempInfo.setEmail(email);
                tempInfo.setUsername(username);
                _infoMapper.insert(tempInfo);
                Techmino::Data tempData;
                tempData.setId(tempAuth.getValueOfId());
                tempData.setEmail(tempAuth.getValueOfEmail());
                _dataMapper.insert(tempData);

                Json::Value tempJson;
                tempJson["email"] = email;
                tempJson["password"] = password;
                tempJson["expires"] = misc::fromDate(configurator->getEmailExpire());

                u8string subject = u8"[Techmino] Email validation";

                message msg;
                msg.header_codec(message::header_codec_t::BASE64);
                msg.from(mail_address(configurator->getMailName(), configurator->getMailAddress()));
                msg.add_recipient(mail_address(username, email));
                msg.subject(string(subject.begin(), subject.end()));
                msg.content_transfer_encoding(mime::content_transfer_encoding_t::QUOTED_PRINTABLE);
                msg.content_type(message::media_type_t::TEXT, "html", "utf-8");
                msg.content(regex_replace(
                        misc::getFileString(configurator->getHtmlPath()),
                        regex(R"(\{\{LINK\}\})"),
                        "http://home.techmino.org/#/user/register?"
                        "id=" + to_string(tempAuth.getValueOfId()) +
                        "&code=" + crypto::panama::encrypt(
                                websocket::fromJson(tempJson),
                                tempAuth.getValueOfAuthToken(),
                                tempAuth.getValueOfAccessToken()
                        )
                ));
                smtps conn(configurator->getHostName(), configurator->getPort());
                conn.authenticate(configurator->getUsername(), configurator->getPassword(), smtps::auth_method_t::START_TLS);
                conn.submit(msg);

                response["action"] = static_cast<int>(actions::App::validateAccount);
                response["type"] = "Server";
                response["data"]["message"] = "Successfully sent email. Please check mailbox in 15 minutes";

                sendSuccess = true;
            }
        } catch (const orm::DrogonDbException &e) {
            LOG_ERROR << "error:" << e.base().what();
            response["type"] = "Error";
            response["reason"] = "Internal error";
            return CloseCode::kUnexpectedCondition;
        } catch (smtp_error &e) {
            LOG_ERROR << e.what();
            response["type"] = "Error";
            response["reason"] = e.what();
        } catch (dialog_error &e) {
            LOG_ERROR << e.what();
            response["type"] = "Error";
            response["reason"] = e.what();
        }
        if(!sendSuccess){
            try {
                auto tempAuth = _authMapper.findOne(Criteria(Techmino::Auth::Cols::_email, CompareOperator::EQ, email));
                _authMapper.deleteOne(tempAuth);
            } catch (const orm::DrogonDbException &e) {
                LOG_FATAL << "Remove unvalidated account failed";
            }
        }
    }
    return CloseCode::kNormalClosure;
}
