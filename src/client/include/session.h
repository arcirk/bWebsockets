//
// Created by arcady on 12.07.2021.
//

#ifndef WS_SOLUTION_WS_SESSION_H
#define WS_SOLUTION_WS_SESSION_H

#include <net.hpp>
#include <beast.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <boost/asio/steady_timer.hpp>

#include <deque>
#include "session_base.hpp"

using boost::asio::steady_timer;

class ws_client;

class session : public session_base, public boost::enable_shared_from_this<session>
        {
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

        public:
            // Resolver and socket require an io_context
            explicit
            session(net::io_context& ioc, const std::string & auth);

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
            stop() override;

            void
            send(boost::shared_ptr<std::string const> const& ss) override;

            void
            on_close(beast::error_code ec);


            bool is_open() const override;

        private:
            std::deque<std::string> output_queue_;
            steady_timer dead_line_;
            steady_timer heartbeat_timer_;

            bool get_started() const;

            void
            fail(beast::error_code ec, char const* what);

            void
            deliver(const std::string& msg);

            void
            check_dead_line();

        };

#endif //WS_SOLUTION_WS_SESSION_H
