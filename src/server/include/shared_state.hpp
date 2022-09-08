//
// Created by arcady on 05.07.2021.
//

#ifndef SHARED_STATE_HPP
#define SHARED_STATE_HPP

#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <map>
#include <set>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators e

#include "subscriber.hpp"
//#include "../include/base.h"

#include <arcirk.hpp>
#include <boost/variant.hpp>

enum bConfFields{
    ServerHost = 0,
    ServerPort,
    ServerUser,
    ServerUserHash,
    ServerName,
    ServerHttpRoot,
    AutoConnect,
    UseLocalWebDavDirectory,
    LocalWebDavDirectory,
    WebDavHost,
    WebDavUser,
    WebDavPwd,
    WebDavSSL,
    SQLFormat,
    SQLHost,
    SQLUser,
    SQLPassword,
    HSHost,
    HSUser,
    HSPassword,
    ServerSSL,
    SSL_csr_file,
    SSL_key_file,
};

using namespace arcirk;

class websocket_session;
class websocket_session_ssl;

//typedef boost::variant<websocket_session*, websocket_session_ssl*> sessions__;

class subscriber;

// Represents the shared server state
class shared_state
{
    std::string const doc_root_;

    // This mutex synchronizes all access to sessions_
    std::mutex mutex_;

    std::map<boost::uuids::uuid, websocket_session*> sessions_; //сессии
    std::map<boost::uuids::uuid, websocket_session_ssl*> sessions_ssl_; //сессии

    std::map<boost::uuids::uuid, std::vector<websocket_session*>> user_sessions; //все сессии пользователя
    std::map<boost::uuids::uuid, std::vector<websocket_session_ssl*>> user_sessions_ssl; //все сессии пользователя

public:
    explicit
    shared_state(std::string doc_root, bool is_ssl = false);
    ~shared_state()= default;

    [[nodiscard]] std::string const&
    doc_root() const noexcept
    {
        return doc_root_;
    }

    void join  (websocket_session* sessions);
    void join  (websocket_session_ssl* sessions);

    void leave (websocket_session* session);
    void leave (websocket_session_ssl* session);
    void send(const std::string& message);
    //void send(const std::string& message, websocket_session* session);

    void on_start();

private:
    bool enable_random_connections;
    bool enable_ssl;

};


#endif //SHARED_STATE_HPP
