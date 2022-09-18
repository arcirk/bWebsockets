//
// Created by arcady on 05.07.2021.
//

#ifndef SHARED_STATE_HPP
#define SHARED_STATE_HPP

#include <arcirk.hpp>
#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <map>
#include <set>

#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>

BOOST_FUSION_DEFINE_STRUCT(
        (public_struct), user_info,
        (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, hash)
                (std::string, role)
                (std::string, performance)
                (std::string, parent)
                (std::string, cache));

BOOST_FUSION_DEFINE_STRUCT(
        (public_struct), server_settings,
        (std::string, ServerHost)
                (int, ServerPort)
                (std::string, ServerUser)
                (std::string, ServerUserHash)
                (std::string, ServerName)
                (std::string, ServerHttpRoot)
                (std::string, AutoConnect)
                (bool, UseLocalWebDavDirectory)
                (std::string, LocalWebDavDirectory)
                (std::string, WebDavHost)
                (std::string, WebDavUser)
                (std::string, WebDavPwd)
                (bool, WebDavSSL)
                (int, SQLFormat)
                (std::string, SQLHost)
                (std::string, SQLUser)
                (std::string, SQLPassword)
                (std::string, HSHost)
                (std::string, HSUser)
                (std::string, HSPassword)
                (bool, ServerSSL)
                (std::string, SSL_csr_file)
                (std::string, SSL_key_file)
                (bool, UseAuthorization)
                (std::string, ApplicationProfile)
);

BOOST_FUSION_DEFINE_STRUCT(
        (public_struct), ServerResponse,
        (std::string, command)
        (std::string, message)
        (std::string, uuid_form)
        (std::string, param)
        );

using namespace arcirk;
using namespace public_struct;

class websocket_session;

typedef std::function<void(const std::string&, const std::string&, const std::string&, const std::string&)> server_events;

namespace arcirk{
    enum DatabaseType{
        dbTypeSQLite = 0,
        dbTypeODBC
    };
}

class subscriber;

// Represents the shared server state
class shared_state
{
    std::string const doc_root_;

    // This mutex synchronizes all access to sessions_
    std::mutex mutex_;

    std::map<boost::uuids::uuid, websocket_session*> sessions_;
    std::map<boost::uuids::uuid, std::vector<websocket_session*>> user_sessions;

    server_settings srv_settings;

//    std::map<boost::uuids::uuid, websocket_session*> sessions_; //сессии
//    std::map<boost::uuids::uuid, websocket_session_ssl*> sessions_ssl_; //сессии

//    std::map<boost::uuids::uuid, std::vector<websocket_session*>> user_sessions; //все сессии пользователя
//    std::map<boost::uuids::uuid, std::vector<websocket_session_ssl*>> user_sessions_ssl; //все сессии пользователя

public:
    explicit
    shared_state(std::string doc_root, server_settings& settings, bool is_ssl = false, bool use_auth = false);
    ~shared_state()= default;

    [[nodiscard]] std::string const&
    doc_root() const noexcept
    {
        return doc_root_;
    }

    void join  (websocket_session* session);
    void leave (websocket_session* session);

//    void join  (websocket_session* sessions);
//    void join  (websocket_session_ssl* sessions);
//
//    void leave (websocket_session* session);
//    void leave (websocket_session_ssl* session);


    void on_start();
    void deliver(const std::string& message, websocket_session* session);
    void send(const std::string& message);

    //[[nodiscard]] bool use_authorization() const;

    void command_to_server(const std::string& response);
    void run_command(const std::string& response, websocket_session *session);


    bool use_authorization() const { return _use_authorization;};
    bool delayed_authorization() const {return _delayed_authorization;};
    bool verify_user(const std::string& basic_header);

private:
    //bool enable_random_connections;
    bool enable_ssl;
    bool _use_authorization;
    bool _delayed_authorization;
    std::map<std::string, server_events> srv_events;

    bool connect_to_database();

};


#endif //SHARED_STATE_HPP
