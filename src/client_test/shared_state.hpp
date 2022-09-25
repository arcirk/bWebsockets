#ifndef WEBSOCKET_CLIENT_SHARED_STATE_HPP
#define WEBSOCKET_CLIENT_SHARED_STATE_HPP

#include <arcirk.hpp>

class session_base;

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

private:
    session_base * session_base_;
};

#endif