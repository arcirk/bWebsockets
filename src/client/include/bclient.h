#ifndef IWS_CLIENT_H
#define IWS_CLIENT_H

#include <net.hpp>
#include <beast.hpp>
#ifdef _WINDOWS
#pragma warning(disable:4061)
#pragma warning(disable:4001)
#pragma warning(disable:4100)
#endif // _WINDOWS
#include <arcirk.hpp>
#include <iostream>
#include <string>
#include "callbacks.h"

class  ws_client;

class  bClient{

public:

    explicit
    bClient(const std::string& host, const int& port, const callback_message& message = {}, const callback_status& status_changed = {}, const callback_connect& connect = {}, const callback_close& close = {}, const callback_error& err = {});
    explicit
    bClient(const arcirk::Uri& url, const callback_message& message = {}, const callback_status& status_changed = {}, const callback_connect& connect = {}, const callback_close& close = {}, const callback_error& err = {});
    explicit
    bClient(const callback_message& message = {}, const callback_status& status_changed = {}, const callback_connect& connect = {}, const callback_close& close = {}, const callback_error& err = {});

    ~bClient(){
        close(true);
    };

    void on_connect();
    void on_message(const std::string& message);
    void on_status_changed(bool status);
    void on_error(const std::string &what, const std::string &err, int code);
    void on_close();

    void close(bool exitParent = false);
    void open(bool new_thread = true);
    void open(const std::string& auth, bool new_thread = true);

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param);
    bool started();
    void send(const std::string& msg, const std::string& sub_user_uuid, const std::string& uuid_form, const std::string& objectName = "", const std::string& msg_ref = "");
    void command_to_client(const std::string &recipient, const std::string &command, const std::string &param, const std::string &uuid_form);
    void command_to_server(const std::string &command, const std::string &param, const std::string& uuid_form);

    void client_details(const std::string& app_name, const std::string& user_name, const boost::uuids::uuid& user_uuid = {}, const std::string& user_hash = "");

    bool is_ssl();

    boost::uuids::uuid session_uuid();

private:
    std::string _app_name;
    std::string _user_name;
    std::string _user_hash;
    boost::uuids::uuid _user_uuid{};
    std::string _host;
    int _port;
    ws_client * client;
    std::string _client_param;

    callback_message _on_message;
    callback_status _on_status_changed;
    callback_connect _on_connect;
    callback_error  _on_error;
    callback_close _on_close;

    bool _exitParent;

    void start(const std::string & auth = "");

    bool _isRun;

    bool _is_ssl;

};

#endif // IWS_CLIENT_H
