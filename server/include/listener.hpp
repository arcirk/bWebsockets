#ifndef  BWEBSOCKETS_LISTENER_HPP
#define BWEBSOCKETS_LISTENER_HPP

#include "http_session.hpp"

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;
    boost::shared_ptr<shared_state> state_;

public:
    listener(
            net::io_context& ioc,
            ssl::context& ctx,
            tcp::endpoint endpoint,
            std::shared_ptr<std::string const> const& doc_root,
            boost::shared_ptr<shared_state> const& state)
            : ioc_(ioc)
            , ctx_(ctx)
            , acceptor_(net::make_strand(ioc))
            , doc_root_(doc_root)
            , state_(state)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "listener::open");
            //return;
            std::exit(0);
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "listener::set_option");
            //return;
            std::exit(0);
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "listener::bind");
            std::exit(0);
            //return;
        }

        //std::cout << "start server listen " << endpoint.address().to_string() << " " << endpoint.port() << " ..." << std::endl;
        info(__FUNCTION__,  "start server listen " + endpoint.address().to_string() + " " + std::to_string(endpoint.port()) + " ...");

        // Start listening for connections
        acceptor_.listen(
                net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listener::listen");
            //return;
            std::exit(0);
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        //std::tm tm = arcirk::current_date();
        //std::cout.imbue(std::locale("ru_RU.utf8"));
        //std::cout << "server run " << std::put_time(&tm, "%c %Z") << std::endl;
        info(__FUNCTION__, "server run");
        state_->start();
        do_accept();
    }

private:
    void
    do_accept()
    {

        // The new connection gets its own strand
        acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                        &listener::on_accept,
                        shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the detector http_session and run it
            std::make_shared<detect_session>(
                    std::move(socket),
                    ctx_,
                    doc_root_,
                    state_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

#endif