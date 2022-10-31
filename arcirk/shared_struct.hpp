#ifndef ARCIRK_SHARED_MODULE_HPP
#define ARCIRK_SHARED_MODULE_HPP

#include "includes.hpp"

typedef std::function<void(const std::string&)> callback_message;
typedef std::function<void(bool)> callback_status;
typedef std::function<void()> callback_connect;
typedef std::function<void(const std::string&, const std::string&, int)> callback_error;
typedef std::function<void()> callback_close;

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), client_param,
        (std::string, app_name)
        (std::string, user_uuid)
        (std::string, user_name)
        (std::string, hash)
        (std::string, host_name)
        (std::string, password)
        (std::string, session_uuid)
        (std::string, system_user)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_response,
        (std::string, command)
        (std::string, message)
        (std::string, param)
        (std::string, result)
        (std::string, sender)
        (std::string, receiver)
)
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_command_result,
        (std::string, command)
        (std::string, uuid_form)
        (std::string, result)
)

namespace arcirk::client{

    typedef boost::variant<callback_message, callback_status, callback_connect, callback_error, callback_close> callbacks;

    struct client_data{

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
        client_data()
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

    enum client_events{
        wsMessage,
        wsStatusChanged,
        wsConnect,
        wsClose,
        wsError,
        TS_INVALID=-1,
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(client_events, {
        {TS_INVALID, nullptr}    ,
        {wsMessage, "Message"}  ,
        {wsStatusChanged, "StatusChanged"}    ,
        {wsConnect, "Connect"}    ,
        {wsClose, "Close"}    ,
        {wsError, "Error"}    ,

    })
    static inline std::string synonym(client_events value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };
}

namespace arcirk::server{

    enum server_commands{
        ServerVersion,
        ServerOnlineClientsList,
        SetClientParam,
        TS_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_commands, {
        {TS_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,
        {SetClientParam, "SetClientParam"}    ,

    })

    static inline std::string synonym(server_commands value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };
}

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_config,
        (std::string, ServerHost)
        (int, ServerPort)
        (std::string, ServerUser)
        (std::string, ServerUserHash)
        (std::string, ServerName)
        (std::string, ServerHttpRoot)
        (std::string, ServerWorkingDirectory)
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
        (std::string, SSL_crt_file)
        (std::string, SSL_key_file)
        (bool, UseAuthorization)
        (std::string, ApplicationProfile)
        (int, ThreadsCount)
        (std::string, Version)
        (bool, ResponseTransferToBase64)
);

#endif