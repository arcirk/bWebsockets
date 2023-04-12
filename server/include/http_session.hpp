#ifndef  BWEBSOCKETS_HTTP_SESSION_HPP
#define BWEBSOCKETS_HTTP_SESSION_HPP

#include "websocket_session.hpp"

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class http_session
{
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    // This queue is used for HTTP pipelining.
    class queue
    {
        enum
        {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct work
        {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::vector<std::unique_ptr<work>> items_;

    public:
        explicit
        queue(http_session& self)
                : self_(self)
        {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        // Returns `true` if we have reached the queue limit
        bool
        is_full() const
        {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool
        on_write()
        {
            BOOST_ASSERT(! items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if(! items_.empty())
                (*items_.front())();
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                http_session& self_;
                http::message<isRequest, Body, Fields> msg_;

                work_impl(
                        http_session& self,
                        http::message<isRequest, Body, Fields>&& msg)
                        : self_(self)
                        , msg_(std::move(msg))
                {
                }

                void
                operator()()
                {
                    http::async_write(
                            self_.derived().stream(),
                            msg_,
                            beast::bind_front_handler(
                                    &http_session::on_write,
                                    self_.derived().shared_from_this(),
                                    msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(
                    boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if(items_.size() == 1)
                (*items_.front())();
        }
    };

    std::shared_ptr<std::string const> doc_root_;
    queue queue_;

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<http::request_parser<http::string_body>> parser_;

protected:
    beast::flat_buffer buffer_;
    boost::shared_ptr<shared_state> state_;

public:
    // Construct the session
    http_session(
            beast::flat_buffer buffer,
            std::shared_ptr<std::string const> const& doc_root,
            boost::shared_ptr<shared_state> const& state)
            : doc_root_(doc_root)
            , queue_(*this)
            , buffer_(std::move(buffer))
            , state_(state)
    {
    }

    void
    do_read()
    {
        // Construct a new parser for each message
        parser_.emplace();

        // Apply a reasonable limit to the allowed size
        // of the body in bytes to prevent abuse.
        parser_->body_limit(10000);

        // Set the timeout.
        beast::get_lowest_layer(
                derived().stream()).expires_after(std::chrono::seconds(30));

        // Read a request using the parser-oriented interface
        http::async_read(
                derived().stream(),
                buffer_,
                *parser_,
                beast::bind_front_handler(
                        &http_session::on_read,
                        derived().shared_from_this()));
    }

    void
    on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return derived().do_eof();

        if(ec)
            return fail(ec, "read");

        auto req = parser_->get();
        bool http_authorization = false;

        if(state_->use_authorization()){
            std::string auth = static_cast<std::string>(req[http::field::authorization]);
            http_authorization = state_->verify_connection(auth);
            if(http_authorization)
                info("on_read", "Authorization passed successfully");
        }

        // See if it is a WebSocket Upgrade
        if(websocket::is_upgrade(req))
        {
            // Disable the timeout.
            // The websocket::stream uses its own timeout settings.
            beast::get_lowest_layer(derived().stream()).expires_never();

            // Create a websocket session, transferring ownership
            // of both the socket and the HTTP request.
            return make_websocket_session(
                    derived().release_stream(),
                    parser_->release(), state_, http_authorization);
        }else{
            if(state_->use_authorization() && !http_authorization){
                std::string shared_files_dir = "/shared_files/releases";
                std::string target = static_cast<std::string>(req.target());
                //auto const post_body = boost::make_shared<std::string const>(req.body());
                //return handle_request(*doc_root_, parser_->release(), queue_, true);
                if(target.empty() || target == "/")
                    return handle_request(*doc_root_, parser_->release(), queue_);
                else if(target.substr(0, shared_files_dir.length()) == "/shared_files/releases"){
                    return handle_request(*doc_root_, parser_->release(), queue_);
                }
            }
        }

        auto req_ = parser_->release();

        if(!http_authorization){
            std::string target = static_cast<std::string>(req.target());
            if (target.empty() || target == "/") // откроем index.html
                return handle_request(*doc_root_, parser_->release(), queue_);
            else{
                http::response<http::string_body> res{http::status::unauthorized, req_.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req_.keep_alive());
                res.body() = "An error occurred: Incorrect username or password.";
                res.prepare_payload();
                return queue_(std::move(res));
            }
        }else{
            auto const bad_request =
                    [&req](beast::string_view why)
                    {
                        http::response<http::string_body> res{http::status::bad_request, req.version()};
                        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                        res.set(http::field::content_type, "text/html");
                        res.keep_alive(req.keep_alive());
                        res.body() = std::string(why);
                        res.prepare_payload();
                        return res;
                    };
            if(req_.method() == http::verb::post){
                const auto body = req_.body();
                std::string result;
                try{
                    result = state_->handle_request(body, static_cast<std::string>(req_[http::field::authorization]));
                }catch (std::exception &e) {
                    return queue_(bad_request(e.what()));
                }
                if(result.c_str() == "error")
                    return queue_(bad_request("Ошибка в параметрах запроса"));
                http::response<http::string_body> res{http::status::ok, req_.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "application/json");
                res.keep_alive(req_.keep_alive());
                res.body() = result;
                res.prepare_payload();
                queue_(std::move(res));
            }else{
                handle_request(*doc_root_, parser_->release(), queue_);
            }
        }
        // If we aren't at the queue limit, try to pipeline another request
        if(! queue_.is_full())
            do_read();
    }

    void
    on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return derived().do_eof();
        }

        // Inform the queue that a write completed
        if(queue_.on_write())
        {
            // Read another request
            do_read();
        }
    }
};

//------------------------------------------------------------------------------

// Handles a plain HTTP connection
class plain_http_session
        : public http_session<plain_http_session>
                , public std::enable_shared_from_this<plain_http_session>
{
    beast::tcp_stream stream_;

public:
    // Create the session
    plain_http_session(
            beast::tcp_stream&& stream,
            beast::flat_buffer&& buffer,
            std::shared_ptr<std::string const> const& doc_root,
            boost::shared_ptr<shared_state> const& state)
            : http_session<plain_http_session>(
            std::move(buffer),
            doc_root, state)
            , stream_(std::move(stream))
    {
    }

    // Start the session
    void
    run()
    {
        this->do_read();
    }

    // Called by the base class
    beast::tcp_stream&
    stream()
    {
        return stream_;
    }

    // Called by the base class
    beast::tcp_stream
    release_stream()
    {
        return std::move(stream_);
    }

    // Called by the base class
    void
    do_eof()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Handles an SSL HTTP connection
class ssl_http_session
        : public http_session<ssl_http_session>
                , public std::enable_shared_from_this<ssl_http_session>
{
    beast::ssl_stream<beast::tcp_stream> stream_;


public:
    // Create the http_session
    ssl_http_session(
            beast::tcp_stream&& stream,
            ssl::context& ctx,
            beast::flat_buffer&& buffer,
            std::shared_ptr<std::string const> const& doc_root,
            boost::shared_ptr<shared_state> const& state)
            : http_session<ssl_http_session>(
            std::move(buffer),
            doc_root, state)
            , stream_(std::move(stream), ctx)
    {
    }

    // Start the session
    void
    run()
    {
        // Set the timeout.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Perform the SSL handshake
        // Note, this is the buffered version of the handshake.
        stream_.async_handshake(
                ssl::stream_base::server,
                buffer_.data(),
                beast::bind_front_handler(
                        &ssl_http_session::on_handshake,
                        shared_from_this()));
    }

    // Called by the base class
    beast::ssl_stream<beast::tcp_stream>&
    stream()
    {
        return stream_;
    }

    // Called by the base class
    beast::ssl_stream<beast::tcp_stream>
    release_stream()
    {
        return std::move(stream_);
    }

    // Called by the base class
    void
    do_eof()
    {
        // Set the timeout.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Perform the SSL shutdown
        stream_.async_shutdown(
                beast::bind_front_handler(
                        &ssl_http_session::on_shutdown,
                        shared_from_this()));
    }

private:
    void
    on_handshake(
            beast::error_code ec,
            std::size_t bytes_used)
    {
        if(ec)
            return fail(ec, "handshake");

        // Consume the portion of the buffer used by the handshake
        buffer_.consume(bytes_used);

        do_read();
    }

    void
    on_shutdown(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Detects SSL handshakes
class detect_session : public std::enable_shared_from_this<detect_session>
{
    beast::tcp_stream stream_;
    ssl::context& ctx_;
    std::shared_ptr<std::string const> doc_root_;
    beast::flat_buffer buffer_;
    boost::shared_ptr<shared_state> state_;

public:
    explicit
    detect_session(
            tcp::socket&& socket,
            ssl::context& ctx,
            std::shared_ptr<std::string const> const& doc_root,
            boost::shared_ptr<shared_state> const& state)
            : stream_(std::move(socket))
            , ctx_(ctx)
            , doc_root_(doc_root)
            , state_(state)
    {
    }

    // Launch the detector
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
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
            std::make_shared<ssl_http_session>(
                    std::move(stream_),
                    ctx_,
                    std::move(buffer_),
                    doc_root_,
                    state_)->run();
            return;
        }

        // Launch plain session
        std::make_shared<plain_http_session>(
                std::move(stream_),
                std::move(buffer_),
                doc_root_,
                state_)->run();
    }
};

#endif