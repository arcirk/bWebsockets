#include "../include/session.hpp"
#include "../include/shared_state.hpp"

shared_state::shared_state() {
    session_base_ = nullptr;
}

void shared_state::on_message(const std::string &message) {

    if(m_data.on_message)
        m_data.on_message(message);
}

void shared_state::on_error(const std::string &what, const std::string &err, int code) {
    if(m_data.on_error)
        m_data.on_error(what, err, code);
}

void shared_state::on_close() {
    if(m_data.on_close)
        m_data.on_close();
}

void shared_state::on_connect(session_base * base) {
    session_base_ = base;

    if(m_data.on_connect)
        m_data.on_connect();
}

void shared_state::on_status_changed(bool status) {
    if(m_data.on_status_changed)
        m_data.on_status_changed(status);
}

void shared_state::connect(const client::bClientEvent &event, const client::callbacks &f) {
    if(event == client::bClientEvent::wsClose){
        m_data.on_close= boost::get<callback_close>(f);
    }else if(event == client::bClientEvent::wsConnect){
        m_data.on_connect = boost::get<callback_connect>(f);
    }else if(event == client::bClientEvent::wsError){
        m_data.on_error = boost::get<callback_error>(f);
    }else if(event == client::bClientEvent::wsMessage){
        m_data.on_message = boost::get<callback_message>(f);
    }else if(event == client::bClientEvent::wsStatusChanged){
        m_data.on_status_changed = boost::get<callback_status>(f);
    }
}

void shared_state::close(bool disable_notify) {
    if(!session_base_)
        return;

    session_base_->close(disable_notify);
}

void shared_state::send(const boost::shared_ptr<const std::string> &ss) {
    if(!session_base_)
        return;
    if(session_base_->started()){
        session_base_->send_message(ss);
    }
}

void shared_state::set_basic_auth_string(const std::string &value) {
    basic_auth_string_ = value;
}

std::string shared_state::basic_auth_string() const {
    return basic_auth_string_;
}