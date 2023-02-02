//
// Created by admin on 30.01.2023.
//
#include <iostream>
#include <arcirk.hpp>
#include <http_session.hpp>
#include <functional>
#include <boost/beast/ssl.hpp>

int
main(int argc, char* argv[]) {

    setlocale(LC_ALL, "Russian");

    // Check command line arguments.
    if(argc != 4 && argc != 5)
    {
        std::cerr <<
                  "Usage: http-client-async <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                  "Example:\n" <<
                  "    http-client-async www.example.com 80 /\n" <<
                  "    http-client-async www.example.com 80 / 1.0\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // Launch the asynchronous operation
    std::make_shared<http_session>(ioc)->run(host, port, target, version);

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();

    return EXIT_SUCCESS;
}