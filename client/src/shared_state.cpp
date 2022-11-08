#include "../include/session.hpp"
#include "../include/shared_state.hpp"

shared_state::shared_state() {
    session_base_ = nullptr;
    session_uuid_ = arcirk::uuids::nil_uuid();
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
    start_date_ = arcirk::current_date();
    if(m_data.on_connect)
        m_data.on_connect();
}

void shared_state::on_status_changed(bool status) {
    if(m_data.on_status_changed)
        m_data.on_status_changed(status);
}

void shared_state::connect(const client::client_events &event, const client::callbacks &f) {
    if(event == client::client_events::wsClose){
        m_data.on_close= boost::get<callback_close>(f);
    }else if(event == client::client_events::wsConnect){
        m_data.on_connect = boost::get<callback_connect>(f);
    }else if(event == client::client_events::wsError){
        m_data.on_error = boost::get<callback_error>(f);
    }else if(event == client::client_events::wsMessage){
        m_data.on_message = boost::get<callback_message>(f);
    }else if(event == client::client_events::wsStatusChanged){
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

void shared_state::command_to_server(const std::string &command, const std::string &param) {

    if(command.empty())
        return;

    std::string cmd = "cmd " + command;

    if(!param.empty()){
        using json_nl = nlohmann::json;
        std::string private_param = arcirk::base64::base64_encode(param);
        json_nl param_ = {
                {"parameters", private_param}
        };
        cmd.append(" ");
        cmd.append(arcirk::base64::base64_encode(param_.dump()));
    }

    auto const ss = boost::make_shared<std::string const>(std::move(cmd));
    send(ss);
}

boost::uuids::uuid shared_state::session_uuid() const {
    return session_uuid_;
}

void shared_state::command_to_client(const std::string &receiver, const std::string &command,
                                     const std::string &param) {
    if(command.empty() || receiver.empty())
        return;

    std::string cmd = "cmd " + enum_synonym(arcirk::server::server_commands::CommandToClient) + receiver;

    if(!param.empty()){
        using json_nl = nlohmann::json;
        std::string private_param = arcirk::base64::base64_encode(param);
        json_nl param_ = {
                {"parameters", private_param},
                {"recipient", receiver},
                {"command", command}
        };
        cmd.append(" ");
        cmd.append(arcirk::base64::base64_encode(param_.dump()));
    }

    auto const ss = boost::make_shared<std::string const>(std::move(cmd));
    send(ss);
}