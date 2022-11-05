#ifndef  WEBSOCKETS_WEBSOCKETS_CLIENT_HPP
#define WEBSOCKETS_WEBSOCKETS_CLIENT_HPP

#include <arcirk.hpp>
#include <net.hpp>
#include <beast.hpp>
#include <shared_struct.hpp>
#include <database_struct.hpp>

#include "shared_state.hpp"
//#include "callbacks.hpp"
#include <boost/beast/ssl.hpp>
#include <boost/asio/steady_timer.hpp>

//#include <pre/json/from_json.hpp>
//#include <pre/json/to_json.hpp>

namespace ssl = boost::asio::ssl;
using namespace arcirk;

class websocket_client{

    //net::steady_timer connection_timer;
    //net::io_context& ioc_parent_;

public:
    explicit
    websocket_client(ssl::context& ctx, client::client_param& client_param);
    void connect(const client::client_events& event, const client::callbacks& f);
    void open(const arcirk::Uri& url);
    void close(bool disable_notify);
    void set_certificate_file(const std::string& file);

    void send_message(const std::string& message);
    void send_message(const std::string& message, const std::string& receiver, const std::string& param = "");
    void send_command(const std::string& command, const std::string& param = "");

    bool started();

    [[nodiscard]] boost::uuids::uuid session_uuid() const;

    static void log(const std::string& what, const std::string& message, bool toLocal = true){
        if(toLocal)
            std::cout << what << ": " << arcirk::local_8bit(message) << std::endl;
        else
            std::cout << what << ": " << message << std::endl;
    }

    static void fail(const std::string& what, const std::string& err, bool toLocal = true){
        if(toLocal)
            std::cerr << what << ":error: " << arcirk::local_8bit(err) << std::endl;
        else
            std::cerr << what << ":error: " << err << std::endl;
    }

    void update_client_param(client::client_param& client_param);
    void set_auto_reconnect(bool value);

private:
    client::client_data m_data;
    ssl::context& ctx_;
    boost::shared_ptr<shared_state> state_;
    boost::filesystem::path certificate_file_;
    client::client_param client_param_;
    boost::uuids::uuid session_uuid_{};
    bool _disable_notify_on_close;
    std::string url_;
    bool auto_reconnect_;
    bool timer_is_run;
    bool closed_by_user;

    void set_certificates(ssl::context& ctx);

    void on_connect();
    void on_message(const std::string& message);
    void on_status_changed(bool status);
    void on_error(const std::string &what, const std::string &err, int code);
    void on_close();

    void start(arcirk::Uri& url);

    [[nodiscard]] std::string base64_to_string(const std::string& base64str) const;

    void start_reconnect(bool* is_run);
    //const boost::system::error_code& ec,
    void check_connection(
                          boost::asio::steady_timer* t);

};

#endif