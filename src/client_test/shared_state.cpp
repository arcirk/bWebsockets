#include "session.hpp"
#include "shared_state.hpp"


shared_state::shared_state() {
    session_base_ = nullptr;
}

void shared_state::on_message(const std::string &message) {

}

void shared_state::on_error(const std::string &what, const std::string &err, int code) {

}

void shared_state::on_close() {

}

void shared_state::on_connect(session_base * base) {
    session_base_ = base;
}
