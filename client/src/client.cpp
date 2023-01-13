#include "../include/client.hpp"
#include <common/root_certificates.hpp>
#include "../include/session.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>

websocket_client::websocket_client(ssl::context& ctx, client::client_param &client_param)
: ctx_(ctx)
{
    state_ = nullptr;
    client_param_ = client_param;
    _disable_notify_on_close = false;
    auto_reconnect_ = false;
    timer_is_run = false;
    closed_by_user = false;
}

void websocket_client::connect(const client::client_events &event, const client::callbacks &f) {
    if(event == client::client_events::wsClose){
        m_data.on_close= boost::get<callback_close>(f);
    }else if(event == client::client_events::wsConnect){
        m_data.on_connect = boost::get<callback_connect>(f);
    }else if(event == client::client_events::wsError){
        m_data.on_error = boost::get<callback_error>(f);
    }else if(event == client::client_events::wsMessage){
        m_data.on_message = boost::get<callback_message>(f);
    }else if(event == client::client_events::wsStatusChanged){
        m_data.on_status_changed = boost::get<callback_status>(f);
    }else if(event == client::client_events::wsSuccessfulAuthorization){
        m_data.on_successful_authorization = boost::get<callback_successful_authorization>(f);
    }
}

void websocket_client::set_auto_reconnect(bool value) {
    auto_reconnect_ = value;
}

void websocket_client::open(const Uri &url) {

    ssl::context ctx{ssl::context::tlsv12_client};
   //bool _ssl = url.Protocol == "wss";
    set_certificates(ctx);

    boost::thread(boost::bind(&websocket_client::start, this, url)).detach();

}

void websocket_client::start(arcirk::Uri &url) {

    closed_by_user = false; //не включать таймер если сокет закрыт пользователем
    url_ = url.Url;
    boost::asio::io_context ioc;
    bool _ssl = url.Protocol == "wss";
    set_certificates(ctx_);
    state_ = boost::make_shared<shared_state>();
    if(!url.BasicAuth.empty())
        state_->set_basic_auth_string(url.BasicAuth);
    else
    {
        if(client_param_.hash.empty()){
            if(!client_param_.password.empty()){
                std::string basic_auth = "Basic " + arcirk::base64::base64_encode(client_param_.user_name +":" + client_param_.password);
                state_->set_basic_auth_string(basic_auth);
            }
        }
//        else{
//            std::string basic_auth = "Token " + client_param_.hash;
//            state_->set_basic_auth_string(basic_auth);
//        }

    }

    state_->connect(client::client_events::wsMessage, (callback_message)std::bind(&websocket_client::on_message, this, std::placeholders::_1));
    state_->connect(client::client_events::wsStatusChanged, (callback_status)std::bind(&websocket_client::on_status_changed, this, std::placeholders::_1));
    state_->connect(client::client_events::wsError, (callback_error)std::bind(&websocket_client::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    state_->connect(client::client_events::wsConnect,  (callback_connect)std::bind(&websocket_client::on_connect, this));
    state_->connect(client::client_events::wsClose, (callback_close)std::bind(&websocket_client::on_close, this));
    state_->connect(client::client_events::wsSuccessfulAuthorization, (callback_successful_authorization)std::bind(&websocket_client::on_successful_authorization, this));
    std::make_shared<resolver>(ioc, ctx_, state_)->run(url.Host.c_str(), url.Port.c_str(), _ssl);

    ioc.run();

    state_ = nullptr;

    log("websocket_client::start", "exit thread" );

    if(!_disable_notify_on_close){
        on_status_changed(false);
    }

}

void websocket_client::set_certificates(ssl::context& ctx) {

    using namespace boost::filesystem;

    if(!exists(certificate_file_))
        return;

    std::string _cert;
    std::ifstream c_in(certificate_file_.string());
    std::ostringstream c_oss;
    c_oss << c_in.rdbuf();
    _cert = c_oss.str();

    if(_cert.empty()){
        fail("websocket_client::set_certificates", "error read certificate files");
        return;
    }

    load_root_default_certificates(ctx);
}

void websocket_client::set_certificate_file(const std::string &file) {
    certificate_file_ = file;
}

void websocket_client::on_close() {
    if(m_data.on_close)
        m_data.on_close();
}

void websocket_client::on_connect() {

    std::string param = to_string(pre::json::to_json(client_param_)) ;
    send_command(enum_synonym(server::server_commands::SetClientParam), param);

    if(m_data.on_connect)
        m_data.on_connect();

    on_status_changed(true);
}

void websocket_client::on_error(const std::string &what, const std::string &err, int code) {
    if(m_data.on_error)
        m_data.on_error(what, err, code);
}

std::string websocket_client::base64_to_string(const std::string &base64str) const {
    try {
        return arcirk::base64::base64_decode(base64str);
    } catch (std::exception &e) {
        return base64str;
    }
}

void websocket_client::on_message(const std::string &message) {

    if(message.empty())
        return;

    bool isSetParam = message.find(enum_synonym(server::server_commands::SetClientParam)) != std::string::npos;

    if(isSetParam){
        //парсим ответ если это установка параметров
        try {
            auto resp = pre::json::from_json<server::server_response>(message);
            resp.version = ARCIRK_VERSION;
            if(resp.command == enum_synonym(server::server_commands::SetClientParam)){
                client::client_param client_param;
                if(!resp.result.empty()){
                    //std::string r = arcirk::base64::base64_decode(resp.result);
                    std::string r = arcirk::base64::base64_decode(resp.param);
                    client_param = pre::json::from_json<client::client_param>(r);
                    client_param_.session_uuid = client_param.session_uuid;
                    boost::uuids::uuid uuid{};
                    if(arcirk::uuids::is_valid_uuid(client_param_.session_uuid, uuid))
                        state_->set_uuid_session(uuid);
                    if(resp.message == "failed authorization")
                        on_error(resp.command, "failed authorization", 0);
                    else
                        on_successful_authorization();
                    log("websocket_client::on_message", resp.command + ": session uuid " +  client_param_.session_uuid + ": user uuid " + client_param_.user_uuid);
                    return;
                }
            }
        } catch (std::exception &e) {
            return fail("websocket_client::on_message", e.what(), false);
        }
    }

    if(m_data.on_message)
        m_data.on_message(message);
}

void websocket_client::on_status_changed(bool status) {
    if(m_data.on_status_changed)
        m_data.on_status_changed(status);
    if(auto_reconnect_ && !timer_is_run && !status && !closed_by_user){
        boost::thread(boost::bind(&websocket_client::start_reconnect, this, &timer_is_run)).detach();
    }
}

void websocket_client::close(bool disable_notify) {

    if(!state_)
        return;
    _disable_notify_on_close = disable_notify;
    closed_by_user = true;
    state_->close(disable_notify);
}

void websocket_client::send_message(const std::string &message) {
    if(!state_)
        return;
    if(message.empty())
        return;
    auto const ss = boost::make_shared<std::string const>(message);
    state_->send(ss);
}

void websocket_client::send_message(const std::string &message, const std::string &receiver, const std::string &param) {

    if(receiver.empty() || message.empty()){
        fail("websocket_client::send_message", "Не верные параметры сообщения!");
        return;
    }

    std::string msg = "msg ";
    msg.append(message + " ");
    msg.append(receiver);
    if(!param.empty()){
        msg.append(" ");
        msg.append(param);
    }

    auto const ss = boost::make_shared<std::string const>(std::move(msg));
    state_->send(ss);

}

void websocket_client::send_command(const std::string &command, const std::string &param) {
    if(!state_)
        return;
    state_->command_to_server(command, param);
}
void websocket_client::send_command_to_client(const std::string &receiver, const std::string &command,
                                              const std::string &param) {
    if(!state_)
        return;

    if(command.empty()){
        fail("websocket_client::send_command_to_client", "Не корректная команда!");
        return;
    }

    boost::uuids::uuid uuid{};
    if(!uuids::is_valid_uuid(receiver, uuid)){
        fail("websocket_client::send_command_to_client", "Не корректный идентификатор получателя!");
        return;
    }else{
        state_->command_to_client(receiver, command, param);
    }

}
bool websocket_client::started() {

    if(!state_)
        return false;

    return true;

}

boost::uuids::uuid websocket_client::session_uuid() const {
    if(!state_)
        return arcirk::uuids::nil_uuid();

    return state_->session_uuid();
}

void websocket_client::update_client_param(client::client_param& client_param) {
    if(started())
        throw std::exception("Клиент запущен. Изменение параметров запрещено.");

    if(!client_param.app_name.empty())
        client_param_.app_name = client_param.app_name;
    if(!client_param.user_name.empty())
        client_param_.user_name = client_param.user_name;
    if(!client_param.user_uuid.empty())
        client_param_.user_uuid = client_param.user_uuid;
    if(!client_param.hash.empty())
        client_param_.hash = client_param.hash;
    if(!client_param.password.empty())
        client_param_.password = client_param.password;

}

//const boost::system::error_code& ec,
void
websocket_client::check_connection(
                                   boost::asio::steady_timer* tmr)
{
    if(started()){
        tmr->expires_at((steady_timer::time_point::max)());
        tmr->cancel();
        log("websocket_client::check_connection", "stop check connection");
    }else{
        if (tmr->expiry() <= steady_timer::clock_type::now())
        {
            log("websocket_client::check_connection", "timer start");
            tmr->expires_after(std::chrono::seconds(60));
            tmr->async_wait(boost::bind(&websocket_client::check_connection, this, tmr));
            open(arcirk::Uri::Parse(url_));
        }
    }

}

void websocket_client::start_reconnect(bool* is_run) {

    if(*is_run)
        return;
    *is_run = true;
    log("websocket_client::start_reconnect", "timer start");
    net::io_context ioc_parent;
    net::steady_timer connection_timer(ioc_parent);
    connection_timer.expires_after(std::chrono::seconds(60));
    connection_timer.async_wait(boost::bind(&websocket_client::check_connection, this, &connection_timer));
    ioc_parent.run();
    log("websocket_client::start_reconnect", "timer stop");
    *is_run = false;
}

std::string websocket_client::get_table_default_struct(arcirk::database::tables table) {

    using namespace arcirk::database;
    switch (table) {
        case tbUsers:{
            auto usr_info = user_info();
            usr_info.ref = arcirk::uuids::nil_string_uuid();
            usr_info.parent = arcirk::uuids::nil_string_uuid();
            usr_info.is_group = 0;
            usr_info.deletion_mark = 0;
            std::string usr_info_json = to_string(pre::json::to_json(usr_info));
            return usr_info_json;
        }
        case tbMessages:{
            auto tbl = messages();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            tbl.content_type ="Text";
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbOrganizations:{
            auto tbl = organizations();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbSubdivisions:{
            auto tbl = subdivisions();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbWarehouses:{
            auto tbl = warehouses();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbPriceTypes:{
            auto tbl = price_types();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbWorkplaces:{
            auto tbl = workplaces();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            tbl.server = arcirk::uuids::nil_string_uuid();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbDevices:{
            auto tbl = devices();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            tbl.deviceType = "Desktop";
            tbl.address = "127.0.0.1";
            tbl.workplace = arcirk::uuids::nil_string_uuid();
            tbl.price = arcirk::uuids::nil_string_uuid();
            tbl.warehouse = arcirk::uuids::nil_string_uuid();
            tbl.subdivision = arcirk::uuids::nil_string_uuid();
            tbl.organization = arcirk::uuids::nil_string_uuid();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbDocumentsTable: {
            auto tbl = document_table();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            tbl.price = 0;
            tbl.quantity = 0;
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tbDocuments: {
            auto tbl = documents();
            tbl.ref = arcirk::uuids::nil_string_uuid();
            tbl.device_id = arcirk::uuids::nil_string_uuid();
            tbl.date = date_to_seconds();
            std::string tbl_json = to_string(pre::json::to_json(tbl));
            return tbl_json;
        }
        case tables_INVALID:{
            break;
        }
    }

    return {};
}

void websocket_client::on_successful_authorization() {
    if(m_data.on_successful_authorization)
        m_data.on_successful_authorization();
}
