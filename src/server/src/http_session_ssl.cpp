#include "../include/http_session_ssl.hpp"
#include "../include/websocket_session_ssl.hpp"

namespace mime_ssl {

    beast::string_view
    mime_type(beast::string_view path) {
        using beast::iequals;
        auto const ext = [&path] {
            auto const pos = path.rfind(".");
            if (pos == beast::string_view::npos)
                return beast::string_view{};
            return path.substr(pos);
        }();
        if (iequals(ext, ".htm")) return "text/html";
        if (iequals(ext, ".html")) return "text/html";
        if (iequals(ext, ".php")) return "text/html";
        if (iequals(ext, ".css")) return "text/css";
        if (iequals(ext, ".txt")) return "text/plain";
        if (iequals(ext, ".js")) return "application/javascript";
        if (iequals(ext, ".json")) return "application/json";
        if (iequals(ext, ".xml")) return "application/xml";
        if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
        if (iequals(ext, ".flv")) return "video/x-flv";
        if (iequals(ext, ".png")) return "image/png";
        if (iequals(ext, ".jpe")) return "image/jpeg";
        if (iequals(ext, ".jpeg")) return "image/jpeg";
        if (iequals(ext, ".jpg")) return "image/jpeg";
        if (iequals(ext, ".gif")) return "image/gif";
        if (iequals(ext, ".bmp")) return "image/bmp";
        if (iequals(ext, ".ico")) return "image/vnd.microsoft.icon";
        if (iequals(ext, ".tiff")) return "image/tiff";
        if (iequals(ext, ".tif")) return "image/tiff";
        if (iequals(ext, ".svg")) return "image/svg+xml";
        if (iequals(ext, ".svgz")) return "image/svg+xml";
        return "application/text";
    }

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
    std::string
    path_cat(
            beast::string_view base,
            beast::string_view path) {
        if (base.empty())
            return std::string(path);
        std::string result(base);
#ifdef BOOST_MSVC
        char constexpr path_separator = '\\';
        if (result.back() == path_separator)
            result.resize(result.size() - 1);
        result.append(path.data(), path.size());
        for (auto &c: result)
            if (c == '/')
                c = path_separator;
#else
        char constexpr path_separator = '/';
        if(result.back() == path_separator)
            result.resize(result.size() - 1);
        result.append(path.data(), path.size());
#endif
        return result;
    }

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
    template<
            class Body, class Allocator,
            class Send>
    void
    handle_request(
            beast::string_view doc_root,
            http::request<Body, http::basic_fields<Allocator>> &&req,
            Send &&send) {
        // Returns a bad request response
        auto const bad_request =
                [&req](beast::string_view why) {
                    http::response<http::string_body> res{http::status::bad_request, req.version()};
                    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = std::string(why);
                    res.prepare_payload();
                    return res;
                };

        // Returns a not found response
        auto const not_found =
                [&req](beast::string_view target) {
                    http::response<http::string_body> res{http::status::not_found, req.version()};
                    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "The resource '" + std::string(target) + "' was not found.";
                    res.prepare_payload();
                    return res;
                };

        // Returns a server error response
        auto const server_error =
                [&req](beast::string_view what) {
                    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "An error occurred: '" + std::string(what) + "'";
                    res.prepare_payload();
                    return res;
                };

        // Make sure we can handle the method
        if (req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return send(bad_request("Unknown HTTP-method"));

        // Request path must be absolute and not contain "..".
        if (req.target().empty() ||
            req.target()[0] != '/' ||
            req.target().find("..") != beast::string_view::npos)
            return send(bad_request("Illegal request-target"));

        // Build the path to the requested file
        std::string path = path_cat(doc_root, req.target());
        if (req.target().back() == '/')
            path.append("index.html");

        // Attempt to open the file
        beast::error_code ec;
        http::file_body::value_type body;
        body.open(path.c_str(), beast::file_mode::scan, ec);

        // Handle the case where the file doesn't exist
        if (ec == beast::errc::no_such_file_or_directory)
            return send(not_found(req.target()));

        // Handle an unknown error
        if (ec)
            return send(server_error(ec.message()));

        // Cache the size since we need it after the move
        auto const size = body.size();

        // Respond to HEAD request
        if (req.method() == http::verb::head) {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(path));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }

        // Respond to GET request
        http::response<http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

//------------------------------------------------------------------------------

// Report a failure
    void
    fail(beast::error_code ec, char const *what) {
        // ssl::error::stream_truncated, also known as an SSL "short read",
        // indicates the peer closed the connection without performing the
        // required closing handshake (for example, Google does this to
        // improve performance). Generally this can be a security issue,
        // but if your communication protocol is self-terminated (as
        // it is with both HTTP and WebSocket) then you may simply
        // ignore the lack of close_notify.
        //
        // https://github.com/boostorg/beast/issues/38
        //
        // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
        //
        // When a short read would cut off the end of an HTTP message,
        // Beast returns the error beast::http::error::partial_message.
        // Therefore, if we see a short read here, it has occurred
        // after the message has been completed, so it is safe to ignore it.

        if (ec == net::ssl::error::stream_truncated)
            return;

        std::cerr << what << ": " << ec.message() << "\n";
    }
}

http_session_ssl::http_session_ssl(
        tcp::socket&& socket,
        ssl::context& ctx,
        boost::shared_ptr<shared_state> const& state)
: stream_(std::move(socket), ctx)
, lambda_(*this)
, state_(state)
, ctx_(ctx)
{
}
// Start the asynchronous operation
void
http_session_ssl::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(
            stream_.get_executor(),
            beast::bind_front_handler(
                    &http_session_ssl::on_run,
                    shared_from_this()));
}

void
http_session_ssl::on_run()
{
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(
            std::chrono::seconds(30));

    // Perform the SSL handshake
    stream_.async_handshake(
            ssl::stream_base::server,
            beast::bind_front_handler(
                    &http_session_ssl::on_handshake,
                    shared_from_this()));
}

void
http_session_ssl::on_handshake(beast::error_code ec)
{
    if(ec)
        return mime_ssl::fail(ec, "handshake");

    do_read();
}

void
http_session_ssl::do_read()
{
//     Make the request empty before reading,
//     otherwise the operation behavior is undefined.
    //req_ = {};
    parser_.emplace();
    parser_->body_limit(10000);

    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(stream_, buffer_, parser_->get(),
                     beast::bind_front_handler(
                             &http_session_ssl::on_read,
                             shared_from_this()));
}

void
http_session_ssl::on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
        return do_close();

    if(ec)
        return mime_ssl::fail(ec, "read");

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(parser_->get()))
    {
        // Create a websocket session, transferring ownership
        // of both the socket and the HTTP request.
        boost::make_shared<websocket_session_ssl>(
                beast::get_lowest_layer(stream_).release_socket(), ctx_,
                state_)->run(parser_->release());
        return;
    }

    // Send the response
    mime_ssl::handle_request(state_->doc_root(), parser_->release(), lambda_);
}

void
http_session_ssl::on_write(
        bool close,
        beast::error_code ec,
        std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return mime_ssl::fail(ec, "write");

    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // We're done with the response so delete it
    res_ = nullptr;

    // Read another request
    do_read();
}

void
http_session_ssl::do_close()
{
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    stream_.async_shutdown(
            beast::bind_front_handler(
                    &http_session_ssl::on_shutdown,
                    shared_from_this()));
}

void
http_session_ssl::on_shutdown(beast::error_code ec)
{
    if(ec)
        return mime_ssl::fail(ec, "shutdown");

    // At this point the connection is closed gracefully
}

