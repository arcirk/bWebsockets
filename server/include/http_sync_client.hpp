#ifndef BWEBSOCKETS_HTTP_SYNC_CLIENT_HPP
#define BWEBSOCKETS_HTTP_SYNC_CLIENT_HPP

#include <iostream>
#include <net.hpp>
#include <beast.hpp>
#include <arcirk.hpp>
#include <functional>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>

class http_sync_client{

public:
    explicit http_sync_client(const std::string& url, const http::verb& method){

        uri_ = arcirk::Uri::Parse(url);
        host_ = uri_.Host;
        port_ = 0;
        if(!uri_.Port.empty())
            port_ = std::stoi(uri_.Port);
        if(port_ == 0){
            if(uri_.Protocol == "http")
                port_ = 80;
            else if(uri_.Protocol == "https")
                port_ = 443;
        }
        if(!uri_.BasicAuth.empty())
            auth_ = uri_.BasicAuth;

        target_ = uri_.QueryString;
        content_type_ = "application/json";

        method_ = method;
        version = 10;
    }

    void set_basic_auth(const std::string& usr, const std::string& pwd){
        auth_ = "Basic " + arcirk::base64::base64_encode(usr + ":" + pwd);
    }

    void set_body(const nlohmann::json& body){
        body_ = body;
    }

    http::response<http::dynamic_body> response(net::io_context& ioc){

        //net::io_context ioc;
        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host_, std::to_string(port_));

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        http::request<http::string_body> req{method_, target_, version};
        req.set(http::field::host, host_);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::authorization, "Basic " + arcirk::base64::base64_encode(auth_));
        req.set(http::field::content_type, content_type_);
        req.body() = body_.dump();
        req.prepare_payload();

        http::write(stream, req);

        beast::flat_buffer buffer;

        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        return res;
    }

private:
    std::string auth_;
    http::verb method_;
    arcirk::Uri uri_;
    std::string host_;
    int port_;
    std::string target_;
    nlohmann::json body_{};
    std::string content_type_;
    int version;
};

#endif