//
// Created by Борисоглебский on 21.09.2022.
//

#include "../include/shared_state.hpp"

shared_state::shared_state(const client::bClientData &param)
: m_data(param)
{

}

void shared_state::on_message(const std::string &message) {
    if(m_data.on_message)
        m_data.on_message(message);
}

void shared_state::on_connect() {
    if(m_data.on_connect)
        m_data.on_connect();
}

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
