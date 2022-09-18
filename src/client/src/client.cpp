//
// Created by arcady on 13.07.2021.
//

#include "net.hpp"
#include <arcirk.hpp>
#include "../include/client.h"
#include "../include/session.h"
#include "../include/session_ssl.h"
#include <boost/locale.hpp>
#include <boost/locale/generator.hpp>
#include <common/root_certificates.hpp>

using namespace arcirk;

ws_client::ws_client(net::io_context &io_context, const std::string& client_param)
: ioc(io_context)
{
    _client_param = client_param;
    set_user_name("anonymous");
    _app_name = "unknown";;
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
//
//    std::string msg;
//
//    if (is_cmd){
//        msg.append("cmd ");
//    } else{
//        msg.append("msg " + _sub_user_uuid + " ");
//    }
//
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
//
//    auto const ss = boost::make_shared<std::string const>(std::move(msg));
//
//    std::vector<boost::weak_ptr<session>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_)
//            v.emplace_back(p->weak_from_this());
//    }
//
//    for(auto const& wp : v)
//        if(auto sp = wp.lock())
//            sp->send(ss);
}

void
ws_client::
on_connect(session_base * sess){

    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(sess);

    if(_on_connect){
        _on_connect();
    }

    if (!_client_param.empty())
        send_command("set_client_param", "", _client_param, sess);

    if(_on_status_changed)
        _on_status_changed(started());
}

void
ws_client::on_stop() {

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

    //exit_base - это выход из приложения, блокируем сообщения
    _exit_parent = exit_base;

    std::vector<boost::weak_ptr<session_base>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            if(p->is_ssl()){
                auto sess = (session*)p;
                v.emplace_back(sess->weak_from_this());
            }else{
                auto sess = (session_ssl*)p;
                v.emplace_back(sess->weak_from_this());
            }

    }

    for(auto const& wp : v)
        if(auto sp = wp.lock())
            sp->stop();

    ioc.stop();

}

bool
ws_client::
started() {

    for(auto p : sessions_){
        if (p->is_open()){
            return true;
        }
    }

    return false;
}

boost::uuids::uuid&
ws_client::
session_uuid() {
    return _session_uuid;
}

void
ws_client::
set_session_uuid(const std::string& uuid) {
    arcirk::uuids::is_valid_uuid(uuid, _session_uuid);
}

void
ws_client::
set_user_uuid(const std::string& uuid) {
    arcirk::uuids::is_valid_uuid(uuid, _user_uuid);
}

std::string ws_client::user_name() const{
    return _user_name;
}

void ws_client::set_user_name(const std::string& name) {
    _user_name = name;
}

void ws_client::set_param(ServerResponse& resp) {
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
ws_client::on_read(const std::string& message) {

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
//            on_error("ws_client::on_read:set_client_param", arcirk::to_utf(e.what()), -1);
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
//        //std::cerr << "ws_client::on_read error: " << e.what() << std::endl;
//        on_error("ws_client::on_read error: ", arcirk::to_utf(e.what()), -1);
//        return;
//    }

}

void ws_client::on_error(const std::string &what, const std::string &err, int code) {
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

    if(_on_error){
        _on_error(what, err, code);
    }
}

boost::uuids::uuid& ws_client::user_uuid() {
    return _user_uuid;
}

std::string &ws_client::app_name() {
    return _app_name;
}

void ws_client::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param, session_base * sess) {

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

void ws_client::open(const char* host, const char* port, const callback_message& message, const callback_status& status_changed, const callback_connect& connect, const callback_error& error, const callback_close& close, const std::string & auth) {

    _on_message = message;
    _on_status_changed = status_changed;
    _on_connect = connect;
    _on_error = error;
    _on_close = close;

    boost::make_shared<session>(ioc, auth)->run(host, port, this);
    ioc.run();

}

void ws_client::open(Uri &url, ssl::context& ctx, const callback_message &message, const callback_status &status_changed,
                     const callback_connect &connect, const callback_error &err, const callback_close &close,
                     const std::string &auth) {
    _on_message = message;
    _on_status_changed = status_changed;
    _on_connect = connect;
    _on_error = err;
    _on_close = close;

    if(url.Protocol == "wss"){
        //ssl::context ctx{ssl::context::tlsv12_client};
        set_certificates(ctx);
        boost::make_shared<session_ssl>(ioc, ctx, auth)->run(url.Host.c_str(), url.Port.c_str(), this);
        ioc.run();
    }else{
        boost::make_shared<session>(ioc, auth)->run(url.Host.c_str(), url.Port.c_str(), this);
        ioc.run();
    }

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

    //load_root_certificates(ctx, _cert);
    load_root_default_certificates(ctx);
}

void ws_client::set_cert_file(const std::string &file) {
    cert_file = file;
}

