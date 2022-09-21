#ifndef ARCIRK_WS_CLIENT_HPP
#define ARCIRK_WS_CLIENT_HPP


#include "net.hpp"
#include "beast.hpp"
#include "arcirk.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

using namespace arcirk;

namespace ssl = boost::asio::ssl;

static void
fail(beast::error_code ec, char const* what)
{
    if(ec == net::ssl::error::stream_truncated)
        return;

    std::cerr << what << " code: " << ec.value() << " : " << ec.message() << "\n";
}

template<class Derived>
class session{

    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_;

public:
    void run(char const* host, char const* port){

    }
};

class plain_session : public session<plain_session>, public std::enable_shared_from_this<plain_session>{
    websocket::stream<beast::tcp_stream> ws_;
public:
    // Create the session
    explicit
    plain_session(
            beast::tcp_stream&& stream)
    : ws_(std::move(stream))
    {
    }
    websocket::stream<beast::tcp_stream>&
    ws()
    {
        return ws_;
    }
};

class ssl_session : public session<ssl_session>, public std::enable_shared_from_this<ssl_session>{
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
public:
    // Create the ssl_websocket_session
    explicit
    ssl_session(
            beast::ssl_stream<beast::tcp_stream>&& stream, ssl::context& ctx)
    : ws_(std::move(stream), ctx)
    {
    }

    websocket::stream<
            beast::ssl_stream<beast::tcp_stream>>&
    ws()
    {
        return ws_;
    }
};

void
make_websocket_session(
        beast::tcp_stream stream){
    std::make_shared<plain_session>(
            std::move(stream));
}
void
make_websocket_session(
        beast::ssl_stream<beast::tcp_stream> stream){
    std::make_shared<ssl_session>(
            std::move(stream));
}

class detect_session : public std::enable_shared_from_this<detect_session>
{
    beast::tcp_stream stream_;
    ssl::context& ctx_;
    beast::flat_buffer buffer_;

public:
    explicit
    detect_session(
            tcp::socket&& socket,
            ssl::context& ctx)
            : stream_(std::move(socket))
            , ctx_(ctx)
    {
    }
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
            std::make_shared<ssl_session>(
                    std::move(stream_),
                    ctx_))->run();
            return;
        }

        // Launch plain session
        std::make_shared<plain_session>(
                std::move(stream_),
                std::move(buffer_))->run();
    }
};

class websocket_client{
    explicit websocket_client(boost::asio::io_context& ioc,
                    boost::asio::ssl::context& ssl_ctx)
            :
            m_io_ctx(ioc),
            m_ssl_ctx(ssl_ctx),
            m_resolver(boost::asio::make_strand(ioc))
    {
    }

    void open(const arcirk::Uri url){

    }

public:
    bool _is_ssl = false;
    boost::asio::io_context& m_io_ctx;
    boost::asio::ssl::context& m_ssl_ctx;
    boost::asio::ip::tcp::resolver m_resolver;
    std::unique_ptr<session> m_session;


};

#endif