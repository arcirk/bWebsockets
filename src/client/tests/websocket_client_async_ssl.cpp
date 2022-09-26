//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket SSL client, asynchronous
//
//------------------------------------------------------------------------------

#include "../../arcirk/common/root_certificates.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

template<class Derived>
class session{
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_;

};
class session_plain : public std::enable_shared_from_this<session_plain>, public session<session_plain>
{

    websocket::stream<beast::tcp_stream> ws_;
public:
    explicit
    session_plain(net::io_context& ioc)
    : resolver_(net::make_strand(ioc))
    {
    }

    void
    run(
            char const* host,
            char const* port,
            char const* text)
    {

    }
};
// Sends a WebSocket message and prints the response
class session_ssl : public std::enable_shared_from_this<session_ssl>, public session<session_ssl>
{
    tcp::resolver resolver_;
    websocket::stream<
        beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;

public:
    // Resolver and socket require an io_context
    explicit
    session_ssl(net::io_context& ioc, ssl::context& ctx)
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc), ctx)
    {
    }

    // Start the asynchronous operation
    void
    run(
        char const* host,
        char const* port,
        char const* text)
    {
        // Save these for later
        host_ = host;
        text_ = text;

        std::cout << "run" << std::endl;

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &session_ssl::on_resolve,
                shared_from_this()));
    }

    void
    on_resolve(
        beast::error_code ec,
        tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Set a timeout on the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        std::cout << "on_resolve" << std::endl;

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &session_ssl::on_connect,
                shared_from_this()));
    }

    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if(ec)
            return fail(ec, "connect");

        // Set a timeout on the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(
                ws_.next_layer().native_handle(),
                host_.c_str()))
        {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                net::error::get_ssl_category());
            return fail(ec, "connect");
        }

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host_ += ':' + std::to_string(ep.port());

        std::cout << "on_connect" << std::endl;

        // Perform the SSL handshake
        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(
                &session_ssl::on_ssl_handshake,
                shared_from_this()));
    }

    void
    on_ssl_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "ssl_handshake");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async-ssl");
            }));

        std::cout << "on_ssl_handshake" << std::endl;

        // Perform the websocket handshake
        ws_.async_handshake(host_, "/",
            beast::bind_front_handler(
                &session_ssl::on_handshake,
                shared_from_this()));
    }

    void
    on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        std::cout << "on_handshake" << std::endl;

        // Send the message
        ws_.async_write(
            net::buffer(text_),
            beast::bind_front_handler(
                &session_ssl::on_write,
                shared_from_this()));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        std::cout << "on_write" << std::endl;

        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &session_ssl::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        std::cout << "on_message" << std::endl;

        // Close the WebSocket connection
        ws_.async_close(websocket::close_code::normal,
            beast::bind_front_handler(
                &session_ssl::on_close,
                shared_from_this()));
    }

    void
    on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << "on_close" << std::endl;
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }
};

//------------------------------------------------------------------------------
class detect_session : public std::enable_shared_from_this<detect_session>
{
    beast::tcp_stream stream_;
    ssl::context& ctx_;
    beast::flat_buffer buffer_;
    tcp::resolver resolver_;
public:
    explicit
    detect_session(
            tcp::socket&& socket,
            ssl::context& ctx)
            : stream_(std::move(socket))
            , ctx_(ctx)
            , resolver_(net::make_strand(ioc))
    {}

    void
    run()
    {
        net::dispatch(
                stream_.get_executor(),
                beast::bind_front_handler(
                        &detect_session::on_run,
                        this->shared_from_this()));
    }
    void
    on_run()
    {
        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        beast::async_detect_ssl(
                stream_,
                buffer_,
                beast::bind_front_handler(
                        &detect_session::on_detect,
                        this->shared_from_this()));
    }

    void
    on_detect(beast::error_code ec, bool result)
    {
        if(ec)
            return fail(ec, "detect");

        if(result)
        {
            // Launch SSL session
            std::make_shared<session_ssl>(
                    std::move(stream_),
                    ctx_)->run();
            return;
        }

        // Launch plain session
        std::make_shared<session_plain>(
                std::move(stream_))->run();
    }
};

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 4)
    {
        std::cerr <<
            "Usage: websocket-client-async-ssl <host> <port> <text>\n" <<
            "Example:\n" <<
            "    websocket-client-async-ssl echo.websocket.org 443 \"Hello, world!\"\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // This holds the root certificate used for verification
    load_root_default_certificates(ctx);

    // Launch the asynchronous operation
    std::make_shared<session_plain>(ioc, ctx)->run(host, port, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}