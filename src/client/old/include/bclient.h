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
#include <boost/beast/ssl.hpp>


using namespace arcirk;

namespace ssl = boost::asio::ssl;

class  ws_client;

typedef boost::variant<callback_message, callback_status, callback_connect, callback_error, callback_close> callbacks;

class  bClient{

    struct bClientData{

        std::string host;
        int port;
        callback_message on_message;
        callback_status on_status_changed;
        callback_connect on_connect;
        callback_error  on_error;
        callback_close on_close;
        callback_message on_message_private;
        callback_status on_status_changed_private;
        callback_connect on_connect_private;
        callback_error  on_error_private;
        callback_close on_close_private;
        bool exitParent;
        bool is_ssl;
        bool isRun;

    public:
        explicit
        bClientData()
                : host("localhost")
                , port(8080)
                , on_message(nullptr)
                , on_status_changed(nullptr)
                , on_connect(nullptr)
                , on_error(nullptr)
                , on_close(nullptr)
                , on_message_private(nullptr)
                , on_status_changed_private(nullptr)
                , on_connect_private(nullptr)
                , on_error_private(nullptr)
                , on_close_private(nullptr)
                , exitParent(false)
                , isRun(false)
                , is_ssl(false)
        {}
    };

    ssl::context& _ctx;

public:

    enum bClientEvent{
        wsMessage = 0,
        wsStatusChanged,
        wsConnect,
        wsClose,
        wsError
    };

    explicit
    bClient(ssl::context& ctx);
    explicit
    bClient(const std::string& host, const int& port, ssl::context& ctx);
    explicit
    bClient(const std::string& url, ssl::context& ctx);
    explicit
    bClient(const arcirk::Uri& url, ssl::context& ctx);

    ~bClient(){
        close(true);
    };

    void connect(const bClientEvent& event, const callbacks& f);

    void close(bool block_message = false);
    void open(const std::string& url = "", const std::string& usr = "", const std::string& pwd = "", bool new_thread = true );

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &json_param);
    bool started();
    void send(const std::string& msg, const std::string& sub_user_uuid, const std::string& uuid_form, const std::string& objectName = "", const std::string& msg_ref = "");
    void command_to_client(const std::string &recipient, const std::string &command, const std::string &json_param, const std::string &uuid_form);
    void command_to_server(const std::string &command, const std::string &json_param, const std::string& uuid_form);

    void client_details(const std::string& app_name, const std::string& user_name, const boost::uuids::uuid& user_uuid = {}, const std::string& user_hash = "");

    bool is_ssl();

    boost::uuids::uuid session_uuid();

    void set_certificate(const std::string& file);

private:

    ClientParam param;
    bClientData m_data;
    ws_client * client;
    std::string _client_param;
    std::string _cert_file;

    void start(const std::string & auth = "", bool is_ssl = false);
    void init();

    void on_connect();
    void on_message(const std::string& message);
    void on_status_changed(bool status);
    void on_error(const std::string &what, const std::string &err, int code);
    void on_close();


};

#endif // IWS_CLIENT_H
