//
// Created by arcady on 13.07.2021.
//

#include "net.hpp"
#include "arcirk.hpp"
#include "../include/client.hpp"
#include <boost/locale.hpp>
#include <boost/locale/generator.hpp>
#include "common/root_certificates.hpp"

#include "../include/websocket_client.hpp"
#include "../include/shared_state.hpp"
#include <boost/bind.hpp>

using namespace arcirk;

ws_client::ws_client(net::io_context &io_context, client::ClientParam& client_param)
: ioc(io_context)
, param(client_param)
{
    set_user_name("anonymous");
    _app_name = "unknown";

    m_data_private = client::bClientData();
    m_data_private.on_error = std::bind(&ws_client::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_data_private.on_close = std::bind(&ws_client::on_stop, this);
    m_data_private.on_connect = std::bind(&ws_client::on_connect, this);
    m_data_private.on_status_changed = std::bind(&ws_client::on_status_changed, this, std::placeholders::_1);
    m_data_private.on_message = std::bind(&ws_client::on_message, this, std::placeholders::_1);

}

void
ws_client::
send(const std::string &message, const boost::uuids::uuid &recipient, const boost::uuids::uuid &uuid_form)
{
//    std::string _sub_user_uuid = sub_user_uuid;
//    if (_sub_user_uuid.empty())
//        _sub_user_uuid = "00000000-0000-0000-0000-000000000000";
//
//    boost::uuids::uuid  uuid_channel = arcirk::uuids::string_to_uuid(_sub_user_uuid);

//    std::string msg;

//    if (is_cmd){
//        msg.append("cmd ");
//    } else{
//        msg.append("msg " + _sub_user_uuid + " ");
//    }

//    auto _msg = ws_message();
//    _msg.message().uuid = get_uuid();
//    _msg.message().message = message;
//    _msg.message().name = get_user_name();
//    _msg.message().uuid_channel = uuid_channel;
//    _msg.message().app_name = get_app_name();
//    _msg.message().command = command;
//    _msg.message().uuid_form = arcirk::string_to_uuid(uuid_form, false);
//    _msg.message().uuid_user = get_user_uuid();
//    _msg.message().object_name = objectName;
//    if (msg_ref.empty())
//        _msg.message().msg_ref = arcirk::random_uuid();
//    else
//        _msg.message().msg_ref = msg_ref;
//
//    msg.append(_msg.get_json(true));

//    auto const ss = boost::make_shared<std::string const>(std::move(message));
//
//    std::vector<boost::weak_ptr<session_base>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_){
//            if(p->is_ssl()) {
//                auto sess = (session *) p;
//                v.emplace_back(sess->weak_from_this());
//            }else{
//                auto sess = (session_ssl *) p;
//                v.emplace_back(sess->weak_from_this());
//            }
//        }

//    }
//    std::vector<boost::weak_ptr<session_ssl>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_){
//            auto sess = (session_ssl *) p;
//            v.emplace_back(sess->weak_from_this());
//        }
//
//    }
//    for(auto const& wp : v)
//        if(auto sp = wp.lock()){
//            try {
//                sp->send(ss);
//            } catch (std::exception &e) {
//                std::cerr << e.what() << std::endl;
//            }
//
//        }


}

void
ws_client::
on_connect(){

    std::cout << "ws_client::on_connect" << std::endl;

//    std::lock_guard<std::mutex> lock(mutex_);
//    sessions_.insert(sess);
//
//    if(_on_connect){
//        _on_connect();
//    }
//
//    if (!_client_param.empty())
//        send_command("set_client_param", "", _client_param, sess);
//
//    if(_on_status_changed)
//        _on_status_changed(started());
}

void
ws_client::on_stop() {

    std::cout << "on_stop" << std::endl;

//    if(_exit_parent)
//        return;
//
//    if (_on_close)
//    {
//        _on_close();
//    }
//
//    console_log("ws_client::on_stop: client on_stop");
//
//    if(_on_status_changed)
//        _on_status_changed(false);
}

void
ws_client::
close(bool exit_base) {
//
//    //exit_base - это выход из приложения, блокируем сообщения
//    _exit_parent = exit_base;
//
//    std::vector<boost::weak_ptr<session_base>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_)
//            if(p->is_ssl()){
//                auto sess = (session*)p;
//                v.emplace_back(sess->weak_from_this());
//            }else{
//                auto sess = (session_ssl*)p;
//                v.emplace_back(sess->weak_from_this());
//            }
//
//    }
//
//    for(auto const& wp : v)
//        if(auto sp = wp.lock())
//            sp->stop();
//
//    ioc.stop();

}

bool
ws_client::
started() {

//    for(auto p : sessions_){
//        if (p->is_open()){
//            return true;
//        }
//    }
//
    return false;
}

boost::uuids::uuid
ws_client::
session_uuid() const {
    return m_data.session_uuid;
}

void
ws_client::
set_session_uuid(const std::string& uuid) {
    arcirk::uuids::is_valid_uuid(uuid, m_data.session_uuid);
}

void
ws_client::
set_user_uuid(const std::string& uuid) {
    param.user_uuid = uuid;
}

std::string ws_client::user_name() const{
    return param.user_name;
}

void ws_client::set_user_name(const std::string& name) {
    param.user_name = name;
}

void ws_client::set_param(client::ServerResponse& resp) {
//    try {
//        std::string uuid = pt.get_member("uuid").to_string();
//        if (!uuid.empty())
//            arcirk::uuids::is_valid_uuid(uuid, _session_uuid);
//        std::string user_uuid = pt.get_member("user_uuid").to_string();
//        if (!user_uuid.empty())
//            set_user_uuid(user_uuid);
//        std::string name = pt.get_member("name").to_string();
//        if (!name.empty())
//            set_user_name(name);
//
//        std::string user_name = pt.get_member("user_name").to_string();
//        if (!name.empty())
//            _user_name = user_name;
//
//        if(_on_connect){
//            _on_connect();
//        }
//
//    }catch (std::exception& e){
//        on_error("ws_client::set_param", arcirk::to_utf(e.what()), -1);
//    }
}

void
ws_client::on_message(const std::string& message) {

    std::cout << "on_message" << std::endl;
//    if (message == "\n" || message.empty() || message == "pong")
//        return;
//    T_vec v = arcirk::split(message, "\n");
//
//    std::string msg;
//
//    if (!v.empty())
//        msg = v[0];
//
//    //если перед сообщением маркер "result" - это приватный ответ сервера клиенту
//    if (v[0].substr(0, 6) == "result"){
//
//        std::string base64 = v[0].substr(7 , v[0].length() - 6);
//        msg = base64;
//
//        try {
//            std::string result = arcirk::base64::base64_decode(base64);
//            auto pt = arcirk::json::bJson();
//            pt.parse(result);
//            if (pt.is_parse()){
//                if (pt.get_member("command").to_string() == "set_client_param"){
//                    //авторизация прошла успешно, устанавливаем параметры сессии на клиенте
//                    set_param(pt);
//                    return;
//                }
//            }
//        }catch (std::exception& e){
//            on_error("ws_client::on_message:set_client_param", arcirk::to_utf(e.what()), -1);
//            return;
//        }
//
//    }
//
//    try {
//        if (_on_message)
//        {
////            if (!decode_message){
////                _on_message(msg);
////            } else{
////                std::string result = base64_decode(msg);
//                _on_message(msg);
////            }
//
//        }
//    }
//    catch (std::exception& e){
//        //std::cerr << "ws_client::on_message error: " << e.what() << std::endl;
//        on_error("ws_client::on_message error: ", arcirk::to_utf(e.what()), -1);
//        return;
//    }

}

void ws_client::on_error(const std::string &what, const std::string &err, int code) {

    std::cout << "on_error" << std::endl;

//    try{
//        if (_on_message)
//        {
//            auto _msg = ws_message();
//            _msg.message().uuid = get_uuid();
//            _msg.message().message = err;
//
//                if(started()){
//                    _msg.message().name = get_user_name();
//                    _msg.message().app_name = get_app_name();
//                }
//
//            _msg.message().command = what;
//            _msg.message().result = "error";
//
//            _on_message(_msg.get_json(true));
//        }
//        if(_status_changed)
//            _status_changed(started());
//    }catch(...){
//        _status_changed(false);
//    }

//    if(m_data.on_error){
//        m_data.on_error(what, err, code);
//    }
}

boost::uuids::uuid ws_client::user_uuid() const {
    return arcirk::uuids::string_to_uuid(param.user_uuid);
}

std::string ws_client::app_name() const{
    return param.app_name;
}

void ws_client::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &cmd_param) {

//        std::string _uuid_form = uuid_form;
////
////        if (cmd.empty())
////            return;
//        if (_uuid_form.empty())
//            _uuid_form = "00000000-0000-0000-0000-000000000000";
//
//        auto _msg = ws_message();
//        _msg.message().uuid = get_uuid();
//        _msg.message().message = param;
//        _msg.message().name = get_user_name();
//        _msg.message().app_name = get_app_name();
//        _msg.message().command = cmd;
//        _msg.message().uuid_form = arcirk::string_to_uuid(_uuid_form, false);
//        _msg.message().user_name = _user_name;
//
//        std::string msg = "cmd " + _msg.get_json(true);
//
//        auto const ss = boost::make_shared<std::string const>(std::move(msg));
//
//        sess->send(ss);
}

void ws_client::send_command(const std::string &cmd, const boost::uuids::uuid &uuid_form, const std::string &json_param) {

//    std::vector<boost::weak_ptr<session>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_)
//            send_command(cmd, uuid_form, json_param, p);
//    }


}

void ws_client::open(const char* host, const char* port, const std::string & auth) {

    bool _ssl = false;

    ssl::context ctx{ssl::context::tlsv12_client};
    set_certificates(ctx);

    std::make_shared<resolver>(ioc, ctx, boost::make_shared<shared_state>(m_data_private))->run(host, port, _ssl);

    ioc.run();

}

void ws_client::open(Uri &url, ssl::context& ctx,
                     const std::string &auth) {

    bool _ssl = url.Protocol == "wss";
    set_certificates(ctx);
    std::make_shared<resolver>(ioc, ctx, boost::make_shared<shared_state>(m_data_private))->run(url.Host.c_str(), url.Port.c_str(), _ssl);

    ioc.run();

}

void ws_client::set_certificates(ssl::context& ctx) {

    using namespace boost::filesystem;

    if(!exists(cert_file))
        return;

    std::string _cert;
    std::ifstream c_in(cert_file.string());
    std::ostringstream c_oss;
    c_oss << c_in.rdbuf();
    _cert = c_oss.str();

    if(_cert.empty()){
        std::cerr << "error read certificate files" << std::endl;
        return;
    }

    load_root_default_certificates(ctx);
}

void ws_client::set_cert_file(const std::string &file) {
    cert_file = file;
}

void ws_client::connect(const client::bClientEvent &event, const client::callbacks& f) {
    if(event == client::bClientEvent::wsClose){
        m_data.on_close= boost::get<callback_close>(f);
    }else if(event == client::bClientEvent::wsConnect){
        m_data.on_connect = boost::get<callback_connect>(f);
    }else if(event == client::bClientEvent::wsError){
        m_data.on_error = boost::get<callback_error>(f);
    }else if(event == client::bClientEvent::wsMessage){
        m_data.on_message = boost::get<callback_message>(f);
    }else if(event == client::bClientEvent::wsStatusChanged){
        m_data.on_status_changed = boost::get<callback_status>(f);
    }
}

void ws_client::on_status_changed(bool status) {
    std::cout << "on_status_changed" << std::endl;
}
