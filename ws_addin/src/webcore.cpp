//
// Created by admin on 02.08.2022.
//

//#include <net.hpp>
#include "webcore.h"
//#include <boost/filesystem.hpp>

#ifdef UNIX
#include <unistd.h>
#endif

void WebCore::get_online_users(const variant_t &uuid_form) {

    if(!m_client->started())
        return;

    using json_nl = nlohmann::json;

    std::string uuid_form_ = std::get<std::string>(uuid_form);
    if(uuid_form_.empty())
        uuid_form_ = arcirk::uuids::nil_string_uuid();

    json_nl param = {
            {"table", true},
            {"uuid_form", uuid_form_}
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

    default_form = arcirk::uuids::nil_uuid();

    m_client = std::make_shared<websocket_client>(ctx, client_param);

    m_client->connect(client::client_events::wsMessage, (callback_message)std::bind(&WebCore::on_message, this, std::placeholders::_1));
    m_client->connect(client::client_events::wsStatusChanged, (callback_status)std::bind(&WebCore::on_status_changed, this, std::placeholders::_1));
    m_client->connect(client::client_events::wsError, (callback_error)std::bind(&WebCore::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_client->connect(client::client_events::wsConnect, (callback_connect)std::bind(&WebCore::on_connect, this));
    m_client->connect(client::client_events::wsClose, (callback_close)std::bind(&WebCore::on_stop, this));
    m_client->connect(client::client_events::wsSuccessfulAuthorization, (callback_successful_authorization)std::bind(&WebCore::on_successful_authorization, this));

    auto_reconnect = false;
    allow_delayed_authorization = false;

    AddProperty(L"Version", L"Версия", [&]() {
        auto s = std::string(Version);
        return std::make_shared<variant_t>(std::move(s));
    });

    AddProperty(L"url", L"url", [&]() {
        const std::string m_url = url_;
        return std::make_shared<variant_t>(std::move(m_url));
    });

    AddProperty(L"AutoReconnect", L"ВосстановитьСоединениеПриРазрыве", [&]() {
        const bool auto_reconnect_ = auto_reconnect;
        return std::make_shared<variant_t>(std::move(auto_reconnect_));
    }, [&](const variant_t& v){
        auto_reconnect = std::get<bool>(v);
    });
    AddProperty(L"AllowDelayedAuthorization", L"РазрешитьОтложеннуюАвторизацию", [&]() {
        const bool allow_delayed_authorization_ = allow_delayed_authorization;
        return std::make_shared<variant_t>(std::move(allow_delayed_authorization_));
    }, [&](const variant_t& v){
        allow_delayed_authorization = std::get<bool>(v);
    });
    AddProperty(L"DefaultForm", L"ОсновнаяФорма", [&]() {
        const std::string form_def = arcirk::uuids::uuid_to_string(default_form);
        return std::make_shared<variant_t>(std::move(form_def));
    }, [&](const variant_t& v){
        std::string def_form =  std::get<std::string>(v);
        default_form = arcirk::uuids::string_to_uuid(def_form);
    });
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
    AddMethod(L"GetOnlineUsers", L"АктивныеПользователи", this, &WebCore::get_online_users);
    AddMethod(L"CommandToClient", L"КомандаКлиенту", this, &WebCore::command_to_client);
    AddMethod(L"CommandToServer", L"КомандаСерверу", this, &WebCore::command_to_server);
    AddMethod(L"Sha1Hash", L"Sha1Hash", this, &WebCore::sha1_hash);
    AddMethod(L"SessionUuid", L"ИдентификаторСессии", this, &WebCore::session_uuid);
    AddMethod(L"SetParam", L"УстановитьПараметры", this, &WebCore::set_client_param);
    AddMethod(L"GetTableRowStructure", L"ПолучитьСтруктуруЗаписи", this, &WebCore::get_table_row_structure);
    AddMethod(L"GetServerCommands", L"ПолучитьСтруктуруКомандСервера", this, &WebCore::get_server_commands);
    AddMethod(L"GetMessages", L"ПолучитьИсториюСообщений", this, &WebCore::get_messages);
    AddMethod(L"SendMessage", L"ОтправитьСообщение", this, &WebCore::send_message);

}

std::string WebCore::get_server_commands() {

    using json_nl = nlohmann::json;
    using namespace arcirk::server;
    json_nl commands = {
            {enum_synonym(server_commands::ServerVersion), enum_synonym(server_commands::ServerVersion)},
            {enum_synonym(server_commands::ServerOnlineClientsList), enum_synonym(server_commands::ServerVersion)},
            {enum_synonym(server_commands::SetClientParam), enum_synonym(server_commands::ServerVersion)},
            {enum_synonym(server_commands::ServerConfiguration), enum_synonym(server_commands::ServerConfiguration)},
            {enum_synonym(server_commands::UserInfo), enum_synonym(server_commands::UserInfo)},
            {enum_synonym(server_commands::InsertOrUpdateUser), enum_synonym(server_commands::InsertOrUpdateUser)},
            {enum_synonym(server_commands::ExecuteSqlQuery), enum_synonym(server_commands::ExecuteSqlQuery)},
            {enum_synonym(server_commands::UpdateServerConfiguration), enum_synonym(server_commands::UpdateServerConfiguration)},
            {enum_synonym(server_commands::InsertToDatabaseFromArray), enum_synonym(server_commands::InsertToDatabaseFromArray)},
            {enum_synonym(server_commands::SetNewDeviceId), enum_synonym(server_commands::SetNewDeviceId)},
    };

    return commands.dump();
}

void WebCore::command_to_client(const variant_t &receiver, const variant_t &command, const variant_t &param) {
    if(!m_client->started())
        return;
    m_client->send_command_to_client(std::get<std::string>(receiver), std::get<std::string>(command), std::get<std::string>(param));
}

void WebCore::command_to_server(const variant_t &command, const variant_t &param, const variant_t &uuid_form) {

    if(!m_client->started())
        return;
    using json_nl = nlohmann::json;
    std::string command_ = std::get<std::string>(command);
    std::string uuid_form_ = std::get<std::string>(uuid_form);
    json_nl param_ = json_nl::parse(std::get<std::string>(param));
    if(param_.is_discarded())
        param_ = {};
    param_ += {"uuid_form", uuid_form_};
    m_client->send_command(command_, param_.dump());
}

void WebCore::send_message(const variant_t &receiver, const variant_t &message, const variant_t &param) {

    if(!m_client->started())
        return;

    std::string message_ = std::get<std::string>(message);
    std::string param_ = std::get<std::string>(param);
    std::string receiver_ = std::get<std::string>(receiver);

    m_client->send_message(message_, receiver_, param_);

}

void WebCore::set_client_param(const variant_t &userName, const variant_t &userHash, const variant_t &userUuid, const variant_t &appName) {
    if(!std::get<std::string>(userName).empty())
        client_param.user_name = std::get<std::string>(userName);
    if(!std::get<std::string>(userHash).empty())
        client_param.hash = std::get<std::string>(userHash);
    if(!std::get<std::string>(userUuid).empty())
        client_param.user_uuid = std::get<std::string>(userUuid);
    if(!std::get<std::string>(appName).empty())
        client_param.app_name = std::get<std::string>(appName);

    if(m_client){
        try {
            m_client->update_client_param(client_param);
        } catch (std::exception &e) {
            error(arcirk::to_utf("WebCore::set_client_param"), arcirk::to_utf(e.what()));
        }
    }
}

WebCore::~WebCore() {
    m_client->close(true);
}

std::string WebCore::get_table_row_structure(const variant_t &table_name) {
    using json_nl = nlohmann::json;
    std::string table = std::get<std::string>(table_name);
    json_nl enm_json = table;
    try {
        auto enm_val = enm_json.get<arcirk::database::tables>();
        return m_client->get_table_default_struct(enm_val);
    } catch (std::exception &ex) {
        error("WebCore::get_table_row_structure", ex.what());
    }
    return {};
}

void WebCore::emit(const std::string& command, const std::string &resp) {
    const std::string source_event = "WebSocketClient";
//    if(_is_source_event_uuid_form && !uuid_form.empty() && uuid_form != arcirk::uuids::nil_string_uuid()){
//        source_event = uuid_form;
//    }
    //this->CleanEventBuffer();
    long sz = this->GetEventBufferDepth();
    if(sz == 1)
        this->SetEventBufferDepth(sz + 1);
    //const long sz = 5;

    this->ExternalEvent(source_event, command, resp);
}

void WebCore::close(const variant_t &exit_base) {
    if (m_client)
    {
        if (m_client->started())
        {
            m_client->close(std::get<bool>(exit_base));
        }
    }
}

void WebCore::open(const variant_t &url) {
    if (m_client->started()) {
        error(arcirk::to_utf("WebCore::open"), arcirk::to_utf("Клиент уже запущен!"));
        return;
    }
    m_client->set_auto_reconnect(auto_reconnect);
    m_client->update_client_param(client_param);
    url_ = std::get<std::string>(url);
    m_client->open(arcirk::Uri::Parse(url_));
}

bool WebCore::started() {
    if(m_client)
        return m_client->started();
    else
        return false;
}

std::string WebCore::session_uuid() {
    if(!m_client)
        return arcirk::uuids::nil_string_uuid();

    if(!m_client->started())
        return arcirk::uuids::nil_string_uuid();
    else
        return arcirk::uuids::uuid_to_string(m_client->session_uuid());
}

void WebCore::on_connect(){
    auto response = arcirk::server::server_response();
    response.command = "on_connect";
    response.message = "ok";
    response.result = "success";
    response.uuid_form = uuids::uuid_to_string(default_form);
    response.version = ARCIRK_VERSION;
    std::string msg = arcirk::base64::base64_encode(to_string(pre::json::to_json(response)));
    emit(enum_synonym(arcirk::client::client_events::wsConnect), msg);
}

void WebCore::on_message(const std::string& message){
    using namespace arcirk::client;
    emit(enum_synonym(client_events::wsMessage), message);
}

void WebCore::on_stop(){
    auto response = arcirk::server::server_response();
    response.command = "on_stop";
    response.message = "ok";
    response.result = "success";
    response.uuid_form = uuids::uuid_to_string(default_form);
    response.version = ARCIRK_VERSION;
    std::string msg = arcirk::base64::base64_encode(to_string(pre::json::to_json(response)));
    emit(enum_synonym(arcirk::client::client_events::wsClose), msg);
}
void
WebCore::on_error(const std::string &what, const std::string &err, int code){
    auto response = arcirk::server::server_response();
    response.command = "on_error";
    response.message = err;
    response.result = "error";
    response.uuid_form = uuids::uuid_to_string(default_form);
    response.version = ARCIRK_VERSION;
    std::string msg = arcirk::base64::base64_encode(to_string(pre::json::to_json(response)));
    emit(enum_synonym(arcirk::client::client_events::wsError), msg);
}

void WebCore::on_status_changed(bool status){
    auto response = arcirk::server::server_response();
    response.command = "on_status_changed";
    response.message = "ok";
    response.result = status ? "true" : "false";
    response.uuid_form = uuids::uuid_to_string(default_form);
    response.version = ARCIRK_VERSION;
    std::string msg = arcirk::base64::base64_encode(to_string(pre::json::to_json(response)));
    emit(enum_synonym(arcirk::client::client_events::wsStatusChanged), msg);
}

std::string WebCore::sha1_hash(const variant_t &source) {
    std::string _source = std::get<std::string>(source);
    if(!_source.empty())
        return arcirk::get_sha1(_source);
    else
        return {};
}

void WebCore::error(const std::string& src, const std::string &msg) {

    AddError(ADDIN_E_FAIL, src, msg, false);
}

void WebCore::get_messages(const variant_t &sender, const variant_t &recipient, const variant_t &uuid_form) {

    nlohmann::json param_{
            {"sender", std::get<std::string>(sender)},
            {"recipient", std::get<std::string>(recipient)},
            {"uuid_form", std::get<std::string>(uuid_form)},
    };

    if(!m_client->started())
        return;

    m_client->send_command(arcirk::enum_synonym(server::server_commands::GetMessages), param_.dump());
}

void WebCore::on_successful_authorization() {
    auto response = arcirk::server::server_response();
    response.command = "on_successful_authorization";
    response.message = "ok";
    response.result = "success";
    response.uuid_form = uuids::uuid_to_string(default_form);
    response.version = ARCIRK_VERSION;
    std::string msg = arcirk::base64::base64_encode(to_string(pre::json::to_json(response)));
    emit(enum_synonym(arcirk::client::client_events::wsSuccessfulAuthorization), msg);
}
