//
// Created by admin on 02.08.2022.
//

#include <net.hpp>
#include "webcore.h"
#include <boost/filesystem.hpp>

#ifdef UNIX
#include <unistd.h>
#endif

void WebCore::get_online_users(){

    if(!m_client->started())
        return;

    using json_nl = nlohmann::json;

    json_nl param = {
            {"table", true}
    };

    std::string alias = json_nl(arcirk::server::server_commands::ServerOnlineClientsList).get<std::string>();
    std::string param_ = param.dump();
    m_client->send_command(alias, param_);
}

std::string WebCore::extensionName() {
    return "WebSocketClient";
}

WebCore::WebCore(){

    app_conf.ServerHost = arcirk::bIp::get_default_host("0.0.0.0", "192.168");
    app_conf.ServerPort = 8080;

    url_ = "ws://" + app_conf.ServerHost + ":" + std::to_string(app_conf.ServerPort);

    ssl::context ctx{ssl::context::tlsv12_client};

    client_param = client::client_param();
    client_param.app_name = application_name;
    client_param.user_uuid = arcirk::uuids::nil_string_uuid();
    client_param.user_name = "unknown";
    client_param.host_name = boost::asio::ip::host_name();
    client_param.session_uuid = arcirk::uuids::nil_string_uuid();
#ifdef _WINDOWS
    client_param.system_user = arcirk::to_utf(std::getenv("username"));
#else
    client_param.system_user = getlogin();
#endif

    m_client = std::make_shared<websocket_client>(ctx, client_param);

    m_client->connect(client::client_events::wsMessage, (callback_message)std::bind(&WebCore::on_message, this, std::placeholders::_1));
    m_client->connect(client::client_events::wsStatusChanged, (callback_status)std::bind(&WebCore::on_status_changed, this, std::placeholders::_1));
    m_client->connect(client::client_events::wsError, (callback_error)std::bind(&WebCore::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_client->connect(client::client_events::wsConnect, (callback_connect)std::bind(&WebCore::on_connect, this));
    m_client->connect(client::client_events::wsClose, (callback_close)std::bind(&WebCore::on_stop, this));


    AddProperty(L"Version", L"Версия", [&]() {
        auto s = std::string(Version);
        return std::make_shared<variant_t>(std::move(s));
    });

    AddProperty(L"url", L"url", [&]() {
        const std::string m_url = url_;
        return std::make_shared<variant_t>(std::move(m_url));
    });

//    AddProperty(L"DisablePublicNotify", L"ОтключитьПубличныеОповещения", [&]() {
//        const bool m_no_notify = no_notify;
//        return std::make_shared<variant_t>(std::move(m_no_notify));
//    }, [&](const variant_t& v){
//        no_notify = std::get<bool>(v);
//    });
//    AddProperty(L"CurrentRecipient", L"ТекущийПолучатель", [&]() {
//        const std::string m_current_recipient = _current_recipient;
//        return std::make_shared<variant_t>(std::move(m_current_recipient));
//    }, [&](const variant_t& v){
//        connect_to_recipient(v);
//    });
//    AddProperty(L"CurrentDocument", L"ТекущийДокумент", [&]() {
//        const std::string m_document_name = _document_name;
//        return std::make_shared<variant_t>(std::move(m_document_name));
//    }, [&](const variant_t& v){
//        set_document_name(v);
//    });

//    AddProperty(L"IsSourceEventUuidFrom", L"ИдентификаторФормыКакИсточникСобытия", [&]() {
//        const bool m_is_source_event_uuid_form = _is_source_event_uuid_form;
//        return std::make_shared<variant_t>(std::move(m_is_source_event_uuid_form));
//    }, [&](const variant_t& v){
//        _is_source_event_uuid_form = std::get<bool>(v);
//    });

    AddMethod(L"Open", L"Открыть", this, &WebCore::open);
    AddMethod(L"Close", L"Закрыть", this, &WebCore::close);
    AddMethod(L"Started", L"Запущен", this, &WebCore::started);
    AddMethod(L"GetOnlineUsers", L"ПолучитьПользователейВСети", this, &WebCore::get_online_users);
//    AddMethod(L"CommandToClient", L"КомандаКлиенту", this, &WebCore::command_to_client);
//    AddMethod(L"CommandToServer", L"КомандаСерверу", this, &WebCore::command_to_server);
//    AddMethod(L"GetSha1", L"ПолучитьSha1", this, &WebCore::getSha1);
//    AddMethod(L"SetAppName", L"УстановитьИмяПриложения", this, &WebCore::set_app_name);
    AddMethod(L"SessionUuid", L"ИдентификаторСессии", this, &WebCore::session_uuid);
//    AddMethod(L"SetJobData", L"УстановитьПараметрыРабочегоМеста", this, &WebCore::set_job_data);
}

WebCore::~WebCore() {

    if (m_client->started()) {
        m_client->close(true);
    }

}

void WebCore::emit(const std::string& command, const std::string &resp, const std::string &uuid_form) {
//    std::string source_event = "WebSocketClient";
//    if(_is_source_event_uuid_form && !uuid_form.empty() && uuid_form != arcirk::uuids::nil_string_uuid()){
//        source_event = uuid_form;
//    }
//    this->ExternalEvent(source_event, command, resp);
}


void WebCore::close(const variant_t &exit_base) {
//    if (client)
//    {
//        if (client->started())
//        {
//            client->close(std::get<bool>(exit_base));
//        }
//
//    }
}

void WebCore::open(const variant_t &url, const variant_t &user, const variant_t &userUuid, const variant_t &pwd) {

//    if (client->started()) {
//        return;
//    }
//
//    std::string m_url = std::get<std::string>(url);
//    if(!m_url.empty())
//        _url = m_url;
//
//    std::string m_usr = std::get<std::string>(user);
//    if(!m_usr.empty())
//        _user_name = m_usr;
//
//
//    std::string _u_uuid = std::get<std::string>(userUuid);
//    arcirk::uuids::is_valid_uuid(_u_uuid, _user_uuid);
//
//    std::string m_pwd = std::get<std::string>(pwd);
//    if(!m_pwd.empty())
//        _hash = arcirk::get_hash(_user_name, m_pwd);
//    else
//        _hash = arcirk::get_hash(_user_name, _u_uuid);
//
//    arcirk::Uri u = arcirk::Uri::Parse(_url);
//    _host = u.Host;
//    try{
//        _port = std::stoi(u.Port);
//    }catch(...){
//        //
//    }
//
//    client->client_details(_app_name, _user_name, _user_uuid, _hash);
//
//    client->open();

}

bool WebCore::started() {
    return m_client->started();
}


std::string WebCore::session_uuid() {
    if(!m_client->started())
        return arcirk::uuids::nil_string_uuid();
    else
        return arcirk::uuids::uuid_to_string(m_client->session_uuid());
}

//void WebCore::set_job_data(const variant_t &jobUuid, const variant_t &jobDescription) {
//    _job = std::get<std::string>(jobUuid);
//    _job_description = std::get<std::string>(jobDescription);
//}
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