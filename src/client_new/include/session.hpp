#ifndef WSCLIENT_SESSION_HPP
#define WSCLIENT_SESSION_HPP

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

using boost::asio::steady_timer;

template<class Derived>
class session
{
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_;

public:
    void
    run(char const* host, char const* port){
        std::cout << "connect to server..." << std::endl;

        dead_line_.async_wait(std::bind(&session_ssl::check_dead_line, this));

        std::cout << "start resolve " << host << ' ' << port << std::endl;

        // Look up the domain name
        resolver_.async_resolve(
                host,
                port,
                beast::bind_front_handler(
                        &session_ssl::on_resolve,
                        shared_from_this()));
    }

private:

};

class plain_session
        : public session<plain_session>, public std::enable_shared_from_this<plain_session>
        {
    websocket::stream<beast::tcp_stream> ws_;
    tcp::resolver resolver_;
public:
    plain_session::
    plain_session(net::io_context& ioc)
            : resolver_(net::make_strand(ioc))
            , ws_(net::make_strand(ioc))
            , dead_line_(ioc)
            , heartbeat_timer_(ioc)
    {

    }

private:
    std::deque<std::string> output_queue_;
    steady_timer dead_line_;
    steady_timer heartbeat_timer_;

};

class ssl_session
        : public session<ssl_session>
                , public std::enable_shared_from_this<ssl_session>{

    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    tcp::resolver resolver_;
public:
    ssl_session::
    ssl_session(net::io_context& ioc, ssl::context& ctx)
            : resolver_(net::make_strand(ioc))
            , ws_(net::make_strand(ioc), ctx)
            , dead_line_(ioc)
            , heartbeat_timer_(ioc)
    {
    }

private:
    std::deque<std::string> output_queue_;
    steady_timer dead_line_;
    steady_timer heartbeat_timer_;
};

template<class T>
void
make_session(beast::tcp_stream stream)
{
    std::make_shared<plain_session>(
            std::move(stream));
}

template<class T>
void
make_session(beast::ssl_stream<beast::tcp_stream> stream)
{
    std::make_shared<ssl_session>(
            std::move(stream));
}

#endif