//
// Created by arcady on 13.07.2021.
//

#include "../include/net.h"
#include "../include/ws_client.h"
#include "../include/ws_session.h"

#include <utility>

#include <boost/locale.hpp>
//#include <boost/lexical_cast.hpp>
#include <boost/locale/generator.hpp>


using namespace arcirk;

ws_client::ws_client(net::io_context &io_context, const std::string& client_param)
: ioc(io_context)
{

    decode_message = false;
    _client_param = client_param;
    set_uuid();
    set_user_uuid();
    set_name("anonymous");
    _app_name = "unknown";
    _is_login = false;
    //m_custom_param = {};
}

void
ws_client::
send(const std::string &message, bool is_cmd, const std::string &sub_user_uuid, const std::string &uuid_form,
     const std::string &command, const std::string &objectName, const std::string &msg_ref)
{
    std::string _sub_user_uuid = sub_user_uuid;
    if (_sub_user_uuid.empty())
        _sub_user_uuid = "00000000-0000-0000-0000-000000000000";

    boost::uuids::uuid  uuid_channel = arcirk::string_to_uuid(_sub_user_uuid);

    std::string msg;

    if (is_cmd){
        msg.append("cmd ");
    } else{
        msg.append("msg " + _sub_user_uuid + " ");
    }

    auto _msg = ws_message();
    _msg.message().uuid = get_uuid();
    _msg.message().message = message;
    _msg.message().name = get_name();
    _msg.message().uuid_channel = uuid_channel;
    _msg.message().app_name = get_app_name();
    _msg.message().command = command;
    _msg.message().uuid_form = arcirk::string_to_uuid(uuid_form, false);
    _msg.message().uuid_user = get_user_uuid();
    _msg.message().object_name = objectName;
    if (msg_ref.empty())
        _msg.message().msg_ref = arcirk::random_uuid();
    else
        _msg.message().msg_ref = msg_ref;

    msg.append(_msg.get_json(true));

    auto const ss = boost::make_shared<std::string const>(std::move(msg));

    std::vector<boost::weak_ptr<session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            v.emplace_back(p->weak_from_this());
    }

    for(auto const& wp : v)
        if(auto sp = wp.lock())
            sp->send(ss);
}

void
ws_client::
open(const char *host, const char *port) {
    boost::make_shared<session>(ioc)->run(host, port, this);
    _is_login = false;
    ioc.run();
}

void
ws_client::
open(const char* host, const char* port, _callback_message& msg) {

    _is_login = false;

    _callback_msg = msg;

    boost::make_shared<session>(ioc)->run(host, port, this);
    ioc.run();

}

void
ws_client::
open(const char *host, const char *port, const char *name) {
    boost::make_shared<session>(ioc)->run(host, port, this);
    _is_login = false;
    set_name(name);
    ioc.run();
}

void
ws_client::
open(const char *host, const char *port, const char *name, const char *uuid) {
    boost::make_shared<session>(ioc)->run(host, port, this);
    set_name(name);
    set_uuid(uuid);
    _is_login = false;
    ioc.run();
}

void
ws_client::
on_connect(session * sess){

    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(sess);
    //_started = true;

    if(callback_connect){
        callback_connect();
    }

    if (_callback_msg)
    {

        auto _msg = ws_message();
        _msg.message().uuid = get_uuid();
        _msg.message().message = "client connection success";
        _msg.message().name = get_name();
        _msg.message().app_name = _app_name;
        _msg.message().command = "connect_unknown_user";
        _msg.message().user_name = _user_name;
        _msg.message().host_name = boost::asio::ip::host_name();

        std::string msg = _msg.get_json(true);
        _callback_msg(msg);
    }

    if (!_client_param.empty())
        send_command("set_client_param", "", _client_param, sess);

    if(status_changed)
        status_changed(started());
}

void
ws_client::on_stop() {

    if(_exit_parent)
        return;

    if (_callback_msg)
    {
        auto _msg = ws_message();
        _msg.message().uuid = get_uuid();
        _msg.message().message = "client leave";
        _msg.message().name = get_name();
        _msg.message().app_name = get_app_name();
        _msg.message().command = "close_connections";
        _msg.message().user_name = _user_name;

        _callback_msg(_msg.get_json(true));

        _callback_msg("exit_thread");

    }
    if(status_changed)
        status_changed(false);

    console_log("ws_client::on_stop: client on_stop");

    //sessions_.clear();
}

void
ws_client::
close(bool exit_base) {

    //exit_base - это выход из приложения, блокируем сообщения
    _exit_parent = exit_base;

    std::vector<boost::weak_ptr<session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            v.emplace_back(p->weak_from_this());
    }

    for(auto const& wp : v)
        if(auto sp = wp.lock())
            sp->stop();

    ioc.stop();

//    if(status_changed)
//        status_changed(started());
}

bool
ws_client::
started() {

    //bool _started = false;

//    std::vector<boost::weak_ptr<session>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_)
//            v.emplace_back(p->weak_from_this());
//    }
//
//    for(auto const& wp : v)
//        if(auto sp = wp.lock()){
//            if (sp->is_open()){
//                _started = true;
//                break;
//            }
//        }

        for(auto p : sessions_){
            if (p->is_open()){
                return true;
                //break;
            }
        }

    return false;
}

boost::uuids::uuid&
ws_client::
get_uuid() {
    return uuid_;
}

void
ws_client::
set_uuid() {
    uuid_ = boost::uuids::random_generator()();
}

void
ws_client::
set_uuid(const std::string& uuid) {
    //boost::uuids::string_generator gen;
    //uuid_ = gen(uuid);
    is_valid_uuid(uuid, uuid_);
}

void
ws_client::
set_user_uuid(const std::string& uuid) {
    is_valid_uuid(uuid, _user_uuid);
}

std::string ws_client::get_name() const{
    return name_;
}

void ws_client::set_name(const std::string& name) {
    name_ = name;
}

std::string ws_client::get_sys_user_name() const{
    return _user_name;
}

void ws_client::set_param(ptree &pt) {
    try {
        std::string uuid = bJson::get_pt_member(pt, "uuid");
        if (!uuid.empty())
            set_uuid(uuid);
        std::string user_uuid = bJson::get_pt_member(pt, "user_uuid");
        if (!user_uuid.empty())
            set_user_uuid(user_uuid);
        std::string name = bJson::get_pt_member(pt, "name");
        if (!name.empty())
            set_name(name);

        std::string user_name = bJson::get_pt_member(pt, "user_name");
        if (!name.empty())
            _user_name = user_name;

    }catch (std::exception&){
        _callback_msg("Ошибка установки параметров сеанса!");
    }
}

void
ws_client::on_read(const std::string& message) {

    //std::string result = message;

    if (message == "\n" || message.empty() || message == "pong")
        return;

    T_vec v = split(message, "\n");

    std::string msg;

    if (!v.empty())
        msg = v[0];

    //если перед сообщением маркер "result" - это приватный ответ сервера клиенту
    if (v[0].substr(0, 6) == "result"){

        std::string base64 = v[0].substr(7 , v[0].length() - 6);
        msg = base64;

        try {
            std::string result = base64_decode(base64);
            ptree pt;
            if (bJson::parse_pt(result, pt)){
                if (bJson::get_pt_member(pt, "command") == "set_client_param"){
                    //авторизация прошла успешно, устанавливаем параметры сессии на клиенте
                    _is_login = true;
                    set_param(pt);
                }
            }
        }catch (std::exception& e){
            std::cerr << "ws_client::on_read:set_client_param error: " << e.what() << std::endl;
            _callback_msg(e.what());
            return;
        }

    }

    try {
        if (_callback_msg)
        {
            //std::cout << "decode_message: {" << decode_message << "}" << std::endl;
            if (!decode_message){
                _callback_msg(msg);
            } else{
                std::string result = base64_decode(msg);
                _callback_msg(result);
            }

        }
    }
    catch (std::exception& e){
        std::cerr << "ws_client::on_read error: " << e.what() << std::endl;
        //std::cout << "message: {" << message << "}" << std::endl;
        //std::cout << "msg: {" << msg << "}" << std::endl;
        return;
    }

}

std::string ws_client::get_client_info() {

    auto _msg = ws_message();
    _msg.message().uuid = get_uuid();
    _msg.message().message = "get_client_info";
    _msg.message().name = get_name();
    _msg.message().app_name = get_app_name();

    return _msg.get_json(true);
}

void ws_client::error(const std::string &what, const std::string &err) {
    try{
        if (_callback_msg)
        {
            auto _msg = ws_message();
            _msg.message().uuid = get_uuid();
            _msg.message().message = err;

                if(started()){
                    _msg.message().name = get_name();
                    _msg.message().app_name = get_app_name();
                }

            _msg.message().command = what;
            _msg.message().result = "error";

            _callback_msg(_msg.get_json(true));
        }
        if(status_changed)
            status_changed(started());
    }catch(...){
        status_changed(false);
    }
}

boost::uuids::uuid& ws_client::get_user_uuid() {
    return _user_uuid;
}

void ws_client::set_user_uuid() {

    _user_uuid = boost::uuids::random_generator()();

}

std::string &ws_client::get_app_name() {
    return _app_name;
}

void ws_client::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param, session * sess) {

        std::string _uuid_form = uuid_form;
//
//        if (cmd.empty())
//            return;
        if (_uuid_form.empty())
            _uuid_form = "00000000-0000-0000-0000-000000000000";

        auto _msg = ws_message();
        _msg.message().uuid = get_uuid();
        _msg.message().message = param;
        _msg.message().name = get_name();
        _msg.message().app_name = get_app_name();
        _msg.message().command = cmd;
        _msg.message().uuid_form = arcirk::string_to_uuid(_uuid_form, false);
        _msg.message().user_name = _user_name;

        std::string msg = "cmd " + _msg.get_json(true);

        auto const ss = boost::make_shared<std::string const>(std::move(msg));

        sess->send(ss);
}

void ws_client::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &json_param) {

    std::vector<boost::weak_ptr<session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            send_command(cmd, uuid_form, json_param, p);
    }


}

void ws_client::open(const char *host, const char *port, _callback_message &msg, _callback_status &st, _callback_connect& cn) {

    _is_login = false;

    _callback_msg = msg;
    status_changed = st;
    callback_connect = cn;

    boost::make_shared<session>(ioc)->run(host, port, this);
    ioc.run();

}

void
ws_client::set_webdav_settings_on_client(const std::string &sett) {

    auto json = arcirk::bJson();
    json.parse(sett);

    if(json.is_parse()){
        _webdav_host = json.get_member(bConf::get_field_alias(bConfFields::WebDavHost)).get_string();
        _webdav_user = json.get_member(bConf::get_field_alias(bConfFields::WebDavUser)).get_string();
        _webdav_pwd = json.get_member(bConf::get_field_alias(bConfFields::WebDavPwd)).get_string();
        _webdav_ssl = json.get_member(bConf::get_field_alias(bConfFields::WebDavSSL)).get_bool();
        _webdav_use_local = json.get_member(bConf::get_field_alias(bConfFields::UseLocalWebDavDirectory)).get_bool();
        _webdav_local_dir = json.get_member(bConf::get_field_alias(bConfFields::LocalWebDavDirectory)).get_bool();
    }

}

void ws_client::set_webdav_directory(const std::string &dir) {

    _webdav_local_dir = dir;
}

void ws_client::use_webdav_local_dir(bool val) {
    _webdav_use_local = val;
}

std::string ws_client::get_webdav_user() const {
    return _webdav_user;
}

std::string ws_client::get_webdav_pwd() const {
    return _webdav_pwd;
}

std::string ws_client::get_webdav_host() const {
    return _webdav_host;
}

bool ws_client::get_webdav_ssl() {
    return _webdav_ssl;
}

//void ws_client::add_custom_client_param(content_value value) {
//    m_custom_param.push_back(value);
//}
