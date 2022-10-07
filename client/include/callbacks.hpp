//
// Created by admin on 10.09.2022.
//

#ifndef ARCIRK_SOLUTION_CALLBACKS_HPP
#define ARCIRK_SOLUTION_CALLBACKS_HPP

#include <iostream>
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>
#include <nlohmann/json.hpp>

typedef std::function<void(const std::string&)> callback_message;
typedef std::function<void(bool)> callback_status;
typedef std::function<void()> callback_connect;
typedef std::function<void(const std::string&, const std::string&, int)> callback_error;
typedef std::function<void()> callback_close;

BOOST_FUSION_DEFINE_STRUCT(
(arcirk::client), ClientParam,
(std::string, app_name)
(std::string, user_uuid)
(std::string, user_name)
(std::string, hash)
(std::string, host_name)
(std::string, password)
)

BOOST_FUSION_DEFINE_STRUCT(
(arcirk::client), ServerResponse,
(std::string, command)
(std::string, message)
)

namespace arcirk::client{

    typedef boost::variant<callback_message, callback_status, callback_connect, callback_error, callback_close> callbacks;

    struct bClientData{

        std::string host;
        int port;
        callback_message on_message;
        callback_status on_status_changed;
        callback_connect on_connect;
        callback_error  on_error;
        callback_close on_close;
        bool exitParent;
        bool is_ssl;
        bool isRun;
        boost::uuids::uuid session_uuid;

        public:
            explicit
            bClientData()
            : host("localhost")
                    , port(8080)
                    , on_message(nullptr)
                    , on_status_changed(nullptr)
                    , on_connect(nullptr)
                    , on_error(nullptr)
                    , on_close(nullptr)
                    , exitParent(false)
                    , isRun(false)
                    , is_ssl(false)
                    , session_uuid(boost::uuids::nil_uuid())
            {}
    };

    enum bClientEvent{
        wsMessage = 0,
        wsStatusChanged,
        wsConnect,
        wsClose,
        wsError
    };

    enum ServerPublicCommands{
        ServerVersion,
        ServerOnlineClientsList,
        TS_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ServerPublicCommands, {
        {TS_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,

    })
}

#endif //ARCIRK_SOLUTION_CALLBACKS_H
