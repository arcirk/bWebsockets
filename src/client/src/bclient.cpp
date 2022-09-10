#include "../include/bclient.h"
#include "../include/client.h"
#include <boost/thread/thread.hpp>

bClient::bClient(const std::string &host, const int &port, const callback_message& message, const callback_status& status_changed, const callback_connect& connect, const callback_close& close, const callback_error& err) {

    _host = host;
    _port = port;

    _on_message = message;
    _on_status_changed = status_changed;
    _on_connect = connect;
    _on_error = err;
    _on_close = close;

    client = nullptr;
    _app_name = "ws_client";
    _user_name = "unanimous";

    _exitParent = false;
    _isRun = false;
}

void bClient::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param) {
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
    _isRun = true;

    if(_on_connect){
        _on_connect();
    }
}

void bClient::close(bool exitParent) {

    _exitParent = exitParent;
    if (client)
    {
        if (started())
        {
            client->close(exitParent);
        }

    }

    _isRun = false;
}

void bClient::on_error(const std::string &what, const std::string &err, int code) {
    if(_on_error){
        _on_error(what, err, code);
    }
}

void bClient::on_message(const std::string &message) {

    if(_on_message){
        _on_message(message);
    }
}

void bClient::on_status_changed(bool status) {
    if(_on_status_changed){
        _on_status_changed(status);
    }
}

void bClient::start(const std::string & auth) {

    boost::asio::io_context ioc;

    if(client)
    {
        std::cerr << arcirk::local_8bit("Клиент уже запущен!") << std::endl;
        return;
    }

    //close();

    _isRun = false;

    callback_connect _connect = std::bind(&bClient::on_connect, this);
    callback_error _err = std::bind(&bClient::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    callback_message _message = std::bind(&bClient::on_message, this, std::placeholders::_1);
    callback_status _status = std::bind(&bClient::on_status_changed, this, std::placeholders::_1);
    callback_close _close = std::bind(&bClient::on_close, this);

    client = new ws_client(ioc, _client_param);

    try {
        client->open(_host.c_str(), std::to_string(_port).c_str(), _on_message, _status, _connect, _err, _close, auth);
    }
    catch (std::exception& e){
        std::cerr << "IClient::start::exception: " << e.what() <<std::endl;
    }

    std::cout << "IClient::start: exit client thread" << std::endl;

   if(!_exitParent) {
       if(_on_status_changed){
           _on_status_changed(false);
       }
       if(_on_close){
           _on_close();
       }
       if(_on_message){
           _on_message("exit client thread");
       }
   }

    if(client){
        std::cout << "delete object" << std::endl;
        delete client;
        client = nullptr;
    }

    _isRun = false;

}

bool bClient::started() {


    if(!_isRun)
        return false;

    bool result = false;

    if (client){
        result = client->started();
    }

    return result;
}

void bClient::client_details(const std::string &app_name, const std::string &user_name,
                             const boost::uuids::uuid &user_uuid, const std::string &user_hash) {
    _app_name = app_name;
    _user_name = user_name;
    _user_uuid = user_uuid;
    _user_hash = user_hash;
}

void bClient::open(bool new_thread){

    auto pt = arcirk::json::bJson();
    pt.set_object();

    pt.insert(arcirk::content_value("app_name", _app_name));
    pt.insert(arcirk::content_value("user_uuid", _user_uuid));
    pt.insert(arcirk::content_value("user_name", _user_name));
    pt.insert(arcirk::content_value("hash", _user_hash));
    pt.insert(arcirk::content_value("host_name", boost::asio::ip::host_name()));

    _client_param = pt.to_string();

    if (new_thread){
        boost::thread(boost::bind(&bClient::start, this, "")).detach();
    }else
        start();

}

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

void bClient::command_to_client(const std::string &recipient, const std::string &command, const std::string &param, const std::string &uuid_form) {

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

void bClient::command_to_server(const std::string &command, const std::string &param, const std::string& uuid_form) {
    if(!client->started())
        return;
    //client->send_command(command, uuid_form, param);
}

void bClient::on_close() {
    if(_on_close)
        _on_close();
}

void bClient::open(const std::string & auth, bool new_thread) {

    auto pt = arcirk::json::bJson();
    pt.set_object();

    pt.insert(arcirk::content_value("app_name", _app_name));
    pt.insert(arcirk::content_value("user_uuid", _user_uuid));
    pt.insert(arcirk::content_value("user_name", _user_name));
    pt.insert(arcirk::content_value("hash", _user_hash));
    pt.insert(arcirk::content_value("host_name", boost::asio::ip::host_name()));

    _client_param = pt.to_string();

    if (new_thread){
        boost::thread(boost::bind(&bClient::start, this, auth)).detach();
    }else
        start(auth);

}
