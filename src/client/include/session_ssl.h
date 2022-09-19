//
// Created by admin on 17.09.2022.
//

#ifndef ARCIRK_SOLUTION_SESSION_SSL_H
#define ARCIRK_SOLUTION_SESSION_SSL_H

#include <net.hpp>
#include <beast.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <boost/asio/steady_timer.hpp>

#include <deque>
#include "session_base.hpp"

class ws_client;

namespace ssl = boost::asio::ssl;
using boost::asio::steady_timer;

class session_ssl : public session_base, public boost::enable_shared_from_this<session_ssl> {
    tcp::resolver resolver_;
    websocket::stream<
            beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;

public:
    explicit
    session_ssl(net::io_context& ioc, ssl::context& ctx, const std::string & auth);
    ~session_ssl();
    void
    run(char const* host, char const* port, ws_client * client);
    void
    on_resolve(
            beast::error_code ec,
            tcp::resolver::results_type results);
    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void
    on_ssl_handshake(beast::error_code ec);
    void
    on_handshake(beast::error_code ec);
    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred);
    void
    on_write(
            beast::error_code ec,
            std::size_t bytes_transferred);
    void
    stop() override;
    void
    send(boost::shared_ptr<std::string const> const& ss) override;

    void
    on_close(beast::error_code ec);

    void start_read();
    void start_write();

    bool is_open() const override;

private:
    std::deque<std::string> output_queue_;
    steady_timer dead_line_;
    steady_timer heartbeat_timer_;

    bool get_started() const;

    void
    fail(beast::error_code ec, char const* what);

    void
    deliver(const std::string& msg);

    void
    check_dead_line();
};


#endif //ARCIRK_SOLUTION_SESSION_SSL_H
