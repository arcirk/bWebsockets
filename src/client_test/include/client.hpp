#ifndef  WEBSOCKETS_WEBSOCKETS_CLIENT_HPP
#define WEBSOCKETS_WEBSOCKETS_CLIENT_HPP

#include <arcirk.hpp>
#include <net.hpp>
#include <beast.hpp>
#include "shared_state.hpp"
#include "callbacks.hpp"
#include <boost/beast/ssl.hpp>

#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

namespace ssl = boost::asio::ssl;
using namespace arcirk;

class websocket_client{

public:
    explicit
    websocket_client(ssl::context& ctx, client::ClientParam& client_param);
    void connect(const client::bClientEvent& event, const client::callbacks& f);
    void open(const arcirk::Uri& url);
    void close(bool disable_notify);
    void set_certificate_file(const std::string& file);

private:
    client::bClientData m_data;
    //boost::asio::io_context &ioc;
    ssl::context& ctx_;
    boost::shared_ptr<shared_state> state_;
    boost::filesystem::path certificate_file_;

    void set_certificates(ssl::context& ctx);

    void on_connect();
    void on_message(const std::string& message);
    void on_status_changed(bool status);
    void on_error(const std::string &what, const std::string &err, int code);
    void on_close();

    void start(arcirk::Uri& url);
};

#endif