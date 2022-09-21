//
// Created by arcady on 13.07.2021.
//
#include "net.hpp"
#include "beast.hpp"
#include <iostream>
#include "../../include/session.h"
#include "../../include/client.hpp"


session::
session(net::io_context& ioc, const std::string & auth)
: resolver_(net::make_strand(ioc))
, ws_(net::make_strand(ioc))
, dead_line_(ioc)
, heartbeat_timer_(ioc)
{
    _auth = auth;
    _is_ssl = false;
}

session::~session() = default;

// Start the asynchronous operation
void
session::run(
        char const* host,
        char const* port, ws_client * client)
{

    client_ = client;

    // Save these for later
    host_ = host;

    deliver("\n");

    std::cout << "connect to server..." << std::endl;

    dead_line_.async_wait(std::bind(&session::check_dead_line, this));

    std::cout << "start resolve " << host << ' ' << port << std::endl;

    // Look up the domain name
    resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                    &session::on_resolve,
                    shared_from_this()));
}
void
session::on_resolve(
        beast::error_code ec,
        tcp::resolver::results_type results)
{
    if(ec)
        return fail(ec, "resolve");

    // Set the timeout for the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(15));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                    &session::on_connect,
                    shared_from_this()));
}
void
session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if (ec.value() == 111 || ec.value() == 10061){ //connection refused
        std::string err = ec.what();
        client_->on_error("connect", arcirk::to_utf(err), ec.value());
        return;
    }

    if(ec){
        return fail(ec, "connect");
    }
    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
            websocket::stream_base::timeout::suggested(
                    beast::role_type::client));

    std::string __auth = _auth;
    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
            [&__auth](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async");
                if(!__auth.empty()){
                    req.set(http::field::authorization, __auth);
                }
            }));

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/",
                        beast::bind_front_handler(
                                &session::on_handshake,
                                shared_from_this()));
                }

void
session::on_handshake(beast::error_code ec)
{

    if(ec){
        if(ec.value() == 20){
            std::cerr << arcirk::local_8bit("Подключение было отклонено удаленным узлом. Ошибка авторизации или сбой на сервере.") << std::endl;
            return;
        }else
            return fail(ec, "handshake");
    }


    std::cout << "session::on_connect: successful connection!" << std::endl;

    client_->on_connect(this);

    started_ = true;

    start_read();

}

void
session::start_read()
{

    if(!ws_.is_open())
        return;

    dead_line_.expires_after(std::chrono::seconds(30));

    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
}

void
session::on_read(
        beast::error_code ec,
        std::size_t bytes_transferred){

    boost::ignore_unused(bytes_transferred);

    std::string err = ec.what();
    std::string what = "on_read";


    if(ec){
        dead_line_.cancel();
    }

    if (ec == boost::asio::error::connection_reset){
        //std::cout << "boost::asio::error::connection_reset " << "error code:" << ec.value() << std::endl;
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        return;
    }else if( ec==boost::asio::error::eof){
        //std::cout << "boost::asio::error::eof " << "error code:" << ec.value() << std::endl;
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        started_ = false;
        return;
    }else if( ec==websocket::error::no_connection){
        //std::cout << "websocket::error::no_connection " << "error code:" << ec.value() << std::endl;
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        return;
    }else if(ec == websocket::error::closed){
        //std::cout << "websocket::error::closed " << "error code:" << ec.value() << std::endl;
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        started_ = false;
        return;
    }

    if(ec.value() == 995){
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        return;
    }

    if(ec.value() == 2){
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        return;
    }

    //125 : Операция отменена
    if(ec.value() == 109 || ec.value() == 125){
        client_->on_error(what, arcirk::to_utf(err), ec.value());
        return;
    }

    if(ec){
        return fail(ec, "read");
    }

    client_->on_read(beast::buffers_to_string(buffer_.data()));

    buffer_.consume(buffer_.size());

    start_read();

}

void
session::start_write()
{
    if ( !get_started()) return;

    ws_.async_write(
            net::buffer(output_queue_.front()),
            beast::bind_front_handler(
                    &session::on_write,
                    shared_from_this()));
}

void
session::on_write(
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
session::stop()
{
    if ( !started_) return;
    started_ = false;
    dead_line_.cancel();
    heartbeat_timer_.cancel();
    //if(!eraseObjOnly)
        ws_.async_close(websocket::close_code::normal,
            beast::bind_front_handler(
                    &session::on_close,
                    shared_from_this()));
}

void
session::on_close(beast::error_code ec)
{

    if(ec)
        return fail(ec, "close");

    client_->on_stop();

}

void
session::send(boost::shared_ptr<std::string const> const& ss) {


    if (!ws_.is_open())
        return;

    deliver(ss->c_str());

    //сбрасываем таймер для отправки следующего сообщения через секунду
    heartbeat_timer_.expires_after(std::chrono::seconds(0));
    heartbeat_timer_.async_wait(std::bind(&session::start_write, this));

}

void
session::fail(beast::error_code ec, char const* what)
{

    std::string _msg = ec.what();

    std::cerr << what << ": " << _msg << "\n";

    client_->on_error(what, arcirk::to_utf(_msg), ec.value());

}

void
session::deliver(const std::string& msg)
{
    output_queue_.clear();
    if (msg == "\n" || msg == "pong"){
        output_queue_.push_back("\n");
    }else
        output_queue_.push_back(msg + "\n");
}

void
session::check_dead_line()
{

    if(!ws_.is_open())
        return;

    if (dead_line_.expiry() <= steady_timer::clock_type::now())
    {
        ws_.async_close(websocket::close_code::normal,
                        beast::bind_front_handler(
                                &session::on_close,
                                shared_from_this()));

        dead_line_.expires_at((steady_timer::time_point::max)());
    }

    dead_line_.async_wait(std::bind(&session::check_dead_line, this));

}

bool session::is_open() const{
    return ws_.is_open();
}

bool session::get_started() const{
    return started_;
}
