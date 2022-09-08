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
#include <functional>

class  ws_client;

typedef std::function<void(std::string)> _callback_message;
typedef std::function<void(bool)> _callback_status;
typedef std::function<void()> _callback_connect;

class  IClient{

public:

    explicit
    IClient(_callback_message& on_message);
    explicit
    IClient(const std::string& _host, const int& _port, _callback_message& on_message);
    explicit
    IClient(const std::string& _host, const int& _port, _callback_message& callback, _callback_status& on_status_changed);
    ~IClient(){
        close(true);
    };

    void ext_message(const std::string& msg);
    void on_connect();

    void close(bool exitParent = false);
    void open(bool new_thread = true);

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param);
    bool started();
    void send(const std::string& msg, const std::string& sub_user_uuid, const std::string& uuid_form, const std::string& objectName = "", const std::string& msg_ref = "");
    void command_to_client(const std::string &recipient, const std::string &command, const std::string &param, const std::string &uuid_form);
    void command_to_server(const std::string &command, const std::string &param, const std::string& uuid_form);


private:
    boost::uuids::uuid app_uuid;
    std::string app_name;
    std::string user_name;
    boost::uuids::uuid user_uuid;
    std::string host;
    int port;
    ws_client * client;
    std::string _client_param;

    _callback_message _on_message;
    _callback_status _status_changed;
    bool _exitParent;

    void start();

    bool _isRun;
};

#endif // IWS_CLIENT_H
