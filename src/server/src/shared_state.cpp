#include "../include/shared_state.hpp"

#include <utility>
#include "../include/session_base.hpp"
#include "../include/websocket_session.hpp"
#include "../include/websocket_session_ssl.hpp"

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#ifdef _WINDOWS
#pragma warning(disable:4100)
#endif

shared_state::
shared_state(std::string doc_root, server_settings& settings, bool is_ssl, bool use_auth)
        : doc_root_(std::move(doc_root))

{
    //enable_random_connections = true;
    enable_ssl = is_ssl;
    _use_authorization = use_auth;
    srv_settings = settings;
    connect_to_database();
}

void shared_state::join(session_base *session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(std::pair<boost::uuids::uuid, session_base*>(session->uuid(), session));
}
void
shared_state::
leave(session_base* session)
{

    std::cout << "client leave: " << arcirk::local_8bit(session->user_name()) << std::endl;

//    if(!enable_random_connections){
//        if (session->is_authorized() && !session->disable_notify()){
////            // Оповещаем всех об отключении клиента
////            send(std::string (R"({"name": ")") + session->get_user_name() +
////                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
////                 arcirk::uuids::uuid_to_string(session->get_uuid()) + "\"}", "client_leave", true, session->notify_apps());
//        }
//    }else{
        if (!session->disable_notify()){
            send(std::string (R"({"name": ")") + session->user_name() +
                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->user_uuid()) + R"(", "uuid": ")" +
                 arcirk::uuids::uuid_to_string(session->uuid()) + R"(", "command": "client_leave"")" + "\"}");
        }
    //}


    std::lock_guard<std::mutex> lock(mutex_);

    auto iter_u = user_sessions.find(session->user_uuid());
    if (iter_u != user_sessions.end() ){
        if (iter_u->second.size() <= 1){
            user_sessions.erase(session->user_uuid());
            std::cout << "erase user sessions" << std::endl;
        }
        else{
            iter_u->second.erase(find(iter_u->second.begin(), iter_u->second.end(),session));
            std::cout << "close user session" << std::endl;
        }
    }

    auto iter = sessions_.find(session->uuid());
    if (iter != sessions_.end() ){
        sessions_.erase(session->uuid());
    }

}

//void
//shared_state::
//leave(websocket_session_ssl* session)
//{
//
//    std::cout << "client leave: " << arcirk::local_8bit(session->get_name()) << std::endl;
//
//    if(!enable_random_connections){
//        if (session->is_authorized() && !session->disable_notify()){
////            // Оповещаем всех об отключении клиента
////            send(std::string (R"({"name": ")") + session->get_user_name() +
////                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
////                 arcirk::uuids::uuid_to_string(session->get_uuid()) + "\"}", "client_leave", true, session->notify_apps());
//        }
//    }else{
//        if (!session->disable_notify()){
//            send(std::string (R"({"name": ")") + session->get_name() +
//                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
//                 arcirk::uuids::uuid_to_string(session->get_uuid()) + R"(", "command": "client_leave"")" + "\"}");
//        }
//    }
//
//
//    std::lock_guard<std::mutex> lock(mutex_);
//
//    auto iter_u = user_sessions_ssl.find(session->get_user_uuid());
//    if (iter_u != user_sessions_ssl.end() ){
//        if (iter_u->second.size() <= 1){
//            user_sessions_ssl.erase(session->get_user_uuid());
//            std::cout << "erase user sessions" << std::endl;
//        }
//        else{
//            iter_u->second.erase(find(iter_u->second.begin(), iter_u->second.end(),session));
//            std::cout << "close user session" << std::endl;
//        }
//    }
//
//    auto iter = sessions_ssl_.find(session->get_uuid());
//    if (iter != sessions_ssl_.end() ){
//        sessions_ssl_.erase(session->get_uuid());
//    }
//
//}
//
//void
//shared_state::
//leave(websocket_session *session) {
//
//    std::string msg = arcirk::local_8bit(session->get_name());
//    std::cout << "client leave: " << msg << std::endl;
//
//    if(!enable_random_connections){
//        if (session->is_authorized() && !session->disable_notify()){
////            // Оповещаем всех об отключении клиента
////            send(std::string (R"({"name": ")") + session->get_user_name() +
////                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
////                 arcirk::uuids::uuid_to_string(session->get_uuid()) + "\"}", "client_leave", true, session->notify_apps());
//        }
//    }else{
//        if (!session->disable_notify()){
//            send(std::string (R"({"name": ")") + session->get_name() +
//                 R"(", "uuid_user": ")" + arcirk::uuids::uuid_to_string(session->get_user_uuid()) + R"(", "uuid": ")" +
//                 arcirk::uuids::uuid_to_string(session->get_uuid()) + R"(", "command": "client_leave"")" + "\"}");
//        }
//    }
//
//
//    std::lock_guard<std::mutex> lock(mutex_);
//
//    auto iter_u = user_sessions.find(session->get_user_uuid());
//    if (iter_u != user_sessions.end() ){
//        if (iter_u->second.size() <= 1){
//            user_sessions.erase(session->get_user_uuid());
//            std::cout << "erase user sessions" << std::endl;
//        }
//        else{
//            iter_u->second.erase(find(iter_u->second.begin(), iter_u->second.end(),session));
//            std::cout << "close user session" << std::endl;
//        }
//    }
//
//    auto iter = sessions_.find(session->get_uuid());
//    if (iter != sessions_.end() ){
//        sessions_.erase(session->get_uuid());
//    }
//
//}

void shared_state::send(const std::string &message) {

    auto const ss = boost::make_shared<std::string const>(message);
//    if(!enable_ssl){
        std::vector<boost::weak_ptr<session_base>> v;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            v.reserve(sessions_.size());
            for(auto p : sessions_){
                if(!enable_ssl) {
                    auto obj = (websocket_session*)p.second;
                    v.emplace_back(obj->weak_from_this());
                }else{
                    auto obj = (websocket_session_ssl*)p.second;
                    v.emplace_back(obj->weak_from_this());
                }
            }
        }
        for(auto const& wp : v)
            if(auto sp = wp.lock())
                sp->send(ss);
//    }else{
//        std::vector<boost::weak_ptr<websocket_session_ssl>> v;
//        {
//            std::lock_guard<std::mutex> lock(mutex_);
//            v.reserve(sessions_ssl_.size());
//            for(auto p : sessions_ssl_){
//                v.emplace_back(p.second->weak_from_this());
//            }
//        }
//        for(auto const& wp : v)
//            if(auto sp = wp.lock())
//                sp->send(ss);
//    }
}

void shared_state::on_start() {

}

//bool shared_state::use_authorization() const {
//    return _use_authorization;
//}

void shared_state::run_command(const std::string &response, session_base *session) {

}

void shared_state::command_to_server(const std::string& response) {

}

void
shared_state::deliver(const std::string& message, session_base *session)
{

    bool is_cmd = message.substr(0, 3) == "cmd";

    if(is_cmd){
        arcirk::T_vec v = arcirk::split(message, " ");
        if(v.size() > 1)
            run_command(v[1], session);
    }


}

bool shared_state::connect_to_database() {

    if(srv_settings.SQLFormat == arcirk::DatabaseType::dbTypeODBC){

    }else{

    }
    return false;
}

bool shared_state::verify_user(const std::string &basic_header) {

    if(basic_header.empty()){
        return false;
    }
    arcirk::T_vec val = arcirk::split(basic_header, " ");
    if(val.size() != 2)
        return false;
    try {
        std::string sz = val[1];
        boost::trim(sz);
        std::string auth = arcirk::base64::base64_decode(sz);
        arcirk::T_vec vec = split(auth, ":");
        if (vec.size() != 2)
            return false;
        else {
            std::string hash = arcirk::get_hash(vec[0], vec[1]);
            if (srv_settings.SQLFormat == arcirk::DatabaseType::dbTypeSQLite) {
                using namespace boost::filesystem;
                path db(srv_settings.ApplicationProfile);
                db = db / +"data" / +"arcirk.sqlite";
                if (!exists(db))
                    return false;
                else {
                    std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true",
                                                                       db.string());
                    soci::session sql(soci::sqlite3, connection_string);
                    int count = -1;
                    sql << "select count(*) from Users where hash = " << "'" << hash << "'", soci::into(count);
                    return count > 0;
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return false;

}
