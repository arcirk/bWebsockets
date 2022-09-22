//
// Created by arcady on 05.07.2021.
//

#ifndef BWEBSOCKETS_SHARED_STATE_HPP
#define BWEBSOCKETS_SHARED_STATE_HPP

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

#include "http.hpp"

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

typedef std::function<void(const std::string&, const std::string&, const std::string&, const std::string&)> server_events;

namespace arcirk{
    enum DatabaseType{
        dbTypeSQLite = 0,
        dbTypeODBC
    };
}

class plain_websocket_session;
class ssl_websocket_session;
class subscriber;

// Represents the shared server state
class shared_state
{
    std::map<boost::uuids::uuid const, subscriber*> sessions_;
    std::mutex mutex_;

public:
    explicit
    shared_state();

    ~shared_state()= default;

    void join(subscriber* session);
    void leave(const boost::uuids::uuid& session_uuid, const std::string& user_name);
    void deliver(const std::string& message, subscriber* session);
    void send(const std::string& message);

private:

};


#endif //BWEBSOCKETS_SHARED_STATE_HPP
