#include "../include/bclient.h"
#include "../include/client.h"
#include <boost/thread/thread.hpp>


void bClient::init(){
    m_data = bClientData();
    param = ClientParam();
    param.app_name = "ws_client";
    param.user_name = "unanimous";

    m_data.on_connect = std::bind(&bClient::on_connect, this);
    m_data.on_error = std::bind(&bClient::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_data.on_message = std::bind(&bClient::on_message, this, std::placeholders::_1);
    m_data.on_status_changed = std::bind(&bClient::on_status_changed, this, std::placeholders::_1);
    m_data.on_close = std::bind(&bClient::on_close, this);
}

bClient::bClient(ssl::context& ctx)
    : client(nullptr),
    _ctx(ctx)
{
    init();
}

bClient::bClient(const arcirk::Uri &url, ssl::context& ctx)
        : client(nullptr),
          _ctx(ctx)
{
    init();
    m_data.host = url.Host;
    m_data.port = url.Port.empty() ? 0 : std::atoi(url.Port.c_str());
}

bClient::bClient(const std::string &url, ssl::context& ctx)
        : client(nullptr),
          _ctx(ctx)
{
    init();
    auto u = arcirk::Uri::Parse(url);
    m_data.host = u.Host;
    m_data.port = u.Port.empty() ? 0 : std::atoi(u.Port.c_str());
}

bClient::bClient(const std::string &host, const int &port, ssl::context& ctx)
        : client(nullptr),
          _ctx(ctx)
{
    init();
    m_data.host = host;
    m_data.port = port;
}

void bClient::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &json_param) {
//
//    if (client){
//
//        if (!started())
//            return;
//
//        const std::string& _cmd = cmd;
//        std::string _uuid_form = uuid_form;
//        std::string _param = param;
//
//        if (_cmd.empty())
//            return;
//        if (_uuid_form.empty())
//            _uuid_form = "00000000-0000-0000-0000-000000000000";
//
//        if (_param.empty())
//            _param = R"({"command":")" + _cmd + "\"}";
//
//        client->send(_param, true, "00000000-0000-0000-0000-000000000000", _uuid_form, _cmd);
//    }

}

void bClient::on_connect()
{
    m_data.isRun = true;

    if(m_data.on_connect_private){
        m_data.on_connect_private();
    }
}

void bClient::close(bool block_message) {

    m_data.exitParent = block_message;
    if (client)
    {
        if (started())
        {
            client->close(block_message);
        }

    }

    m_data.isRun = false;
}

void bClient::on_error(const std::string &what, const std::string &err, int code) {
    if(m_data.on_error_private){
        m_data.on_error_private(what, err, code);
    }
}

void bClient::on_message(const std::string &message) {

    if(m_data.on_message_private){
        m_data.on_message_private(message);
    }
}

void bClient::on_status_changed(bool status) {
    if(m_data.on_status_changed_private){
        m_data.on_status_changed_private(status);
    }
}

void bClient::start(const std::string & auth, bool is_ssl) {

    boost::asio::io_context ioc;

    if(client)
    {
        std::cerr << arcirk::local_8bit("Клиент уже запущен!") << std::endl;
        return;
    }

    m_data.isRun = false;

    client = new ws_client(ioc, _client_param);
    client->set_cert_file(_cert_file);

    try {
        if(!is_ssl)
            client->open(m_data.host.c_str(), std::to_string(m_data.port).c_str(), m_data.on_message, m_data.on_status_changed, m_data.on_connect, m_data.on_error, m_data.on_close, auth);
        else{
            auto url = arcirk::Uri::Parse("wss://" + m_data.host + ":" + std::to_string(m_data.port));
            client->open(url, _ctx, m_data.on_message, m_data.on_status_changed, m_data.on_connect, m_data.on_error, m_data.on_close, auth);
        }
    }
    catch (std::exception& e){
        std::cerr << "IClient::start::exception: " << e.what() <<std::endl;
    }

    std::cout << "IClient::start: exit client thread" << std::endl;

   if(!m_data.exitParent) {
       if(m_data.on_status_changed){
           m_data.on_status_changed(false);
       }
       if(m_data.on_close){
           m_data.on_close();
       }
       if(m_data.on_message){
           m_data.on_message("exit client thread");
       }
   }

    if(client){
        std::cout << "delete websocket object" << std::endl;
        delete client;
        client = nullptr;
    }

    m_data.isRun = false;

}

bool bClient::started() {

    if(!m_data.isRun)
        return false;

    bool result = false;

    if (client){
        result = client->started();
    }

    return result;
}

void bClient::client_details(const std::string &app_name, const std::string &user_name,
                             const boost::uuids::uuid &user_uuid, const std::string &user_hash) {
    param.app_name = app_name;
    param.user_name = user_name;
    param.user_uuid = arcirk::uuids::uuid_to_string(user_uuid);
    param.hash = user_hash;
}

void bClient::open(const std::string &url, const std::string &usr, const std::string &pwd, bool new_thread) {

    bool _is_ssl = false;
    if(!url.empty()){
        arcirk::Uri _url = arcirk::Uri::Parse(url);
        m_data.host = _url.Host;
        m_data.port = _url.Port.empty() ? 0 : std::atoi(_url.Port.c_str());
        _is_ssl = _url.Protocol == "wss";
    }
    if(m_data.host.empty()){
        std::cerr << arcirk::local_8bit("Не верный адрес сервера!") << std::endl;
        return;
    }

    std::string auth;
    if(!usr.empty())
        param.user_name = usr;
    if(!pwd.empty()){
        param.hash = arcirk::get_hash(usr, pwd);
        auth = "Basic " + arcirk::base64::base64_encode(usr + ":" + pwd);
    }

    if (new_thread){
        boost::thread(boost::bind(&bClient::start, this, auth, _is_ssl)).detach();
    }else
        start(auth);

}

//void bClient::open(bool new_thread){
//
//    _client_param= to_string(pre::json::to_json(param));
//
//    if (new_thread){
//        boost::thread(boost::bind(&bClient::start, this, "")).detach();
//    }else
//        start();
//
//}
//
//void bClient::open(const std::string & auth, bool new_thread) {
//
//    _client_param= to_string(pre::json::to_json(param));
//    if (new_thread){
//        boost::thread(boost::bind(&bClient::start, this, auth)).detach();
//    }else
//        start(auth);
//
//}
//
//void bClient::open(const std::string &usr, const std::string &pwd, bool new_thread) {
//    param.user_name = usr;
//    param.hash = arcirk::get_hash(usr, pwd);
//    _client_param= to_string(pre::json::to_json(param));
//    std::string auth_cred;
//    if(usr.empty()){
//        auth_cred = "admin:admin";
//    }else{
//        auth_cred = usr + ":" + pwd;
//    }
//    std::string encoded_auth_cred = "Basic " + arcirk::base64::base64_encode(auth_cred);
//    if (new_thread){
//        boost::thread(boost::bind(&bClient::start, this, encoded_auth_cred)).detach();
//    }else
//        start(encoded_auth_cred);
//}
//
//void bClient::open(const std::string &url, const std::string &usr, const std::string &pwd, bool new_thread) {
//    arcirk::Uri _url = arcirk::Uri::Parse(url);
//    _host = _url.Host;
//    _port = _url.Port.empty() ? 0 : std::atoi(_url.Port.c_str());
//    param.user_name = usr;
//    param.hash = arcirk::get_hash(usr, pwd);
//    _client_param= to_string(pre::json::to_json(param));
//    std::string auth_cred;
//    if(usr.empty()){
//        auth_cred = "admin:admin";
//    }else{
//        auth_cred = usr + ":" + pwd;
//    }
//    std::string encoded_auth_cred = "Basic " + arcirk::base64::base64_encode(auth_cred);
//    if (new_thread){
//        boost::thread(boost::bind(&bClient::start, this, encoded_auth_cred)).detach();
//    }else
//        start(encoded_auth_cred);
//}

void bClient::send(const std::string &msg, const std::string &sub_user_uuid, const std::string &uuid_form, const std::string& objectName, const std::string& msg_ref) {
//
//    std::string _msg = msg;
//    std::string _uuid_form = uuid_form;
//    std::string _sub_user_uuid = sub_user_uuid;
//
//    if (_msg.empty())
//        return;
//    if (_uuid_form.empty())
//        _uuid_form = "00000000-0000-0000-0000-000000000000";
//    if (_sub_user_uuid.empty())
//        _sub_user_uuid = "00000000-0000-0000-0000-000000000000";
//
//    if (client)
//    {
//        if (started())
//        {
//            client->send(_msg, false, _sub_user_uuid, _uuid_form, "message", objectName, msg_ref);
//        }
//    }
}

void bClient::command_to_client(const std::string &recipient, const std::string &command, const std::string &json_param, const std::string &uuid_form) {

//    if(!client->started())
//        return;
//
//    auto params = arcirk::json::bJson();
//    params.set_object();
//    params.insert(arcirk::content_value("command", command));
//    params.insert(arcirk::content_value("param",  arcirk::base64::base64_encode(param)));
//    arcirk::bValue p = params.to_object();
//
//    auto obj = arcirk::json::bJson();
//    obj.set_object();
//    obj.insert(arcirk::content_value("uuid_agent", recipient));
//    obj.insert(arcirk::content_value("uuid_client", client->get_uuid()));
//    obj.insert("param", p);
//
//
//    std::string arg = obj.to_string();
//    client->send_command("command_to_client", uuid_form, arg);

}

void bClient::command_to_server(const std::string &command, const std::string &json_param, const std::string& uuid_form) {
    if(!client->started())
        return;
    //client->send_command(command, uuid_form, param);
}

void bClient::on_close() {
    if(m_data.on_close_private)
        m_data.on_close_private();
}



boost::uuids::uuid bClient::session_uuid() {
    if(client)
        return client->session_uuid();
    else
        return arcirk::uuids::nil_uuid();
}

void bClient::connect(const bClient::bClientEvent &event, const callbacks& f) {
    if(event == bClientEvent::wsClose){
        m_data.on_close_private = boost::get<callback_close>(f);
    }else if(event == bClientEvent::wsConnect){
        m_data.on_connect_private = boost::get<callback_connect>(f);
    }else if(event == bClientEvent::wsError){
        m_data.on_error_private = boost::get<callback_error>(f);
    }else if(event == bClientEvent::wsMessage){
        m_data.on_message_private = boost::get<callback_message>(f);
    }else if(event == bClientEvent::wsStatusChanged){
        m_data.on_status_changed_private = boost::get<callback_status>(f);
    }
}

void bClient::set_certificate(const std::string &file) {
    _cert_file = file;
}
