#include <utility>
#include "../include/shared_state.hpp"
#include "../include/listener.hpp"
#include "../include/websocket_session.hpp"

shared_state::shared_state() = default;

void shared_state::join(subscriber *session) {

    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(std::pair<boost::uuids::uuid, subscriber*>(session->uuid_session(), session));
    std::cout << "client join: " << arcirk::uuids::uuid_to_string(session->uuid_session()) << std::endl;

}

void shared_state::leave(const boost::uuids::uuid& session_uuid, const std::string& user_name) {
    auto iter = sessions_.find(session_uuid);
    if (iter != sessions_.end() ){
        sessions_.erase(session_uuid);
    }
    std::cout << "client leave: " << user_name << " (" << arcirk::uuids::uuid_to_string(session_uuid) << ")" << std::endl;
}

void shared_state::deliver(const std::string &message, subscriber *session) {

    std::string result;

    if (result == "\n")
        return;

    if (result == "ping")
        result = "pong";

    std::cout << "message: " << message << std::endl;

    send(message);

}


void shared_state::send(const std::string &message) {

//    std::vector<boost::weak_ptr<plain_websocket_session*>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_){
//            v.emplace_back(p.second->get<plain_websocket_session*>());
//        }
//        for(auto const& wp : v)
//            if(auto sp = wp.lock()){
//                auto const ss = boost::make_shared<std::string const>(message);
//                sp->send(ss);
//            }
//    }
//    std::vector<subscriber*> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_){
//            auto ww = p.second->get_ptr();
//            v.emplace_back(p.second);
//        }
//        for(auto const& wp : v)
//            if(auto sp = wp->lock()){
//                auto const ss = boost::make_shared<std::string const>(message);
//                sp->send(ss);
//            }
//    }
}

