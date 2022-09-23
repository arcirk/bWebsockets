#include <utility>
#include "../include/shared_state.hpp"
//#include "../include/listener.hpp"
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

    std::vector<boost::weak_ptr<subscriber>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_){
            //auto sess = (plain_websocket_session*)(p.second);
            v.emplace_back(p.second);
        }
//        for(auto const& wp : v)
//            if(auto sp = wp.lock()){
//                auto const ss = boost::make_shared<std::string const>(message);
//                sp->send(ss);
//            }
    }


//    std::vector<boost::weak_ptr<plain_websocket_session>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(v_sessions_.size());
//        for(auto p : v_sessions_){
//            plain_websocket_session* sess = boost::get<plain_websocket_session*>(p.second);
//            v.emplace_back(sess->weak_from_this());
//        }
//        for(auto const& wp : v)
//            if(auto sp = wp.lock()){
//                auto const ss = boost::make_shared<std::string const>(message);
//                sp->send(ss);
//            }
//    }

//    std::vector<boost::weak_ptr<websocket_session<plain_websocket_session>>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(plain_sessions_.size());
//        for(auto p : plain_sessions_){
//            //plain_websocket_session* sess = boost::get<plain_websocket_session*>(p.second);
//            v.emplace_back(p.second->weak_from_this());
//        }
//        for(auto const& wp : v)
//            if(auto sp = wp.lock()){
//                auto const ss = boost::make_shared<std::string const>(message);
//                sp->send(ss);
//            }
//    }
//    std::vector<boost::weak_ptr<ssl_websocket_session>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(sessions_.size());
//        for(auto p : sessions_){
//            //plain_websocket_session* sess = boost::get<plain_websocket_session*>(p.second);
//            if(p.second->is_ssl()){
//                auto pw = (ssl_websocket_session*)p.second;
//                v.emplace_back(pw->shared_from_this());
//            }
//
//        }
//        for(auto const& wp : v)
//            if(auto sp = wp.lock()){
//                auto const ss = boost::make_shared<std::string const>(message);
//                sp->send(ss);
//            }
//    }
}

void shared_state::join_adv(plain_websocket_session *session) {
    //v_sessions_.insert(std::pair<boost::uuids::uuid const, vSessions>(session->uuid_session(), session));
}

void shared_state::join_adv(ssl_websocket_session *session) {
    //v_sessions_.insert(std::pair<boost::uuids::uuid const, vSessions>(session->uuid_session(), session));
}
