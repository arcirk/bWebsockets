//
// Created by arcady on 12.07.2021.
//

#ifndef WS_SOLUTION_WS_SESSION_H
#define WS_SOLUTION_WS_SESSION_H

#include "net.h"
#include "beast.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <boost/asio/steady_timer.hpp>

#include <deque>

using boost::asio::steady_timer;

class ws_client;

class session : public boost::enable_shared_from_this<session>
        {
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_ = "test";

    ws_client* client_{};

        public:
            // Resolver and socket require an io_context
            explicit
            session(net::io_context& ioc);

            ~session();

            // Start the asynchronous operation
            void
            run(char const* host, char const* port, ws_client * client);

            void
            on_resolve(
                    beast::error_code ec,
                    tcp::resolver::results_type results);
            void
            on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

            void
            on_handshake(beast::error_code ec);

            void start_read();

            void
            on_read(
                    beast::error_code ec,
                    std::size_t bytes_transferred);

            void start_write();

            void
            on_write(
                    beast::error_code ec,
                    std::size_t bytes_transferred);

            void
            stop(bool eraseObjOnly = false);

            void
            send(boost::shared_ptr<std::string const> const& ss);

            void
            on_close(beast::error_code ec);


            bool is_open() const;

        private:
            std::deque<std::string> output_queue_;
            steady_timer deadline_;
            steady_timer heartbeat_timer_;
            //std::string input_buffer_;
            bool started_ = false;

            bool get_started() const;

            void
            fail(beast::error_code ec, char const* what);

            void
            deliver(const std::string& msg);

            void
            check_deadline();

        };

#endif //WS_SOLUTION_WS_SESSION_H
