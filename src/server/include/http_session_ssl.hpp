#ifndef ARCIRK_HTTP_SESSION1_HPP
#define ARCIRK_HTTP_SESSION1_HPP

#include "net.hpp"
#include "beast.hpp"
#include "shared_state.hpp"
#include <boost/beast/ssl.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <boost/bind/bind.hpp>

namespace ssl = boost::asio::ssl;

class http_session_ssl : public boost::enable_shared_from_this<http_session_ssl>
{

    struct send_lambda
    {
        http_session_ssl& self_;

        explicit
        send_lambda(http_session_ssl& self)
                : self_(self)
        {
        }

        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                    http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                    self_.stream_,
                    *sp,
                    beast::bind_front_handler(
                            &http_session_ssl::on_write,
                            self_.shared_from_this(),
                            sp->need_eof()));
        }
    };

    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    //std::shared_ptr<std::string const> doc_root_;
    //http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;
    boost::shared_ptr<shared_state> state_;
    boost::optional<http::request_parser<http::string_body>> parser_;
    ssl::context& ctx_;

public:
    // Take ownership of the socket
    explicit
    http_session_ssl(
            tcp::socket&& socket,
            ssl::context& ctx,
            boost::shared_ptr<shared_state> const& state);

    void run();
    void
    on_run();
    void
    on_handshake(beast::error_code ec);
    void
    do_read();
    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred);
    void
    on_write(
            bool close,
            beast::error_code ec,
            std::size_t bytes_transferred);
    void
    do_close();

    void
    on_shutdown(beast::error_code ec);

};

#endif