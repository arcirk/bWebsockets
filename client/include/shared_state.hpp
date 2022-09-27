#ifndef WEBSOCKET_CLIENT_SHARED_STATE_HPP
#define WEBSOCKET_CLIENT_SHARED_STATE_HPP

#include <arcirk.hpp>
#include "callbacks.hpp"

class session_base;

using namespace arcirk;

class shared_state{

public:
    explicit
    shared_state();
    ~shared_state()= default;

public:
    void on_message(const std::string& message);
    void on_error(const std::string &what, const std::string &err, int code);
    void on_close();
    void on_connect(session_base * base);
    void on_status_changed(bool status);

    void connect(const client::bClientEvent& event, const client::callbacks& f);
    void close(bool disable_notify);

    void send(boost::shared_ptr<std::string const> const& ss);

private:
    session_base * session_base_;
    client::bClientData m_data;

};

#endif