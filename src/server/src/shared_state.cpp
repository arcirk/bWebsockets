#include "../include/shared_state.hpp"

#include <utility>
#include "../include/websocket_session.hpp"
#include "../include/websocket_session_ssl.hpp"

#ifdef _WINDOWS
#pragma warning(disable:4100)
#endif

shared_state::
shared_state(std::string doc_root, bool is_ssl)
        : doc_root_(std::move(doc_root))
{
    enable_random_connections = true;
    enable_ssl = is_ssl;
}

void shared_state::join(websocket_session_ssl *session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_ssl_.insert(std::pair<boost::uuids::uuid, websocket_session_ssl*>(session->get_uuid(), session));
}
void shared_state::join(websocket_session *session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(std::pair<boost::uuids::uuid, websocket_session*>(session->get_uuid(), session));
}

void
shared_state::
leave(websocket_session_ssl* session)
{
#ifdef _WINDOWS
    std::string msg = boost::locale::conv::from_utf(session->get_name(), "windows-1251");
#else
    std::string msg = session->get_name();
#endif
    std::cout << "client leave: " << msg << std::endl;

    if(!enable_random_connections){
        if (session->is_authorized() && !session->disable_notify()){
//            // Оповещаем всех об отключении клиента
//            send(std::string (R"({"name": ")") + session->get_name() +
//                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
//                 arcirk::uuids::uuid_to_string(session->get_uuid()) + "\"}", "client_leave", true, session->notify_apps());
        }
    }else{
        if (!session->disable_notify()){
            send(std::string (R"({"name": ")") + session->get_name() +
                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
                 arcirk::uuids::uuid_to_string(session->get_uuid()) + R"(", "command": "client_leave"")" + "\"}");
        }
    }


    std::lock_guard<std::mutex> lock(mutex_);

    auto iter_u = user_sessions_ssl.find(session->get_user_uuid());
    if (iter_u != user_sessions_ssl.end() ){
        if (iter_u->second.size() <= 1){
            user_sessions_ssl.erase(session->get_user_uuid());
            std::cout << "erase user sessions" << std::endl;
        }
        else{
            iter_u->second.erase(find(iter_u->second.begin(), iter_u->second.end(),session));
            std::cout << "close user session" << std::endl;
        }
    }

    auto iter = sessions_ssl_.find(session->get_uuid());
    if (iter != sessions_ssl_.end() ){
        sessions_ssl_.erase(session->get_uuid());
    }

}

void
shared_state::
leave(websocket_session *session) {
#ifdef _WINDOWS
    std::string msg = boost::locale::conv::from_utf(session->get_name(), "windows-1251");
#else
    std::string msg = session->get_name();
#endif
    std::cout << "client leave: " << msg << std::endl;

    if(!enable_random_connections){
        if (session->is_authorized() && !session->disable_notify()){
//            // Оповещаем всех об отключении клиента
//            send(std::string (R"({"name": ")") + session->get_name() +
//                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
//                 arcirk::uuids::uuid_to_string(session->get_uuid()) + "\"}", "client_leave", true, session->notify_apps());
        }
    }else{
        if (!session->disable_notify()){
            send(std::string (R"({"name": ")") + session->get_name() +
                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
                 arcirk::uuids::uuid_to_string(session->get_uuid()) + R"(", "command": "client_leave"")" + "\"}");
        }
    }


    std::lock_guard<std::mutex> lock(mutex_);

    auto iter_u = user_sessions.find(session->get_user_uuid());
    if (iter_u != user_sessions.end() ){
        if (iter_u->second.size() <= 1){
            user_sessions.erase(session->get_user_uuid());
            std::cout << "erase user sessions" << std::endl;
        }
        else{
            iter_u->second.erase(find(iter_u->second.begin(), iter_u->second.end(),session));
            std::cout << "close user session" << std::endl;
        }
    }

    auto iter = sessions_.find(session->get_uuid());
    if (iter != sessions_.end() ){
        sessions_.erase(session->get_uuid());
    }

}

void shared_state::send(const std::string &message) {

    auto const ss = boost::make_shared<std::string const>(message);
    if(!enable_ssl){
        std::vector<boost::weak_ptr<websocket_session>> v;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            v.reserve(sessions_.size());
            for(auto p : sessions_){
                v.emplace_back(p.second->weak_from_this());
            }
        }
        for(auto const& wp : v)
            if(auto sp = wp.lock())
                sp->send(ss);
    }else{
        std::vector<boost::weak_ptr<websocket_session_ssl>> v;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            v.reserve(sessions_ssl_.size());
            for(auto p : sessions_ssl_){
                v.emplace_back(p.second->weak_from_this());
            }
        }
        for(auto const& wp : v)
            if(auto sp = wp.lock())
                sp->send(ss);
    }
}

void shared_state::on_start() {

}
