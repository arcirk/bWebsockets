//#include "net.h"

#include <arcirk.hpp>
#include <net.hpp>
#include <beast.hpp>

#include <iostream>
#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <boost/asio/io_context.hpp>
#include "callbacks.hpp"

#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

#include <boost/beast/ssl.hpp>
#include "../include/shared_state.hpp"

using namespace arcirk;

namespace ssl = boost::asio::ssl;

class ws_client final{

public:
    explicit
    ws_client(net::io_context &io_context, client::ClientParam& client_param);

    void connect(const client::bClientEvent& event, const client::callbacks& f);

    void open(const char* host, const char* port, const std::string & auth = "");
    void open(arcirk::Uri& url, ssl::context& ctx, const std::string & auth = "");

    void send(const std::string &message, const boost::uuids::uuid &recipient = {}, const boost::uuids::uuid &uuid_form = {});

    void send_command(const std::string &cmd, const boost::uuids::uuid &uuid_form, const std::string &cmd_param);

    void close(bool block_notify = false);
    bool started();

    boost::uuids::uuid session_uuid() const;
    std::string app_name() const;

    boost::uuids::uuid user_uuid() const;
    void set_user_uuid(const std::string& uuid);
    [[nodiscard]] std::string user_name() const;
    void set_user_name(const std::string& name);

    void on_connect();
    void on_message(const std::string& message);
    void on_stop();
    void on_error(const std::string &what, const std::string &err, int code);
    void on_status_changed(bool status);

    void set_cert_file(const std::string& file);

private:
    client::ClientParam param;
    client::bClientData m_data;
    client::bClientData m_data_private;
    boost::shared_ptr<shared_state> state_;
    boost::asio::io_context &ioc;

    //std::string _client_param;

    std::string _app_name;
    //boost::uuids::uuid _session_uuid{};

    //boost::uuids::uuid  _user_uuid{};
    //std::string _user_name;

    //bool _exit_parent = false;

    boost::filesystem::path cert_file;

    static void console_log(const std::string& log){
        std::cout << arcirk::local_8bit(log) << std::endl;
    }

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param);

    void set_param(client::ServerResponse& resp);

    void set_session_uuid(const std::string& uuid);

    void set_certificates(ssl::context& ctx);

};
