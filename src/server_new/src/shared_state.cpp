#include "../include/shared_state.hpp"
#include "../include/listener.hpp"

shared_state::shared_state() = default;

void shared_state::join(subscriber *session) {

    std::cout << "client join: " << arcirk::uuids::uuid_to_string(session->uuid_session()) << std::endl;

}

void shared_state::leave(const boost::uuids::uuid& session_uuid, const std::string& user_name) {
    std::cout << "client leave: " << user_name << " (" << arcirk::uuids::uuid_to_string(session_uuid) << ")" << std::endl;
}

void shared_state::deliver(const std::string &message, subscriber *session) {
    std::cout << "message: " << message << std::endl;
}

