
#include "../include/websocket_session_ssl.hpp"
#include <iostream>
#include <utility>
#include <boost/bind.hpp>

websocket_session_ssl::
websocket_session_ssl(tcp::socket&& socket, ssl::context&& ctx, boost::shared_ptr<shared_state>  state)
: ws_(std::move(socket), ctx)
, state_(std::move(state))
, _dead_line(socket.get_executor())
{
    boost::beast::error_code ec;
    auto remote = ws_.next_layer().next_layer().socket().remote_endpoint(ec);
    _ip_address = remote.address().to_string();
    _port = std::to_string(remote.port());
    _uuid = boost::uuids::random_generator()();

    last_error = 0;

}

void websocket_session_ssl::deliver(const boost::shared_ptr<const std::string> &msg) {
    //
    send(msg);
};

websocket_session_ssl::
~websocket_session_ssl()
{
    // Remove this session from the list of active sessions
    state_->leave(this);

}

void
websocket_session_ssl::
fail(beast::error_code ec, char const* what)
{
    // Don't report these
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed)
        return;

    last_error = ec.value();

    if(ec.value() == 10054){
        dead_line_cancel();
    }

    std::cerr << what << " code: " << ec.value() << " " << ec.message() << std::endl;
}

void
websocket_session_ssl::
on_accept(beast::error_code ec)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->join(this);

    std::cout << "connect client" << std::endl;
    if(state_->use_authorization()) {
        //Установка крайнего срока для авторизации
        _dead_line.expires_after(std::chrono::seconds(60));
        _dead_line.async_wait(boost::bind(&websocket_session_ssl::check_dead_line, this));
    }

    // Read a message
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &websocket_session_ssl::on_read,
                    shared_from_this()));
}

void
websocket_session_ssl::
on_run()
{
//    // Set the timeout.
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
            ssl::stream_base::server,
            beast::bind_front_handler(
                    &websocket_session_ssl::on_handshake,
                    shared_from_this()));
}

void
websocket_session_ssl::
on_handshake(beast::error_code ec)
{
    if(ec)
        return fail(ec, "handshake");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
            websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                        " arcirk websocket server");
            }));

    // Accept the websocket handshake
    ws_.async_accept(
            beast::bind_front_handler(
                    &websocket_session_ssl::on_accept,
                    shared_from_this()));
}

void
websocket_session_ssl::
on_read(beast::error_code ec, std::size_t)
{
    if (ec == boost::asio::error::eof){
        std::cerr << "websocket_session::on_read: " << "boost::asio::error::eof" << std::endl;
        return;
    }
    // Handle the error, if any
    if(ec){
        return fail(ec, "read");
    }

    std::string msg = beast::buffers_to_string(buffer_.data());

    state_->send(msg);

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &websocket_session_ssl::on_read,
                    shared_from_this()));
}

void
websocket_session_ssl::
send(boost::shared_ptr<std::string const> const& ss)
{

    net::post(
            ws_.get_executor(),
            beast::bind_front_handler(
                    &websocket_session_ssl::on_send,
                    shared_from_this(),
                    ss));
}


void
websocket_session_ssl::
on_send(boost::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so deliver this immediately
    ws_.async_write(
            net::buffer(*queue_.front()),
            beast::bind_front_handler(
                    &websocket_session_ssl::on_write,
                    shared_from_this()));
}

void
websocket_session_ssl::
on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
                net::buffer(*queue_.front()),
                beast::bind_front_handler(
                        &websocket_session_ssl::on_write,
                        shared_from_this()));
}

std::string websocket_session_ssl::ip_address() const{

    return _ip_address;
}

std::string websocket_session_ssl::host_name() const
{
    return _host_name;
}

void websocket_session_ssl::set_host_name(const std::string &value)
{
    _host_name = value;
}
std::map<boost::uuids::uuid, websocket_session_ssl *>*
websocket_session_ssl::
get_subscribers() {
    return &subscribers_;
}

void
websocket_session_ssl::close() {

    dead_line_cancel();

    ws_.async_close(websocket::close_code::normal,
                beast::bind_front_handler(
                        &websocket_session_ssl::on_close,
                        shared_from_this()));
}
void
websocket_session_ssl::on_close(beast::error_code ec)
{
    if(ec)
        return fail(ec, "close");

    std::cout << beast::make_printable(buffer_.data()) << std::endl;
}
bool websocket_session_ssl::stopped() const
{

    bool result = false;
    try {
        result  = !ws_.next_layer().next_layer().socket().is_open();
    }  catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return result;

}

void
websocket_session_ssl::dead_line_cancel(){
    _dead_line.cancel();
}

void
websocket_session_ssl::check_dead_line()
{
    if(this->last_error < 0)
        return;

    if (this->authorized()){
        _dead_line.cancel();
    }else{
        if (_dead_line.expiry() <= steady_timer::clock_type::now())
        {
            std::string msg = arcirk::local_8bit("Превышено время на авторизацию! Отключение клиента...");

            std::cerr << msg << std::endl;

            ws_.async_close(websocket::close_code::normal,
                            beast::bind_front_handler(
                                    &websocket_session_ssl::on_close,
                                    shared_from_this()));

            _dead_line.expires_at((steady_timer::time_point::max)());
            _dead_line.cancel();
            return;
        }
        // Put the actor back to sleep.
        _dead_line.async_wait(boost::bind(&websocket_session_ssl::check_dead_line, this));
    }

}
