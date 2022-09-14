//
// Created by admin on 02.08.2022.
//

//#ifdef _WINDOWS
//#include "stdafx.h"
//#endif // _WINDOWS

#include <net.hpp>
#include "webcore.h"
#include <boost/filesystem.hpp>

std::string WebCore::extensionName() {
    return "WebSocketClient";
}

WebCore::WebCore(){

    callback_connect _connect = std::bind(&WebCore::on_connect, this);
    callback_error _err = std::bind(&WebCore::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    callback_message _message = std::bind(&WebCore::on_message, this, std::placeholders::_1);
    callback_status _status = std::bind(&WebCore::on_status_changed, this, std::placeholders::_1);
    callback_close _on_close = std::bind(&WebCore::on_stop, this);

    client = new bClient();
    _port = 8080;
    _user_name = "unknown";
    _hash = "";
    _host = "localhost";
    _url = "ws://localhost";
    _user_uuid = arcirk::uuids::nil_uuid();

#ifdef _WINDOWS
    _sys_user = arcirk::to_utf(std::getenv("username"));
#endif
    no_notify = false;
    _is_source_event_uuid_form = false;

    AddProperty(L"Version", L"Версия", [&]() {
        auto s = std::string(Version);
        return std::make_shared<variant_t>(std::move(s));
    });

    AddProperty(L"url", L"url", [&]() {
        const std::string m_url = _url;
        return std::make_shared<variant_t>(std::move(m_url));
    });

    AddProperty(L"DisablePublicNotify", L"ОтключитьПубличныеОповещения", [&]() {
        const bool m_no_notify = no_notify;
        return std::make_shared<variant_t>(std::move(m_no_notify));
    }, [&](const variant_t& v){
        no_notify = std::get<bool>(v);
    });
    AddProperty(L"CurrentRecipient", L"ТекущийПолучатель", [&]() {
        const std::string m_current_recipient = _current_recipient;
        return std::make_shared<variant_t>(std::move(m_current_recipient));
    }, [&](const variant_t& v){
        connect_to_recipient(v);
    });
    AddProperty(L"CurrentDocument", L"ТекущийДокумент", [&]() {
        const std::string m_document_name = _document_name;
        return std::make_shared<variant_t>(std::move(m_document_name));
    }, [&](const variant_t& v){
        set_document_name(v);
    });

    AddProperty(L"IsSourceEventUuidFrom", L"ИдентификаторФормыКакИсточникСобытия", [&]() {
        const bool m_is_source_event_uuid_form = _is_source_event_uuid_form;
        return std::make_shared<variant_t>(std::move(m_is_source_event_uuid_form));
    }, [&](const variant_t& v){
        _is_source_event_uuid_form = std::get<bool>(v);
    });

    AddMethod(L"Open", L"Открыть", this, &WebCore::open);
    AddMethod(L"Close", L"Закрыть", this, &WebCore::close);
    AddMethod(L"Started", L"Запущен", this, &WebCore::started);
    AddMethod(L"GetOnlineUsers", L"ПолучитьПользователейВСети", this, &WebCore::get_online_users);
    AddMethod(L"CommandToClient", L"КомандаКлиенту", this, &WebCore::command_to_client);
    AddMethod(L"CommandToServer", L"КомандаСерверу", this, &WebCore::command_to_server);
    AddMethod(L"GetSha1", L"ПолучитьSha1", this, &WebCore::getSha1);
    AddMethod(L"SetAppName", L"УстановитьИмяПриложения", this, &WebCore::set_app_name);
    AddMethod(L"UuidSession", L"ИдентификаторСессии", this, &WebCore::uuid_session);
    AddMethod(L"SetJobData", L"УстановитьПараметрыРабочегоМеста", this, &WebCore::set_job_data);
}

WebCore::~WebCore() {

    if (client->started()) {
        client->close(true);
    }

    delete client;
    client = nullptr;
}

void WebCore::emit(const std::string& command, const std::string &resp, const std::string &uuid_form) {
    std::string source_event = "WebSocketClient";
    if(_is_source_event_uuid_form && !uuid_form.empty() && uuid_form != arcirk::uuids::nil_string_uuid()){
        source_event = uuid_form;
    }
    this->ExternalEvent(source_event, command, resp);
}

void WebCore::on_server_response(const std::string &resp) {
    if(resp == "exit_thread"){
        return;
    }

//    std::string result = ServerResponse::base64_decode(resp);
//
//    if(!result.empty()){
//        processServeResponse(result);
//    }
}

void WebCore::close(const variant_t &exit_base) {
    if (client)
    {
        if (client->started())
        {
            client->close(std::get<bool>(exit_base));
        }

    }
}

void WebCore::open(const variant_t &url, const variant_t &user, const variant_t &userUuid, const variant_t &pwd) {

    if (client->started()) {
        return;
    }

    std::string m_url = std::get<std::string>(url);
    if(!m_url.empty())
        _url = m_url;

    std::string m_usr = std::get<std::string>(user);
    if(!m_usr.empty())
        _user_name = m_usr;


    std::string _u_uuid = std::get<std::string>(userUuid);
    arcirk::uuids::is_valid_uuid(_u_uuid, _user_uuid);

    std::string m_pwd = std::get<std::string>(pwd);
    if(!m_pwd.empty())
        _hash = arcirk::get_hash(_user_name, m_pwd);
    else
        _hash = arcirk::get_hash(_user_name, _u_uuid);

    arcirk::Uri u = arcirk::Uri::Parse(_url);
    _host = u.Host;
    try{
        _port = std::stoi(u.Port);
    }catch(...){
        //
    }

    client->client_details(_app_name, _user_name, _user_uuid, _hash);

    client->open();

}

void WebCore::processServeResponse(const std::string &jsonResp) {

//    auto resp = new ServerResponse(jsonResp);
//
//    if(!resp->isParse){
//        delete resp;
//        return;
//    }
//
//    if(resp->result == "error"){
//
//        emit(resp->command, jsonResp);
//
//        if(resp->command == "set_client_param")
//            client->close();
//
//    }else{
//        if(resp->command == "connect_unknown_user")
//            return;
//        else if(resp->command == "client_leave" || resp->command == "client_join"){
//            if(no_notify)
//                return;
//            else{
//                if(!notify_apps.empty())
//                    if (notify_apps.find(_app_name) == std::string::npos)
//                        return;
//                emit(resp->command, jsonResp, resp->uuid_form);
//            }
//        }else if(resp->command == "command_to_client"){
//            if(!resp->client_command.empty())
//                emit(resp->client_command, resp->param, resp->uuid_form);
//            else
//                emit(resp->command, jsonResp, resp->uuid_form);
//        }else
//            emit(resp->command, jsonResp, resp->uuid_form);
//    }
//
//    delete resp;
}

bool WebCore::started() {
    return client->started();
}

//
void WebCore::get_online_users(const variant_t &appFilter, const variant_t& uuid_form) {

//    std::string filter = std::get<std::string>(appFilter);
//
//    auto param = arcirk::bJson();
//    param.set_object();
//    if(!filter.empty())
//        param.addMember("app_name", filter);
//    param.addMember("table", true);
//
//    client->send_command("get_active_users", std::get<std::string>(uuid_form), param.to_string());
}

void WebCore::command_to_client(const variant_t &recipient, const variant_t &command, const variant_t &param, const variant_t &uuid_form) {

    if(!client->started())
        return;

//    std::string _recipient = std::get<std::string>(recipient);
//    std::string _command = std::get<std::string>(command);
//    std::string _uuid_form = std::get<std::string>(uuid_form);
//    std::string _param = std::get<std::string >(param);
//
//    auto params = arcirk::bJson();
//    params.set_object();
//    params.addMember("command", _command);
//    params.addMember("param",  arcirk::base64_encode(_param));
//    _Value p = params.to_object();
//
//    auto obj = arcirk::bJson();
//    obj.set_object();
//    obj.addMember("uuid_agent", _recipient);
//    obj.addMember("uuid_client", client->get_app_uuid());
//    obj.addMember("param", p);
//
//
//    std::string arg = obj.to_string();
//    client->send_command("command_to_client", _uuid_form, arg);

}

void WebCore::command_to_server(const variant_t &command, const variant_t &param, const variant_t& uuid_form) {
    if(!client->started())
        return;
    std::string _command = std::get<std::string>(command);
    std::string _uuid_form = std::get<std::string>(uuid_form);
    std::string _param = std::get<std::string >(param);
    client->send_command(_command, _uuid_form, _param);
}

std::string WebCore::getSha1(const variant_t &source) {
    return arcirk::get_sha1(std::get<std::string>(source));
}

void WebCore::connect_to_recipient(const variant_t &recipient) {
//    _current_recipient = std::get<std::string>(recipient);
//    if(!client->started())
//        return;
//    auto obj = arcirk::bJson();
//    obj.set_object();
//    obj.addMember("uuid_session", client->get_app_uuid());
//    std::string param = obj.to_string(true);
//
//    command_to_client(recipient, "SetRecipient", param, "");
}

void WebCore::set_document_name(const variant_t &value) {

////#ifdef _WINDOWS
////    _document_name = boost::locale::conv::to_utf<char>( std::get<std::string>(value).c_str(), "Windows-1251");
////#endif
//    _document_name = std::get<std::string>(value);
//    client->set_document_name(_document_name);
//
//    if(!client->started())
//        return;
//
//    std::string _command = "set_document_name";
//    auto param = arcirk::bJson();
//    param.set_object();
//    param.addMember("document_name", _document_name);
//    client->send_command(_command, "", param.to_string());
}

void WebCore::set_app_name(const variant_t &value) {
    _app_name = std::get<std::string>(value);
}

std::string WebCore::uuid_session() {
    if(!client->started())
        return arcirk::uuids::nil_string_uuid();
    else
        return arcirk::uuids::uuid_to_string(client->session_uuid());
}

void WebCore::set_job_data(const variant_t &jobUuid, const variant_t &jobDescription) {
    _job = std::get<std::string>(jobUuid);
    _job_description = std::get<std::string>(jobDescription);
}
void WebCore::on_connect(){
    std::cout << "websocket on_connect" << std::endl;
}

void WebCore::on_message(const std::string& message){
    std::cout << "websocket on_message: " << message << std::endl;
}

void WebCore::on_stop(){
    std::cout << "websocket on_stop" << std::endl;
}
void
WebCore::on_error(const std::string &what, const std::string &err, int code){
    std::cerr << what << "(" << code << "): " << arcirk::local_8bit(err) << std::endl;
}

void WebCore::on_status_changed(bool status){
    std::cout << "websocket on_status_changed: " << status << std::endl;
}