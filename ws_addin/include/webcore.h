//
// Created by admin on 02.08.2022.
//

#ifndef WS_SOLUTION_WEBCORE_H
#define WS_SOLUTION_WEBCORE_H

#include "stdafx.h"

#include "Component.h"
#include <arcirk.hpp>
#include <client.hpp>
#include <boost/beast/ssl.hpp>

class WebCore final : public Component {

public:
    const char *Version = u8"1.1.0";
    const std::string version = "1.1.0";
    const std::string application_name = "1c_webcore";

    std::string extensionName() override;

    WebCore();
    ~WebCore() override;

    void close(const variant_t& exit_base = false);
    void open(const variant_t &url, const variant_t &user, const variant_t &user_uuid, const variant_t &pwd = "");

private:
    std::shared_ptr<websocket_client> m_client;
    client::client_param client_param;
    server::server_config app_conf;
    std::string url_;

//    std::string _client_param;

//
//    std::string _sys_user;
//
//

//
//    boost::uuids::uuid _user_uuid;
//    std::string _user_name;
//    std::string _hash;
//
//    std::string  _current_recipient;
//    std::string _document_name;
//    bool _is_source_event_uuid_form;
//    std::string  _job;
//    std::string  _job_description;
//
//    std::string _app_name = "1c_client";
//    bool no_notify;
//
//    void on_server_response(const std::string& resp);
//
//    void processServeResponse(const std::string &jsonResp);
//
    void emit(const std::string& command, const std::string &resp, const std::string &uuid_form = "");
//
    bool started();
//
//    void get_online_users(const variant_t &appFilter, const variant_t& uuid_form);
//
//    void command_to_client(const variant_t &recipient, const variant_t &command, const variant_t &param, const variant_t& uuid_form);
//
//    void command_to_server(const variant_t &command, const variant_t &param, const variant_t& uuid_form);
//
//    void connect_to_recipient(const variant_t &recipient);
//
//    std::string getSha1(const variant_t &source);
//
//    void set_document_name(const variant_t &value);
//
//    void set_app_name(const variant_t &value);
//
    std::string session_uuid();
//
//    void set_job_data(const variant_t &jobUuid, const variant_t &jobDescription);
//
    //callbacks
    void on_connect();
    void on_message(const std::string& message);
    void on_stop();
    void on_error(const std::string &what, const std::string &err, int code);
    void on_status_changed(bool status);

    void verify_directories();
    void read_conf(server::server_config & result);
    void write_conf(server::server_config & conf);

    void get_online_users();
};

#endif //WS_SOLUTION_WEBCORE_H