#ifndef  BWEBSOCKETS_HTTP_HPP
#define BWEBSOCKETS_HTTP_HPP

#include <iostream>
#include "../arcirk/net.hpp"
#include "../arcirk/beast.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <boost/variant.hpp>
#include <boost/config.hpp>

class shared_state;
class plain_websocket_session;
class ssl_websocket_session;

#define UnknownUser         "Unknown user"
#define UnknownApplication  "Unknown application"

static beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
static std::string
path_cat(
        beast::string_view base,
        beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

template<class Send>
static void
handle_request_post(
        beast::string_view doc_root,
        Send&& send,
        const std::string& result,
        const std::string& target){

    std::string path = path_cat(doc_root, target);

//        boost::beast::string_view content_type = req[http::field::content_type];
//        if (content_type != "multipart/form-data" && content_type != "application/x-www-form-urlencoded")
//        {
//            return send(bad_request("Bad request"));
//        }
        unsigned int version = 0;
        http::response<http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(result)),
                std::make_tuple(http::status::ok, version) };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(result.size());
        res.keep_alive(false);
        return send(std::move(res));

}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
        class Body, class Allocator,
        class Send>
static void
handle_request(
        beast::string_view doc_root,
        http::request<Body, http::basic_fields<Allocator>>&& req,
        Send&& send, bool unauthorized = false)
{
    // Returns a bad request response
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

    // Returns a not found response
    auto const not_found =
            [&req](beast::string_view target)
            {
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
            [&req](beast::string_view what)
            {
                http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + std::string(what) + "'";
                res.prepare_payload();
                return res;
            };

    auto const server_authorization_error =
            [&req](beast::string_view what)
            {
                http::response<http::string_body> res{http::status::unauthorized, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + std::string(what) + "'";
                res.prepare_payload();
                return res;
            };

    if(unauthorized)
        return send(server_authorization_error("Incorrect username or password"));

    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::head &&
        req.method() != http::verb::post)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");
    else if (req.target().front() == '/' && req.target().size() > 1)
    {
//        using boost::beast::iequals;
//        bool r = false;
//        const auto ext = [&path, &r]
//        {
//            const auto pos = path.rfind(".");
//            if (pos == boost::beast::string_view::npos)
//            {
//                //return send(bad_request("Illegal request-target"));
//                r = true;
//            }
//            const auto pos2 = path.find(" ");
//            if (pos != boost::beast::string_view::npos)
//            {
//                //return send(bad_request("Illegal request-target"));
//                r = true;
//            }
//        }();
//        if (!iequals(ext, ".cgi") && !iequals(ext, ".exe") || r)
//        {
//            return send(bad_request("Illegal request-target"));
//        }

    }

    if(req.method() == http::verb::head || req.method() == http::verb::get) {
        // Attempt to open the file
        beast::error_code ec;
        http::file_body::value_type body;
        body.open(path.c_str(), beast::file_mode::scan, ec);

        // Handle the case where the file doesn't exist
        if(ec == beast::errc::no_such_file_or_directory)
            return send(not_found(req.target()));

        // Handle an unknown error
        if(ec)
            return send(server_error(ec.message()));

        // Cache the size since we need it after the move
        auto const size = body.size();

        // Respond to HEAD request
        if(req.method() == http::verb::head)
        {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(path));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }
        else if (req.method() == http::verb::get) {
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
    }

////    else if (req.method() == http::verb::post)
////    {
//        //post
//        //auto const body = state.handle_request(req.body(), "");
//
//        boost::beast::string_view content_type = req[http::field::content_type];
//        if (content_type != "multipart/form-data" && content_type != "application/x-www-form-urlencoded")
//        {
//            return send(bad_request("Bad request"));
//        }
//        http::response<http::file_body> res{
//                std::piecewise_construct,
//                std::make_tuple(std::move(body)),
//                std::make_tuple(http::status::ok, req.version()) };
//        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
//        res.set(http::field::content_type, mime_type(path));
//        res.content_length(body.size());
//        res.keep_alive(req.keep_alive());
//        return send(std::move(res));
// //   }

}

//------------------------------------------------------------------------------

// Report a failure
static void
fail(beast::error_code ec, char const* what, bool conv = true)
{
    if(ec == net::ssl::error::stream_truncated)
        return;

    std::tm tm = arcirk::current_date();
    char cur_date[100];
    std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

    int code = ec.value();
    std::string szCode = code != 0 ? " " + std::to_string(code) : "";
    std::string msg = code != 0 ? "::" + ec.message() : "";
    std::string what_ = conv ? arcirk::local_8bit(what) : what;

    std::cerr << std::string(cur_date) << " Error" << szCode << ": " << what_ << msg << std::endl;
}

static void info(const std::string& what, const std::string& message, bool conv = true){
    std::tm tm = arcirk::current_date();
    char cur_date[100];
    std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

    if(conv)
        std::cout << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(message) << std::endl;
    else
        std::cout << std::string(cur_date) << " " << what << ": " <<message << std::endl;
}

#endif