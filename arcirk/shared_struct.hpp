#ifndef ARCIRK_SHARED_MODULE_HPP
#define ARCIRK_SHARED_MODULE_HPP

#include "arcirk.hpp"
#include "includes.hpp"

#define ARCIRK_PROJECT "arcirk"
#define ARCIRK_VERSION "1.1.1"
#define CLIENT_VERSION 3
#define ARCIRK_SERVER_CONF "server_conf.json"
#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"
#define SHARED_CHANNEL_UUID "3e3b54bf-4319-4e73-9917-22f06cc1bfbd"
#define MESSAGES_DATE_GROUP_UUID "78cb038c-4761-442e-b2d5-01c359dc2a53"
#define WS_RESULT_SUCCESS "success"
#define WS_RESULT_ERROR "error"

typedef std::function<void(const std::string&)> callback_message;
typedef std::function<void(bool)> callback_status;
typedef std::function<void()> callback_connect;
typedef std::function<void(const std::string&, const std::string&, int)> callback_error;
typedef std::function<void()> callback_close;
typedef std::function<void()> callback_successful_authorization;

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), version_application,
        (int, major)
        (int, minor)
        (int, path)
)

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
        (std::string, info_base)
        (std::string, product)
        (std::string, sid)
        (int, version)
)

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), session_info,
        (std::string, session_uuid)
        (std::string, user_name)
        (std::string, user_uuid)
        (std::string, start_date)
        (std::string, app_name)
        (std::string, role)
        (std::string, device_id)
        (std::string, address)
        (std::string, info_base)
        (std::string, host_name)
        (std::string, product)
        (std::string, system_user)
        (std::string, sid)
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
        (arcirk::ByteArray, data)
)
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::server), server_command_result,
        (std::string, command)
        (std::string, uuid_form)
        (std::string, result)
        (std::string, message)
        (std::string, error_description)
        (std::string, param)
        (arcirk::ByteArray, data)

)

class server_commands_exception : public std::exception
{
private:
    std::string m_error{}; // handle our own string
    std::string uuid_form_{};
    std::string command_{};
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

    static inline version_application get_version(){
        T_vec vec = arcirk::split(ARCIRK_VERSION, ".");
         auto ver = version_application();
         ver.major = std::stoi(vec[0]);
         ver.minor = std::stoi(vec[1]);
         ver.path = std::stoi(vec[2]);
        return ver;
    }

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
        SyncUpdateBarcode, //синхронизирует на сервере штрихкод и номенклатуру по запросу клиента с сервером 1с
        DownloadFile, //Загружает файл на сервер
        GetInformationAboutFile, //получить информацию о файле
        CheckForUpdates, //поиск фалов обрновления
        UploadFile, //скачать файл
        GetDatabaseTables,
        FileToDatabase, //Загрузка таблицы базы данных из файла
        ProfileDirFileList,
        ProfileDeleteFile,
        DeviceGetFullInfo,
        GetTasks,
        UpdateTaskOptions,
        TasksRestart,
        RunTask,
        StopTask,
        SendNotify,
        GetCertUser,
        VerifyAdministrator,
        UserMessage,
        GetChannelToken,
        IsChannel,
        GetDatabaseStructure,
        Run1CScript,
        CreateDirectories,
        DeleteDirectory,
        bDeleteFile,
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
        {DownloadFile, "DownloadFile"}    ,
        {GetInformationAboutFile, "GetInformationAboutFile"}    ,
        {CheckForUpdates, "CheckForUpdates"}    ,
        {UploadFile, "UploadFile"}    ,
        {GetDatabaseTables, "GetDatabaseTables"}    ,
        {FileToDatabase, "FileToDatabase"}    ,
        {ProfileDirFileList, "ProfileDirFileList"}    ,
        {ProfileDeleteFile, "ProfileDeleteFile"}    ,
        {DeviceGetFullInfo, "DeviceGetFullInfo"}    ,
        {GetTasks, "GetTasks"}    ,
        {UpdateTaskOptions, "UpdateTaskOptions"}    ,
        {TasksRestart, "TasksRestart"}    ,
        {RunTask, "RunTask"}    ,
        {StopTask, "StopTask"}    ,
        {SendNotify, "SendNotify"}    ,
        {GetCertUser, "GetCertUser"}    ,
        {VerifyAdministrator, "VerifyAdministrator"}    ,
        {UserMessage, "UserMessage"}    ,
        {GetChannelToken, "GetChannelToken"}    ,
        {IsChannel, "IsChannel"}    ,
        {GetDatabaseStructure, "GetDatabaseStructure"}    ,
        {Run1CScript, "Run1CScript"}    ,
        {CreateDirectories, "CreateDirectories"}    ,
        {DeleteDirectory, "DeleteDirectory"}    ,
        {bDeleteFile, "DeleteFile"}    ,
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
        (std::string, WebDavRoot)
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
        (std::string, ServerProtocol)
        (bool, WriteJournal)
        (bool, AllowIdentificationByWINSID)
);

#endif