//#include "net.h"

#include "../../arcirk/include/arcirk.h"
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <boost/asio/io_context.hpp>
//#include "../include/sync_session.h"

using boost::property_tree::ptree;

class session;

using std::placeholders::_1;
using std::placeholders::_2;

typedef std::function<void(std::string)> _callback_message;
typedef std::function<void(bool)> _callback_status;
typedef std::function<void()> _callback_connect;

using namespace arcirk;

class ws_client final{

    std::unordered_set<session*> sessions_;
    std::mutex mutex_;

public:
    explicit
    ws_client(boost::asio::io_context &io_context, const std::string& client_param = "");

    void open(const char *host, const char *port);
    void open(const char *host, const char *port, const char *name);
    void open(const char *host, const char *port, const char *name, const char *uuid);
    void open(const char* host, const char* port, _callback_message& msg);
    void open(const char* host, const char* port, _callback_message& msg, _callback_status& st, _callback_connect& cn);
    void send(const std::string &message, bool is_cmd, const std::string &sub_user_uuid = "", const std::string &uuid_form = "",
              const std::string &command = "", const std::string &objectName = "", const std::string &msg_ref = "");
//    void
//    to_channel(const std::string &message, const std::string& uuid_sub, const std::string& uuid_form);
    void on_connect(session * sess);
    void on_read(const std::string& message);
    void on_stop();
    void close(bool exit_base = false);
    bool started();
    boost::uuids::uuid& get_uuid();
    boost::uuids::uuid& get_user_uuid();
    std::string get_name() const;
    std::string get_sys_user_name() const;
    void set_name(const std::string& name);
    std::string& get_app_name();
    void set_uuid(const std::string& uuid);
    void set_user_uuid(const std::string& uuid);
    void set_user_uuid();
    void set_uuid();

    std::string get_client_info();

    void error(const std::string &what, const std::string &err);

    static boost::uuids::uuid string_to_uuid(const std::string& uuid){return string_to_uuid(uuid);};
    static std::string get_hash(const std::string& name,
                                const std::string& pwd){return get_sha1(name + pwd);};
//    void subscribe_to_channel(const std::string &uuid_channel, const std::string &uuid_form);
//    void subscribe_exit_channel(const std::string &uuid_channel, const std::string &uuid_form);
//    void set_session_param(const std::string &uuid, const std::string &name, const std::string &pwd, const std::string &app_name, const std::string &user_uuid);

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &json_param);

    bool decode_message;

    bool is_login(){return _is_login;}

    void set_webdav_settings_on_client(const std::string& sett);

    void set_webdav_directory(const std::string& dir);
    void use_webdav_local_dir(bool val);
    std::string get_webdav_user() const;
    std::string get_webdav_pwd() const;
    std::string get_webdav_host() const;
    bool get_webdav_ssl();



private:

    boost::asio::io_context &ioc;
    boost::uuids::uuid uuid_{};
    std::string name_;
    _callback_message _callback_msg;
    _callback_status status_changed;
    _callback_connect callback_connect;
    std::string _client_param;
    std::string _app_name;
    boost::uuids::uuid  _user_uuid{};
    bool _is_login;
    std::string _user_name;
    std::string _webdav_host;
    std::string _webdav_user;
    std::string _webdav_pwd;
    bool _webdav_use_local;
    std::string _webdav_local_dir;
    bool _webdav_ssl;
    bool _exit_parent = false;

    //std::vector<content_value> m_custom_param;


    static void console_log(const std::string logText){
        std::cout << logText << std::endl;
    }

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param, session * sess);

    void set_param(ptree& pt);

    boost::uuids::uuid agent_session; //идентификатор сессии агента под управлением которго текущая сессия
    boost::uuids::uuid current_client_session; //идентификатор клиента если текущая сессия является агентом.

};
