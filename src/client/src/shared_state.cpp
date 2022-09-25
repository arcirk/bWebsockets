//
// Created by Борисоглебский on 21.09.2022.
//

#include "../include/shared_state.hpp"
#include "../include/websocket_client.hpp"
#include "../../client_test/shared_state.hpp"


shared_state::shared_state(const client::bClientData &param)
: m_data(param)
{
    session_ = nullptr;
}

void shared_state::on_message(const std::string &message) {

    if (message == "\n" || message.empty() || message == "pong")
        return;

    T_vec v = split(message, "\n");

    std::string msg;

    if (!v.empty())
        msg = v[0];

    if(m_data.on_message)
        m_data.on_message(message);
}

void shared_state::on_connect(session_base * sess) {
    session_ = sess;
    if(m_data.on_connect)
        m_data.on_connect();
}

//void shared_state::on_connect(plain_session* sess) {
//    session_ = sess;
//    if(m_data.on_connect)
//        m_data.on_connect();
//}

void shared_state::on_stop() {
    if(m_data.on_close)
        m_data.on_close();
}

void shared_state::on_error(const std::string &what, const std::string &err, int code) {
    if(m_data.on_error)
        m_data.on_error(what, err, code);
}

void shared_state::on_status_changed(bool status) {
    if(m_data.on_status_changed)
        m_data.on_status_changed(status);
}

void shared_state::close(bool block_notify) {

//    bool is_ssl = false;
//
//    if(session_.type() == typeid(ssl_session*))
//        is_ssl = true;
//
//    if(is_ssl){
//        auto sess = boost::get<ssl_session*>(session_);
//        if(sess)
//            sess->stop();
//    }else
//    {
//        auto sess = boost::get<plain_session*>(session_);
//        if(sess)
//            sess->stop();
//    }

    if(!session_)
        return;

    if(session_->is_ssl())
        session_->derived<ssl_session>().stop();
    else
        session_->derived<plain_session>().stop();

}

bool shared_state::started() {
    if(!session_)
        return false;

    return session_->started();

//    bool is_ssl = false;
//
//    if(session_.type() == typeid(ssl_session*))
//        is_ssl = true;
//
//    if(is_ssl){
//        auto sess = boost::get<ssl_session*>(session_);
//        if(sess)
//            return sess->is_open();
//    }else
//    {
//        auto sess = boost::get<plain_session*>(session_);
//        if(sess)
//            return sess->is_open();
//    }

}
