#include "websocket_session.hpp"

//void shared_state::send() {
//
//    std::vector<boost::weak_ptr<plain_websocket_session>> v;
//    {
//        std::lock_guard<std::mutex> lock(mutex_);
//        v.reserve(plain_sessions_.size());
//        for(auto p : plain_sessions_)
//            v.emplace_back(p.second->derived().weak_from_this());
//    }
//
//}