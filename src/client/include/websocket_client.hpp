#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <arcirk.hpp>
#include <net.hpp>
#include <beast.hpp>
#include <common/root_certificates.hpp>
#include "shared_state.hpp"

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
#include <deque>

namespace ssl = boost::asio::ssl;

using boost::asio::steady_timer;

static void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

template<class Derived>
class session {

    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

public:

    void do_connect(tcp::resolver::results_type results){
        beast::get_lowest_layer(derived().ws()).expires_after(std::chrono::seconds(30));
        beast::get_lowest_layer(derived().ws()).async_connect(
                results,
                beast::bind_front_handler(
                        &session::on_connect,
                        derived().shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep){

        if (ec.value() == 111 || ec.value() == 10061){ //connection refused
            std::string err = ec.what();
            //client_->on_error("connect", arcirk::to_utf(err), ec.value());
            std::cerr << err << std::endl;
            return;
        }

        if(ec){
            return fail(ec, "connect");
        }
        derived().ws().set_option(
                websocket::stream_base::timeout::suggested(
                        beast::role_type::client));

        std::string __auth = "";
        // Set a decorator to change the User-Agent of the handshake
        derived().ws().set_option(websocket::stream_base::decorator(
                [&__auth](websocket::request_type& req)
                {
                    req.set(http::field::user_agent,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                            " arcirk-client-async");
                    if(!__auth.empty()){
                        req.set(http::field::authorization, __auth);
                    }
                }));

        derived().connect(ec, ep);
    }

    void run(tcp::resolver::results_type results){
        do_connect(results);
    }
};

class plain_session : public std::enable_shared_from_this<plain_session>, public session<plain_session> {
    websocket::stream<beast::tcp_stream> ws_;
    std::string host_;
    bool started_ = false;
    beast::flat_buffer buffer_;
    boost::shared_ptr<shared_state> state_;
    steady_timer heartbeat_timer_;
    steady_timer dead_line_;
public:
    plain_session(net::io_context& ioc, const std::string& host, boost::shared_ptr<shared_state> sharedPtr)
            : ws_(net::make_strand(ioc))
            , state_(std::move(sharedPtr))
            , heartbeat_timer_(ioc)
            , dead_line_(ioc)
    {host_ = host;}

    websocket::stream<beast::tcp_stream>&
    ws()
    {
        return ws_;
    }

    void
    connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        host_ += ':' + std::to_string(ep.port());

        deliver("\n");

        // Perform the websocket handshake
        ws_.async_handshake(host_, "/",
                            beast::bind_front_handler(
                                    &plain_session::on_handshake,
                                    shared_from_this()));
    }

    void
    on_handshake(beast::error_code ec)
    {

        if(ec){
            if(ec.value() == 20){
                //std::cerr << arcirk::local_8bit("Подключение было отклонено удаленным узлом. Ошибка авторизации или сбой на сервере.") << std::endl;
                return;
            }else
                return fail(ec, "handshake");
        }


        std::cout << "session::on_connect: successful connection!" << std::endl;

        state_->on_connect(this);

        started_ = true;

        start_write();
        start_read();

    }
    void
    start_read()
    {

        if(!ws_.is_open())
            return;

        dead_line_.expires_after(std::chrono::seconds(30));

        ws_.async_read(
                buffer_,
                beast::bind_front_handler(
                        &plain_session::on_read,
                        shared_from_this()));
    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred){

        boost::ignore_unused(bytes_transferred);

        std::string err = ec.what();
        std::string what = "on_message";


        if(ec){
            dead_line_.cancel();
        }

        if (ec == boost::asio::error::connection_reset){
            //std::cout << "boost::asio::error::connection_reset " << "error code:" << ec.value() << std::endl;
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            return;
        }else if( ec==boost::asio::error::eof){
            //std::cout << "boost::asio::error::eof " << "error code:" << ec.value() << std::endl;
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            started_ = false;
            return;
        }else if( ec==websocket::error::no_connection){
            //std::cout << "websocket::error::no_connection " << "error code:" << ec.value() << std::endl;
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            return;
        }else if(ec == websocket::error::closed){
            //std::cout << "websocket::error::closed " << "error code:" << ec.value() << std::endl;
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            started_ = false;
            return;
        }

        if(ec.value() == 995){
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            return;
        }

        if(ec.value() == 2){
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            return;
        }

        //125 : Операция отменена
        if(ec.value() == 109 || ec.value() == 125){
            state_->on_error(what, arcirk::to_utf(err), ec.value());
            return;
        }

        if(ec){
            return fail(ec, "read");
        }

        state_->on_message(beast::buffers_to_string(buffer_.data()));

        buffer_.consume(buffer_.size());

        start_read();

    }

    void
    start_write()
    {
        if ( !started()) return;

        ws_.async_write(
                net::buffer(output_queue_.front()),
                beast::bind_front_handler(
                        &plain_session::on_write,
                        shared_from_this()));
    }

    void
    on_write(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {

        if(ec){
            std::cerr << ec.value() << std::endl;
        }

        if (ec == boost::asio::error::connection_reset){
            return;
        }

        boost::ignore_unused(bytes_transferred);

        if(ec == websocket::error::closed){
            return;
        }

        if(ec)
            return fail(ec, "write");

        buffer_.consume(buffer_.size());

        output_queue_.pop_front();

        if (output_queue_.empty()) {
            output_queue_.emplace_back("\n");
        }
    }

    void
    stop()
    {
        if ( !started_) return;
        started_ = false;
        dead_line_.cancel();
        heartbeat_timer_.cancel();

        ws_.async_close(websocket::close_code::normal,
                        beast::bind_front_handler(
                                &plain_session::on_close,
                                shared_from_this()));
    }

    void
    on_close(beast::error_code ec)
    {

        if(ec)
            return fail(ec, "close");

        state_->on_stop();

    }

    void
    send(boost::shared_ptr<std::string const> const& ss) {


        if (!ws_.is_open())
            return;

        deliver(ss->c_str());

        //сбрасываем таймер для отправки следующего сообщения через секунду
        heartbeat_timer_.expires_after(std::chrono::seconds(0));
        heartbeat_timer_.async_wait(std::bind(&plain_session::start_write, this));

    }

//    void
//    plain_session::fail(beast::error_code ec, char const* what)
//    {
//
//        std::string _msg = ec.what();
//
//        std::cerr << what << ": " << _msg << "\n";
//
//        client_->on_error(what, arcirk::to_utf(_msg), ec.value());
//
//    }

    void
    deliver(const std::string& msg)
    {
        output_queue_.clear();
        if (msg == "\n" || msg == "pong"){
            output_queue_.push_back("\n");
        }else
            output_queue_.push_back(msg + "\n");
    }

    void
    check_dead_line()
    {

        if(!ws_.is_open())
            return;

        if (dead_line_.expiry() <= steady_timer::clock_type::now())
        {
            ws_.async_close(websocket::close_code::normal,
                            beast::bind_front_handler(
                                    &plain_session::on_close,
                                    shared_from_this()));

            dead_line_.expires_at((steady_timer::time_point::max)());
        }

        dead_line_.async_wait(std::bind(&plain_session::check_dead_line, this));

    }

    bool is_open() const{
        return ws_.is_open();
    }

    bool started() const{
        return started_;
    }

private:
    std::deque<std::string> output_queue_;

};

class ssl_session : public std::enable_shared_from_this<ssl_session>, public session<ssl_session> {
    websocket::stream<
            beast::ssl_stream<beast::tcp_stream>> ws_;
    std::string host_;
    beast::flat_buffer buffer_;
    boost::shared_ptr<shared_state> state_;
    bool started_ = false;
    steady_timer heartbeat_timer_;
    steady_timer dead_line_;

public:
    ssl_session(net::io_context& ioc, ssl::context& ctx, const std::string& host, boost::shared_ptr<shared_state> sharedPtr)
            : ws_(net::make_strand(ioc), ctx)
            , state_(std::move(sharedPtr))
            , heartbeat_timer_(ioc)
            , dead_line_(ioc)
    {host_ = host;}

    websocket::stream<
            beast::ssl_stream<beast::tcp_stream>>&
    ws()
    {
        return ws_;
    }

    void
    connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {

        if (ec.value() == 111 || ec.value() == 10061){ //connection refused
            std::string err = ec.message();
#ifdef _WINDOWS
            err = "В соединении отказано";
#endif
            state_->on_error("connect", err, ec.value());
            return;
        }

        if (ec)
            return fail(ec, "connect");

        // Set a timeout on the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(
                ws_.next_layer().native_handle(),
                host_.c_str())) {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                                   net::error::get_ssl_category());
            return fail(ec, "connect");
        }

        host_ += ':' + std::to_string(ep.port());

        deliver("\n");

        dead_line_.async_wait(std::bind(&ssl_session::check_dead_line, this));

        // Perform the SSL handshake
        ws_.next_layer().async_handshake(
                ssl::stream_base::client,
                beast::bind_front_handler(
                        &ssl_session::on_ssl_handshake,
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

       // std::cout << "on_ssl_handshake" << std::endl;

        // Perform the websocket handshake
        ws_.async_handshake(host_, "/",
                            beast::bind_front_handler(
                                    &ssl_session::on_handshake,
                                    shared_from_this()));
    }

    void
    on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        //std::cout << "on_handshake" << std::endl;

        state_->on_connect(this);

        started_ = true;

        start_read();

//        // Send the message
//        ws_.async_write(
//                net::buffer("test ssl connection"),
//                beast::bind_front_handler(
//                        &ssl_session::on_write,
//                        shared_from_this()));
    }

    void
    start_write()
    {
        if ( !started()) return;

        ws_.async_write(
                net::buffer(output_queue_.front()),
                beast::bind_front_handler(
                        &ssl_session::on_write,
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

//        // Read a message into our buffer
//        ws_.async_read(
//                buffer_,
//                beast::bind_front_handler(
//                        &ssl_session::on_read,
//                        shared_from_this()));
    }
    void
    start_read()
    {

        if(!ws_.is_open())
            return;

        dead_line_.expires_after(std::chrono::seconds(30));

        ws_.async_read(
                buffer_,
                beast::bind_front_handler(
                        &ssl_session::on_read,
                        shared_from_this()));
    }
    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec){
            dead_line_.cancel();
        }

        if (ec == boost::asio::error::connection_reset){
            std::cout << "session::on_read: " << "boost::asio::error::connection_reset" << std::endl;
            return;
        }else if( ec==boost::asio::error::eof){
            state_->on_error("session::on_read", "boost::asio::error::eof", ec.value());
            started_ = false;
            return;
        }else if( ec==websocket::error::no_connection){
            std::cout << "session::on_read: " << "websocket::error::no_connection" << std::endl;
            return;
        }

        if(ec == websocket::error::closed){
            state_->on_error("read","Сервер не доступен!", ec.value());
            started_ = false;
            return;
        }

        if(ec.value() == 995){
            std::string err = "I/O operation was aborted";
            std::cerr << "session::on_read: " << err << std::endl;
            //client_->error("read", err);
            return;
        }

        if(ec.value() == 2){
            std::string err = ec.message();
#ifdef _WINDOWS
            err = "Соединение разорвано!";
#endif
            state_->on_error("read", err, ec.value());
            return;
        }

        //125 : Операция отменена
        if(ec.value() == 109 || ec.value() == 125){
            std::string err = ec.message();
#ifdef _WINDOWS
            err = "Соединение разорвано другой стороной!";
#endif

            if (!ws_.is_open())
                return;

            state_->on_error("read", err, ec.value());

            return;
        }
        if(ec)
            return fail(ec, "read");

        std::cout << "on_message" << std::endl;

        state_->on_message(beast::buffers_to_string(buffer_.data()));

        buffer_.consume(buffer_.size());

        start_read();
//        // Close the WebSocket connection
//        ws_.async_close(websocket::close_code::normal,
//                        beast::bind_front_handler(
//                                &ssl_session::on_close,
//                                shared_from_this()));
    }

    void
    on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");

        state_->on_stop();
        // If we get here then the connection is closed gracefully

//        // The make_printable() function helps print a ConstBufferSequence
//        std::cout << "on_close" << std::endl;
//        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

    void
    deliver(const std::string& msg)
    {
        output_queue_.clear();
        if (msg == "\n" || msg == "pong"){
            output_queue_.push_back("\n");
        }else
            output_queue_.push_back(msg + "\n");
    }

    void
    stop()
    {
        if ( !started_) return;
        started_ = false;
        dead_line_.cancel();
        heartbeat_timer_.cancel();

        ws_.async_close(websocket::close_code::normal,
                    beast::bind_front_handler(
                            &ssl_session::on_close,
                            shared_from_this()));
    }

    void
    send(boost::shared_ptr<std::string const> const& ss) {

        //if ( !started_) return;

        if (!ws_.is_open())
            return;

        deliver(ss->c_str());

        //сбрасываем таймер для отправки следующего сообщения через секунду
        heartbeat_timer_.expires_after(std::chrono::seconds(0));
        heartbeat_timer_.async_wait(std::bind(&ssl_session::start_write, this));

    }

    bool started() const{
        return started_;
    }

    bool is_open() const{
        return ws_.is_open();
    }

    void
    check_dead_line()
    {

        if(!ws_.is_open())
            return;

        if (dead_line_.expiry() <= steady_timer::clock_type::now())
        {
            ws_.async_close(websocket::close_code::normal,
                            beast::bind_front_handler(
                                    &ssl_session::on_close,
                                    shared_from_this()));

            dead_line_.expires_at((steady_timer::time_point::max)());
        }

        dead_line_.async_wait(std::bind(&ssl_session::check_dead_line, this));

    }

private:
    std::deque<std::string> output_queue_;

};

class resolver : public std::enable_shared_from_this<resolver>{

    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::resolver resolver_;
    std::string host_;
    std::string port_;
    bool _is_ssl = false;
    boost::shared_ptr<shared_state> state_;

public:
    resolver(
            net::io_context& ioc,
            ssl::context& ctx,
            boost::shared_ptr<shared_state> const& state)
            : ioc_(ioc)
            , ctx_(ctx)
            , resolver_(net::make_strand(ioc))
            , state_(state)
    {

    }

    void
    run(char const* host,
        char const* port, bool is_ssl)
    {
        host_ = host;
        port_ = port;
        _is_ssl = is_ssl;

        std::cout << "connect to server..." << std::endl;

        do_resolve();
    }

    void do_resolve(){

        std::cout << "start resolve " << host_ << ' ' << port_ << std::endl;

        resolver_.async_resolve(
                host_,
                port_,
                beast::bind_front_handler(
                        &resolver::on_resolve,
                        shared_from_this()));
    }

    void
    on_resolve(
            beast::error_code ec,
            tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        //std::cout << "on_resolve" << std::endl;


        if(_is_ssl){
            auto sess = std::make_shared<ssl_session>(ioc_, ctx_, host_, state_);
            sess->run(results);
        }else
            std::make_shared<plain_session>(ioc_, host_, state_)->run(results);


//        beast::get_lowest_layer(sess->ws()).expires_after(std::chrono::seconds(30));
//        beast::get_lowest_layer(sess->ws()).async_connect(
//        results,
//        beast::bind_front_handler(
//                &ssl_session::on_connect,
//                sess->shared_from_this()));


//        // Set a timeout on the operation
//        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
//
//        std::cout << "on_resolve" << std::endl;
//
//        // Make the connection on the IP address we get from a lookup
//        beast::get_lowest_layer(ws_).async_connect(
//                results,
//                beast::bind_front_handler(
//                        &session_ssl::on_connect,
//                        shared_from_this()));
    }
};

#endif