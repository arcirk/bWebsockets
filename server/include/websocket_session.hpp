#ifndef  BWEBSOCKETS_WEBSOCKETS_SESSION_HPP
#define BWEBSOCKETS_WEBSOCKETS_SESSION_HPP

#include "http.hpp"
#include "shared_state.hpp"
#include <arcirk.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>

using boost::asio::steady_timer;

class subscriber{

public:

   virtual std::tm start_date() const{
        return start_date_;
   }

    [[nodiscard]] virtual std::string user_name() const{
        return _user_name;
    }

    [[nodiscard]] virtual boost::uuids::uuid user_uuid() const {
        return _user_uuid;
    }

    [[nodiscard]] virtual boost::uuids::uuid uuid_session() const {
        return _uuid_session;
    }

    [[nodiscard]] virtual std::string app_name() const{
        return _app_name;
    }

    virtual void set_app_name(const std::string& value){
       _app_name = value;
    }

    virtual void set_uuid_session(const boost::uuids::uuid& value){
        _uuid_session = value;
    }

    virtual void set_user_name(const std::string& value){
        _user_name = value;
    }
    virtual void set_user_uuid(const boost::uuids::uuid& value){
        _user_uuid = value;
    }
    virtual void send(boost::shared_ptr<std::string const> const& ss) = 0;

    virtual bool is_ssl() = 0;

    [[nodiscard]] virtual bool authorized() const{
        return authorized_;
    };

    virtual void set_authorized(bool value){
        authorized_ = value;
    }

    template<class Derived>
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }

protected:
    std::string _user_name = UnknownUser;
    boost::uuids::uuid _user_uuid{};
    boost::uuids::uuid _uuid_session{};
    bool authorized_ = false;
    std::tm start_date_{};
    std::string _app_name = UnknownApplication;
};

template<class Derived>
class websocket_session
{
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_;
    std::vector<boost::shared_ptr<std::string const>> queue_;

    // Start the asynchronous operation
    template<class Body, class Allocator>
    void
    do_accept(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Set suggested timeout settings for the websocket
        derived().ws().set_option(
                websocket::stream_base::timeout::suggested(
                        beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        derived().ws().set_option(
                websocket::stream_base::decorator(
                        [](websocket::response_type& res)
                        {
                            res.set(http::field::server,
                                    std::string(BOOST_BEAST_VERSION_STRING) +
                                    " arcirk-server");
                        }));

        // Accept the websocket handshake
        derived().ws().async_accept(
                req,
                beast::bind_front_handler(
                        &websocket_session::on_accept,
                        derived().shared_from_this()));
    }

    void
    on_accept(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "accept");

        derived().set_uuid_session(arcirk::uuids::random_uuid());
        derived().join();
        if(derived().state()->use_authorization() && !derived().authorized()){
            if(derived().state()->allow_delayed_authorization()){
                //Запускаем таймер ожидания авторизации
                derived().dead_line().expires_after(std::chrono::seconds(60));
                derived().dead_line().async_wait(boost::bind(&websocket_session::check_dead_line, derived().shared_from_this()));
            }

        }

        // Read a message
        do_read();
    }

    void
    check_dead_line()
    {
        if(this->last_error < 0)
            return;

        if (derived().authorized()){
            derived().dead_line().cancel();
        }else{
            if (derived().dead_line().expiry() <= steady_timer::clock_type::now())
            {
                beast::error_code ec{};
                fail(ec, "Превышено время на авторизацию! Отключение клиента...");
                derived().ws().async_close(websocket::close_code::normal,
                                beast::bind_front_handler(
                                        &websocket_session::on_close,
                                        derived().shared_from_this()));

                derived().dead_line().expires_at((steady_timer::time_point::max)());
                derived().dead_line().cancel();
                return;
            }
            // Put the actor back to sleep.
            derived().dead_line().async_wait(std::bind(&websocket_session::check_dead_line, derived().shared_from_this()));
        }

    }

    void
    on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

    void close() {

        derived().dead_line().cancel();

        derived().ws().async_close(websocket::close_code::normal,
                        beast::bind_front_handler(
                                &websocket_session::on_close,
                                derived().shared_from_this()));
    }

    void
    do_read()
    {
        buffer_.consume(buffer_.size());
        // Read a message into our buffer
        derived().ws().async_read(
                buffer_,
                beast::bind_front_handler(
                        &websocket_session::on_read,
                        derived().shared_from_this()));
    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec == boost::asio::error::eof){
            std::cerr << "websocket_session::on_read: " << "boost::asio::error::eof" << std::endl;
            return;
        }

        // This indicates that the websocket_session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            return fail(ec, "read");

        derived().deliver(beast::buffers_to_string(buffer_.data()));

        do_read();

    }

    void do_write(){

        queue_.erase(queue_.begin());

        if(!queue_.empty())
            derived().ws().async_write(
                net::buffer(*queue_.front()),
                beast::bind_front_handler(
                        &websocket_session::on_write,
                        derived().shared_from_this()));
    }

    void
    on_write(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        do_write();

    }

    void
    on_send(boost::shared_ptr<std::string const> const& ss)
    {
        // Always add to queue
        queue_.push_back(ss);

        // Are we already writing?
        if(queue_.size() > 1)
            return;

        // We are not currently writing, so deliver this immediately
        derived().ws().async_write(
                net::buffer(*queue_.front()),
                beast::bind_front_handler(
                        &websocket_session::on_write,
                        derived().shared_from_this()));
    }

public:
    // Start the asynchronous operation
    template<class Body, class Allocator>
    void
    run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        if(derived().state()->use_authorization()){
            if(!derived().state()->allow_delayed_authorization() && !derived().authorized()){
                beast::error_code ec{};
                return fail(ec, "Unauthorized");
            }

        }
        // Accept the WebSocket upgrade request
        do_accept(std::move(req));
    }

    void
    send_message(boost::shared_ptr<std::string const> const& ss)
    {
        net::post(
                derived().ws().get_executor(),
                beast::bind_front_handler(
                        &websocket_session::on_send,
                        derived().shared_from_this(),
                        ss));
    }

protected:
    int last_error;
//    auto super()
//    {
//        return this;
//    }
};

//------------------------------------------------------------------------------

// Handles a plain WebSocket connection
class plain_websocket_session
        : public websocket_session<plain_websocket_session>
                , public std::enable_shared_from_this<plain_websocket_session>, public subscriber
{
    websocket::stream<beast::tcp_stream> ws_;
    boost::shared_ptr<shared_state> state_;
    steady_timer dead_line_;
public:
    // Create the session
    explicit
    plain_websocket_session(
            beast::tcp_stream&& stream, boost::shared_ptr<shared_state> sharedPtr, bool is_http_authorization)
            : ws_(std::move(stream))
            , state_(std::move(sharedPtr))
            , dead_line_(stream.socket().get_executor())
    {
        authorized_ = is_http_authorization;
        last_error = 0;
    }

    ~plain_websocket_session(){
        state_->leave(uuid_session(), user_name());
    }

    // Called by the base class
    websocket::stream<beast::tcp_stream>&
    ws()
    {
        return ws_;
    }

    boost::shared_ptr<shared_state>
    state(){
        return state_;
    }

    steady_timer&
    dead_line(){
        return dead_line_;
    }

    void join(){
        start_date_ = arcirk::current_date();
        state_->join(this);
    }

    void send(boost::shared_ptr<std::string const> const& ss) override{
       auto _super = boost::dynamic_pointer_cast<plain_websocket_session>(shared_from_this());
        _super->send_message(ss);
        //super()->send(ss);
//        net::post(
//                ws_.get_executor(),
//                beast::bind_front_handler(
//                        &websocket_session::on_send,
//                        derived().shared_from_this(),
//                        ss));
    }

    void deliver(const std::string& message){
        state_->deliver(message, this);
    }

    bool is_ssl() override{
        return false;
    }

    void set_authorized(bool value) override{
        authorized_ = value;
        if(value)
            dead_line_.cancel();

    }
};

//------------------------------------------------------------------------------

// Handles an SSL WebSocket connection
class ssl_websocket_session
        : public websocket_session<ssl_websocket_session>
                , public std::enable_shared_from_this<ssl_websocket_session>, public subscriber
{
    websocket::stream<
    beast::ssl_stream<beast::tcp_stream>> ws_;
    boost::shared_ptr<shared_state> state_;
    steady_timer dead_line_;
public:
    // Create the ssl_websocket_session
    explicit
    ssl_websocket_session(
            beast::ssl_stream<beast::tcp_stream>&& stream, boost::shared_ptr<shared_state> sharedPtr, bool is_http_authorization)
            : ws_(std::move(stream))
            , state_(std::move(sharedPtr))
            , dead_line_(stream.next_layer().socket().get_executor())
    {
        authorized_ = is_http_authorization;
        last_error = 0;
    }

    ~ssl_websocket_session(){
        state_->leave(uuid_session(), user_name());
    }

    // Called by the base class
    websocket::stream<
    beast::ssl_stream<beast::tcp_stream>>&
    ws()
    {
        return ws_;
    }

    boost::shared_ptr<shared_state>
    state(){
        return state_;
    }

    void join(){
        start_date_ = arcirk::current_date();
        state_->join(this);
    }

    void send(boost::shared_ptr<std::string const> const& ss) override{
        auto _super = boost::dynamic_pointer_cast<ssl_websocket_session>(shared_from_this());
        _super->send_message(ss);
    }

    void deliver(const std::string& message){
        state_->deliver(message, this);
    }

    bool is_ssl() override{
        return true;
    }

    steady_timer&
    dead_line(){
        return dead_line_;
    }

    void set_authorized(bool value) override{
        authorized_ = value;
        if(value)
            dead_line_.cancel();

    }
};

//------------------------------------------------------------------------------

template<class Body, class Allocator>
void
make_websocket_session(
        beast::tcp_stream stream,
        http::request<Body, http::basic_fields<Allocator>> req, boost::shared_ptr<shared_state> sharedPtr, bool is_http_authorization)
{
    std::make_shared<plain_websocket_session>(
            std::move(stream), std::move(sharedPtr), is_http_authorization)->run(std::move(req));
}

template<class Body, class Allocator>
void
make_websocket_session(
        beast::ssl_stream<beast::tcp_stream> stream,
        http::request<Body, http::basic_fields<Allocator>> req, boost::shared_ptr<shared_state> sharedPtr, bool is_http_authorization)
{
    std::make_shared<ssl_websocket_session>(
            std::move(stream), std::move(sharedPtr), is_http_authorization)->run(std::move(req));
}


#endif
