#ifndef ARCIRK_SHARED_MODULE_HPP
#define ARCIRK_SHARED_MODULE_HPP

#include "includes.hpp"

#define ARCIRK_VERSION "1.1.0"
#define ARCIRK_SERVER_CONF "server_conf.json"

typedef std::function<void(const std::string&)> callback_message;
typedef std::function<void(bool)> callback_status;
typedef std::function<void()> callback_connect;
typedef std::function<void(const std::string&, const std::string&, int)> callback_error;
typedef std::function<void()> callback_close;
typedef std::function<void()> callback_successful_authorization;


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
                (std::string, device_id)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_response,
        (std::string, command)
                (std::string, message)
                (std::string, param)
                (std::string, result)
                (std::string, sender)
                (std::string, receiver)
                (std::string, uuid_form)
                (std::string, app_name)
                (std::string, sender_name)
                (std::string, sender_uuid)
                (std::string, receiver_name)
                (std::string, receiver_uuid)
                (std::string, version)
)
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_command_result,
        (std::string, command)
                (std::string, uuid_form)
                (std::string, result)
                (std::string, message)
                (std::string, error_description)
                (std::string, param)

)

class server_commands_exception : public std::exception
{
private:
    std::string m_error{}; // handle our own string
    std::string uuid_form_{};
    std::string command_{};
//    std::string result_{};
//    std::string message_{};
//    std::string error_description_{};
public:
    server_commands_exception(std::string_view error, std::string_view command, std::string_view uuid_form)
            : m_error{error},
              uuid_form_{uuid_form},
              command_{command}
    {
    }

    [[nodiscard]] const char* what() const noexcept override { return m_error.c_str(); }
    [[nodiscard]] const char* uuid_form() const noexcept { return uuid_form_.c_str(); }
    [[nodiscard]] const char* command() const noexcept { return command_.c_str(); }
};

namespace arcirk::client{

    typedef boost::variant<callback_message, callback_status, callback_connect, callback_error, callback_close, callback_successful_authorization> callbacks;

    struct client_data{

        std::string host;
        int port;
        callback_message on_message;
        callback_status on_status_changed;
        callback_connect on_connect;
        callback_error  on_error;
        callback_close on_close;
        callback_successful_authorization on_successful_authorization;
//        bool exitParent;
//        bool is_ssl;
//        bool isRun;
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
                , on_successful_authorization(nullptr)
//                , exitParent(false)
//                , isRun(false)
//                , is_ssl(false)
                , session_uuid(boost::uuids::nil_uuid())
        {}
    };

    enum client_events{
        wsMessage,
        wsStatusChanged,
        wsConnect,
        wsClose,
        wsError,
        wsSuccessfulAuthorization,
        TS_INVALID=-1,
    };


    NLOHMANN_JSON_SERIALIZE_ENUM(client_events, {
        {TS_INVALID, nullptr}    ,
        {wsMessage, "WebCore::Message"}  ,
        {wsStatusChanged, "WebCore::StatusChanged"}    ,
        {wsConnect, "WebCore::Connect"}    ,
        {wsClose, "WebCore::Close"}    ,
        {wsError, "WebCore::Error"}    ,
        {wsSuccessfulAuthorization, "WebCore::SuccessfulAuthorization"}    ,

    })

}

namespace arcirk::server{

    enum server_commands{
        ServerVersion, //Версия сервера
        ServerOnlineClientsList, //Список активных пользователей
        SetClientParam, //Параметры клиента
        ServerConfiguration, //Конфигурация сервера
        UserInfo, //Информация о пользователе (база данных)
        InsertOrUpdateUser, //Обновить или добавить пользователя (база данных)
        CommandToClient, //Команда клиенту (подписчику)
        ServerUsersList, //Список пользователей (база данных)
        ExecuteSqlQuery, //выполнить запрос к базе данных
        GetMessages, //Список сообщений
        UpdateServerConfiguration, //Обновить конфигурацию сервера
        HttpServiceConfiguration, //Получить конфигурацию http сервиса 1С
        InsertToDatabaseFromArray, //Добавить массив записей в базу //устарела удалить
        SetNewDeviceId, //Явная установка идентификатора на устройствах где не возможно его получить
        ObjectSetToDatabase, //Синхронизация объекта 1С с базой
        ObjectGetFromDatabase, //Получить объект типа 1С из базы данных для десериализации
        SyncGetDiscrepancyInData, //Получить расхождения в данных между базами на клиенте и на Сервере
        SyncUpdateDataOnTheServer, //Обновляет данные на сервере по запросу клиента
        WebDavServiceConfiguration, //Получить настройки WebDav
        SyncUpdateBarcode,
        CMD_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_commands, {
        {CMD_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,
        {SetClientParam, "SetClientParam"}    ,
        {ServerConfiguration, "ServerConfiguration"}    ,
        {UserInfo, "UserInfo"}    ,
        {InsertOrUpdateUser, "InsertOrUpdateUser"}    ,
        {CommandToClient, "CommandToClient"}    ,
        {ServerUsersList, "ServerUsersList"}    ,
        {ExecuteSqlQuery, "ExecuteSqlQuery"}    ,
        {GetMessages, "GetMessages"}    ,
        {UpdateServerConfiguration, "UpdateServerConfiguration"}    ,
        {HttpServiceConfiguration, "HttpServiceConfiguration"}    ,
        {InsertToDatabaseFromArray, "InsertToDatabaseFromArray"}    ,
        {SetNewDeviceId, "SetNewDeviceId"}    ,
        {ObjectSetToDatabase, "ObjectSetToDatabase"}    ,
        {ObjectGetFromDatabase, "ObjectGetFromDatabase"}    ,
        {SyncGetDiscrepancyInData, "SyncGetDiscrepancyInData"}    ,
        {SyncUpdateDataOnTheServer, "SyncUpdateDataOnTheServer"}    ,
        {WebDavServiceConfiguration, "WebDavServiceConfiguration"}    ,
        {SyncUpdateBarcode, "SyncUpdateBarcode"}    ,
    })

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
        (bool, AutoConnect)
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
        (bool, AllowDelayedAuthorization)
        (bool, AllowHistoryMessages)
        (std::string, ExchangePlan)
);

#endif