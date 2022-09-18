//
// Created by arcady on 05.07.2021.
//

#ifndef ARCIRK_LISTENER_SSL_HPP
#define ARCIRK_LISTENER_SSL_HPP

#include "beast.hpp"
#include "net.hpp"
#include <boost/smart_ptr.hpp>
#include <memory>
#include <string>
#include <boost/beast/ssl.hpp>

// Forward declaration
class shared_state;

namespace ssl = boost::asio::ssl;

// Accepts incoming connections and launches the sessions
class listener_ssl : public boost::enable_shared_from_this<listener_ssl>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    boost::shared_ptr<shared_state> state_;
    ssl::context& ctx_;

    static void fail(beast::error_code ec, char const* what);

public:
    listener_ssl(
            net::io_context& ioc,
            boost::asio::ssl::context& ctx,
            tcp::endpoint endpoint,
            boost::shared_ptr<shared_state> const& state);


    // Start accepting incoming connections
    void run();

private:
    void
    do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif //ARCIRK_LISTENER_SSL_HPP
