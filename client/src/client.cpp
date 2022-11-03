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
    }
}

void websocket_client::open(const Uri &url) {

    ssl::context ctx{ssl::context::tlsv12_client};
   // bool _ssl = url.Protocol == "wss";
    set_certificates(ctx);
    boost::thread(boost::bind(&websocket_client::start, this, url)).detach();

}

void websocket_client::start(arcirk::Uri &url) {

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
        }else{
            std::string basic_auth = "Token " + arcirk::base64::base64_encode(client_param_.user_name +":" + client_param_.password);
            state_->set_basic_auth_string(basic_auth);
        }

    }
    state_->connect(client::client_events::wsMessage, (callback_message)std::bind(&websocket_client::on_message, this, std::placeholders::_1));
    state_->connect(client::client_events::wsStatusChanged, (callback_status)std::bind(&websocket_client::on_status_changed, this, std::placeholders::_1));
    state_->connect(client::client_events::wsError, (callback_error)std::bind(&websocket_client::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    state_->connect(client::client_events::wsConnect,  (callback_connect)std::bind(&websocket_client::on_connect, this));
    state_->connect(client::client_events::wsClose, (callback_close)std::bind(&websocket_client::on_close, this));
    std::make_shared<resolver>(ioc, ctx_, state_)->run(url.Host.c_str(), url.Port.c_str(), _ssl);

    ioc.run();

    state_ = nullptr;

    log("websocket_client::start", "exit thread" );

    if(!_disable_notify_on_close)
        on_status_changed(false);
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
    send_command(server::synonym(server::server_commands::SetClientParam), param);

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

    bool isSetParam = message.find(server::synonym(server::server_commands::SetClientParam)) != std::string::npos;

    if(isSetParam){
        //парсим ответ если это установка параметров
        try {
            auto resp = pre::json::from_json<server::server_response>(message);
            if(resp.command == server::synonym(server::server_commands::SetClientParam)){
                client::client_param client_param;
                if(!resp.result.empty()){
                    std::string r = arcirk::base64::base64_decode(resp.result);
                    client_param = pre::json::from_json<client::client_param>(r);
                    client_param_.session_uuid = client_param.session_uuid;
                    boost::uuids::uuid uuid{};
                    if(arcirk::uuids::is_valid_uuid(client_param_.session_uuid, uuid))
                        state_->set_uuid_session(uuid);

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
}

void websocket_client::close(bool disable_notify) {

    if(!state_)
        return;
    _disable_notify_on_close = disable_notify;
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
