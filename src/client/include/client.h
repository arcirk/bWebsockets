//#include "net.h"

#include <arcirk.hpp>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <boost/asio/io_context.hpp>

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

    void send(const std::string &message, const boost::uuids::uuid &recipient = {}, const boost::uuids::uuid &uuid_form = {});
    void send_command(const std::string &cmd, const boost::uuids::uuid &uuid_form, const std::string &json_param);

    void on_connect(session * sess);
    void on_read(const std::string& message);
    void on_stop();
    void close(bool exit_base = false);
    bool started();

    boost::uuids::uuid& get_uuid();
    boost::uuids::uuid& get_user_uuid();
    [[nodiscard]] std::string get_user_name() const;
    void set_user_name(const std::string& name);
    std::string& get_app_name();
    void set_user_uuid(const std::string& uuid);
    void set_user_uuid();

    void error(const std::string &what, const std::string &err);



private:
    bool decode_message;
    boost::asio::io_context &ioc;
    boost::uuids::uuid uuid_{};
    std::string name_;
    _callback_message _on_message;
    _callback_status _status_changed;
    _callback_connect _on_connect;
    std::string _client_param;
    std::string _app_name;
    boost::uuids::uuid  _user_uuid{};
    std::string _user_name;

    bool _exit_parent = false;

    static void console_log(const std::string log){
        std::cout << arcirk::local_8bit(log) << std::endl;
    }

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param, session * sess);

    void set_param(arcirk::json::bJson& pt);

    void set_uuid(const std::string& uuid);
    void set_uuid();

};
