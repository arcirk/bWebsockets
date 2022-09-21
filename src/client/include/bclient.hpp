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
#include "callbacks.hpp"
#include <boost/beast/ssl.hpp>

using namespace arcirk;

namespace ssl = boost::asio::ssl;

class  ws_client;

class  bClient{

    ssl::context& _ctx;

public:

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

    void connect(const client::bClientEvent& event, const client::callbacks& f);

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

    client::ClientParam param;
    client::bClientData m_data;
    ws_client * _client;
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
