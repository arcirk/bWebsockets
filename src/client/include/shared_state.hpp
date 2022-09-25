//
// Created by Борисоглебский on 21.09.2022.
//

#ifndef ARCIRK_SOLUTION_SHARED_STATE_H
#define ARCIRK_SOLUTION_SHARED_STATE_H

#include <arcirk.hpp>
#include "callbacks.hpp"
#include <boost/variant.hpp>

//template<class Derived>
//class session;
//class ssl_session;
//class plain_session;

class session_base;

using namespace arcirk;

//typedef boost::variant<ssl_session*, plain_session*> current_session;

class shared_state {
    client::bClientData m_data;
public:
    explicit
    shared_state()= default;;
    explicit
    shared_state(const client::bClientData& param);

    void on_connect(session_base* sess);
    //void on_connect(plain_session* sess);
    void on_message(const std::string& message);
    void on_stop();
    void on_error(const std::string &what, const std::string &err, int code);
    void on_status_changed(bool status);

    void close(bool block_notify = false);

    bool started();

    void send(const std::string& message);

private:
    session_base * session_;

};


#endif //ARCIRK_SOLUTION_CHARED_STATE_H
