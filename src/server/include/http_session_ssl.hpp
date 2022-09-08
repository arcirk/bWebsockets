//
// Created by arcady on 05.07.2021.
//

#ifndef ARCIRK_HTTP_SESSION_SSL_HPP
#define ARCIRK_HTTP_SESSION_SSL_HPP

#include "net.hpp"
#include "beast.hpp"
#include "shared_state.hpp"
#include <boost/optional.hpp>
#include <boost/smart_ptr.hpp>
#include <cstdlib>
#include <memory>

#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace ssl = boost::asio::ssl;

/** Represents an established HTTP connection
*/
class http_session_ssl : public boost::enable_shared_from_this<http_session_ssl>
{
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    boost::shared_ptr<shared_state> state_;
    http::request<http::string_body> req_;
    ssl::context& ctx_;

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<http::request_parser<http::string_body>> parser_;

    struct send_lambda;

    void fail(beast::error_code ec, char const* what);
    void do_read();
    void on_read(beast::error_code ec, std::size_t);
    void on_write(beast::error_code ec, std::size_t, bool close);

public:
    http_session_ssl(
            tcp::socket&& socket,
            ssl::context& ctx,
            boost::shared_ptr<shared_state> const& state);

    void run();

};


#endif //ARCIRK_HTTP_SESSION_SSL_HPP
