//
// Created by admin on 30.01.2023.
//
#include <iostream>
#include <net.hpp>
#include <beast.hpp>
#include <arcirk.hpp>
#include <functional>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>

int
main(int argc, char* argv[]) {

    setlocale(LC_ALL, "Russian");

    try
    {
        // Check command line arguments.
        if(argc != 4 && argc != 5)
        {
            std::cerr <<
                      "Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                      "Example:\n" <<
                      "    http-client-sync www.example.com 80 /\n" <<
                      "    http-client-sync www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        auto const target = argv[3];
        int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

//        // Set up an HTTP GET request message
//        http::request<http::string_body> req{http::verb::get, target, version};
//        req.set(http::field::host, host);
//        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        http::request<http::string_body> req{http::verb::post, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        std::string auth = "IIS_1C:LbyFvj1";
        req.set(http::field::authorization, "Basic " + arcirk::base64::base64_encode(auth));
        req.set(http::field::content_type, "application/json");

        nlohmann::json body = {
                {"command", "Version"}
        };

        req.body() = body.dump();
        req.prepare_payload();
        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}