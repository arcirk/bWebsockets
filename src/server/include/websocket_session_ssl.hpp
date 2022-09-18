//
// Created by arcady on 05.07.2021.
//

#ifndef WEBSOCKET_SESSION_SSL_HPP
#define WEBSOCKET_SESSION_SSL_HPP
#include "net.hpp"
#include "beast.hpp"
#include "shared_state.hpp"
#include "subscriber.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio/steady_timer.hpp>

#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "session_base.hpp"

using boost::asio::steady_timer;
// Forward declaration
class shared_state;

namespace ssl = boost::asio::ssl;

/** Represents an active WebSocket connection to the server
*/
class websocket_session_ssl : public session_base, public boost::enable_shared_from_this<websocket_session_ssl>
{
    beast::flat_buffer buffer_;

    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;

    boost::shared_ptr<shared_state> state_;
    std::vector<boost::shared_ptr<std::string const>> queue_;

    std::map<boost::uuids::uuid, websocket_session_ssl*> subscribers_;

    void fail(beast::error_code ec, char const* what) override;
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred) ;
    void on_write(beast::error_code ec, std::size_t bytes_transferred) ;

    int last_error;
public:
    websocket_session_ssl(
            tcp::socket&& socket,
            ssl::context&& ctx,
            boost::shared_ptr<shared_state> state);

    ~websocket_session_ssl() override;

    template<class Body, class Allocator>
    void
    run(http::request<Body, http::basic_fields<Allocator>> req);

    // Send a message
    void
    send(boost::shared_ptr<std::string const> const& ss) override;

//    boost::uuids::uuid& get_uuid() override;
//    boost::uuids::uuid & get_user_uuid() override;
//    const std::string & get_role() override;

    void deliver(const boost::shared_ptr<const std::string> &msg) override;

    std::map<boost::uuids::uuid, websocket_session_ssl*>* get_subscribers();

    //void throw_authorized() override;

    void close() override;
    bool stopped() const override;

    void dead_line_cancel();

    std::string ip_address() const;
    std::string host_name() const;

    void
    on_run();
    void
    on_handshake(beast::error_code ec);

private:

    std::string _ip_address = "0.0.0.0";
    std::string _port = "0";
    steady_timer _dead_line;
    std::string _host_name;

    void
    on_send(boost::shared_ptr<std::string const> const& ss) ;
    void
    on_close(beast::error_code ec) ;

    void check_dead_line();

    void set_host_name(const std::string& value);
};

template<class Body, class Allocator>
void
websocket_session_ssl::
run(http::request<Body, http::basic_fields<Allocator>> req)
{
//    // Set suggested timeout settings for the websocket
//    ws_.set_option(
//            websocket::stream_base::timeout::suggested(
//                    beast::role_type::server));
//
//    // Set a decorator to change the Server of the handshake
//    ws_.set_option(websocket::stream_base::decorator(
//            [](websocket::response_type& res)
//            {
//                res.set(http::field::server,
//                        std::string(BOOST_BEAST_VERSION_STRING) +
//                        "arcirk websocket");
//            }));
//
//    // Accept the websocket handshake
//    ws_.async_accept(
//            req,
//            beast::bind_front_handler(
//                    &websocket_session_ssl::on_accept,
//                    shared_from_this()));

//    SSL_CTX_set_session_id_context()
//
//    beast::get_lowest_layer(ws_).co
//    sslSocket2.lowest_layer().connect( tcpEndpoint, ec );
//    SSLSocket::impl_type impl1 = sslSocket1.impl();
//    SSLSocket::impl_type impl2 = sslSocket2.impl();
//    SSL_SESSION *savedSession = SSL_get1_session( impl1->ssl );
//    SSL_set_session( impl2->ssl, savedSession );
//    SSL_connect( impl2->ssl );

    net::dispatch(ws_.get_executor(),
                  beast::bind_front_handler(
                          &websocket_session_ssl::on_run,
                          shared_from_this()));
}
#endif //WEBSOCKET_SESSION_SSL_HPP
