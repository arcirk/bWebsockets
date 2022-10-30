#ifndef WEBSOCKET_CLIENT_SHARED_STATE_HPP
#define WEBSOCKET_CLIENT_SHARED_STATE_HPP

#include <arcirk.hpp>
#include <shared_struct.hpp>
#include <database_struct.hpp>

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

    void connect(const client::client_events& event, const client::callbacks& f);
    void close(bool disable_notify);

    void send(boost::shared_ptr<std::string const> const& ss);

    void set_basic_auth_string(const std::string& value);

    [[nodiscard]] std::string basic_auth_string() const;

    void command_to_server(const std::string& command, const std::string& param = "");

    [[nodiscard]] boost::uuids::uuid session_uuid() const;

    void set_uuid_session(const boost::uuids::uuid& value){
        session_uuid_ = value;
    }

private:
    session_base * session_base_;
    client::client_data m_data;
    std::string basic_auth_string_;
    boost::uuids::uuid session_uuid_{};
    std::tm start_date_{};
};

#endif