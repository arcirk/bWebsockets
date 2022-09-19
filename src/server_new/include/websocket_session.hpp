#ifndef  BWEBSOCKETS_WEBSOCKETS_SESSION_HPP
#define BWEBSOCKETS_WEBSOCKETS_SESSION_HPP

#include "http.hpp"
#include "shared_state.hpp"
#include <arcirk.hpp>

class subscriber{

public:

    virtual std::string user_name() const{
        return _user_name;
    }

    virtual boost::uuids::uuid user_uuid() const {
        return _user_uuid;
    }

    virtual boost::uuids::uuid uuid_session() const {
        return _uuid_session;
    }

    virtual void set_uuid_session(const boost::uuids::uuid& value){
        _uuid_session = value;
    }

protected:
    std::string _user_name = UnknownUser;
    boost::uuids::uuid _user_uuid{};
    boost::uuids::uuid _uuid_session{};
    //
};

// Echoes back all received WebSocket messages.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class websocket_session
{
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_;

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
        // Read a message
        do_read();
    }

    void
    do_read()
    {
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

        // This indicates that the websocket_session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            return fail(ec, "read");

//        // Echo the message
//        derived().ws().text(derived().ws().got_text());

        derived().deliver(beast::buffers_to_string(buffer_.data()));

        do_read();

    }

    void do_write(){

        derived().ws().async_write(
                buffer_.data(),
                beast::bind_front_handler(
                        &websocket_session::on_write,
                        derived().shared_from_this()));
    };

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

        // Do another read
        //do_read();
        do_write();

    }


public:
    // Start the asynchronous operation
    template<class Body, class Allocator>
    void
    run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Accept the WebSocket upgrade request
        do_accept(std::move(req));
    }

//    boost::uuids::uuid uuid() const{
//        return _uuid;
//    }


protected:
    //boost::uuids::uuid _uuid{};
    //boost::shared_ptr<shared_state> state_;
};

//------------------------------------------------------------------------------

// Handles a plain WebSocket connection
class plain_websocket_session
        : public websocket_session<plain_websocket_session>
                , public std::enable_shared_from_this<plain_websocket_session>, public subscriber
{
    websocket::stream<beast::tcp_stream> ws_;
    boost::shared_ptr<shared_state> state_;
public:
    // Create the session
    explicit
    plain_websocket_session(
            beast::tcp_stream&& stream, boost::shared_ptr<shared_state> sharedPtr)
            : ws_(std::move(stream))
            , state_(std::move(sharedPtr))
    {
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

    boost::shared_ptr<shared_state>&
    state(){
        return state_;
    }

    void join(){
        state_->join(this);
    }

    void deliver(const std::string& message){
        state_->deliver(message, this);
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

public:
    // Create the ssl_websocket_session
    explicit
    ssl_websocket_session(
            beast::ssl_stream<beast::tcp_stream>&& stream, boost::shared_ptr<shared_state> sharedPtr)
            : ws_(std::move(stream))
            , state_(std::move(sharedPtr))
    {
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

    boost::shared_ptr<shared_state>&
    state(){
        return state_;
    }

    void join(){
        state_->join(this);
    }

    void deliver(const std::string& message){
        state_->deliver(message, this);
    }
};

//------------------------------------------------------------------------------

template<class Body, class Allocator>
void
make_websocket_session(
        beast::tcp_stream stream,
        http::request<Body, http::basic_fields<Allocator>> req, boost::shared_ptr<shared_state> sharedPtr)
{
    std::make_shared<plain_websocket_session>(
            std::move(stream), std::move(sharedPtr))->run(std::move(req));
}

template<class Body, class Allocator>
void
make_websocket_session(
        beast::ssl_stream<beast::tcp_stream> stream,
        http::request<Body, http::basic_fields<Allocator>> req, boost::shared_ptr<shared_state> sharedPtr)
{
    std::make_shared<ssl_websocket_session>(
            std::move(stream), std::move(sharedPtr))->run(std::move(req));
}


#endif
