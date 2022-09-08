#include "../include/iclient.h"
#include "../include/client.h"
#include <boost/thread/thread.hpp>


IClient::IClient(_callback_message& on_message)
{
    host = "localhost";
    port = 8080;
    _on_message = on_message;
    client = nullptr;
    app_name = "ws_client";
    user_uuid = arcirk::uuids::nil_uuid();
    _exitParent = false;
    _isRun = false;
}

IClient::IClient(const std::string& _host, const int& _port, _callback_message& on_message)
{
    host = _host;
    port = _port;
    _on_message = on_message;
    client = nullptr;
    app_name = "ws_client";
    user_uuid = arcirk::uuids::nil_uuid();
    _exitParent = false;
    _isRun = false;
}
IClient::IClient(const std::string &_host, const int &_port, _callback_message &on_message,
                 _callback_status &on_status_changed) {
    host = _host;
    port = _port;
    _on_message = on_message;
    client = nullptr;
    app_name = "ws_client";
    user_uuid = arcirk::uuids::nil_uuid();
    _status_changed = on_status_changed;
    _exitParent = false;
    _isRun = false;
}

void IClient::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param) {
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

void IClient::on_connect()
{
    _isRun = true;
}

void IClient::close(bool exitParent) {

    _exitParent = exitParent;
    if (client)
    {
        if (started())
        {
            client->close(exitParent);
        }

        delete client;
        client = nullptr;
    }

    _isRun = false;
}

void IClient::start() {

    boost::asio::io_context ioc;

    close();

    _isRun = false;

    _callback_connect callback = std::bind(&IClient::on_connect, this);

    client = new ws_client(ioc, _client_param);

    try {
        client->open(host.c_str(), std::to_string(port).c_str(), _on_message, _status_changed, callback);
    }
    catch (std::exception& e){
        std::cerr << "IClient::start::exception: " << e.what() <<std::endl;
    }

    std::cout << "IClient::start: exit client thread" << std::endl;


    if(client && !_exitParent){
        if(_on_message){
            _on_message("exit_thread");
        }        
        delete client;
        client = nullptr;
    }

    _isRun = false;

}

bool IClient::started() {


    if(!_isRun)
        return false;

    bool result = false;

    if (client){
        result = client->started();
    }

    return result;
}

void IClient::open(bool new_thread){

    app_uuid = arcirk::uuids::random_uuid();
    user_uuid = arcirk::uuids::random_uuid();

    auto pt = arcirk::json::bJson();
    pt.set_object();

    pt.insert(arcirk::content_value("uuid", app_uuid));
    pt.insert(arcirk::content_value("app_name", app_name));
    pt.insert(arcirk::content_value("user_uuid", user_uuid));
    pt.insert(arcirk::content_value("user_name", user_name));
    pt.insert(arcirk::content_value("host_name", boost::asio::ip::host_name()));

    _client_param = pt.to_string();

    if (new_thread){
        boost::thread(boost::bind(&IClient::start, this)).detach();
    }else
        start();

}

void IClient::send(const std::string &msg, const std::string &sub_user_uuid, const std::string &uuid_form, const std::string& objectName, const std::string& msg_ref) {
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

void IClient::command_to_client(const std::string &recipient, const std::string &command, const std::string &param, const std::string &uuid_form) {

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

void IClient::command_to_server(const std::string &command, const std::string &param, const std::string& uuid_form) {
    if(!client->started())
        return;
    //client->send_command(command, uuid_form, param);
}
