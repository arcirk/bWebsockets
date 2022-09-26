//#include "net.h"

#include <arcirk.hpp>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <boost/asio/io_context.hpp>
#include "callbacks.h"

//#include <boost/fusion/include/for_each.hpp>
//#include <boost/fusion/adapted/struct/define_struct.hpp>

#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

#include "client_certificates.hpp"

class session_base;

using namespace arcirk;

namespace ssl = boost::asio::ssl;

class ws_client final{


    std::unordered_set<session_base*> sessions_;
    std::mutex mutex_;

public:
    explicit
    ws_client(boost::asio::io_context &io_context, const std::string& client_param = "");

    void open(const char* host, const char* port, const callback_message& message = {}, const callback_status& status_changed = {}, const callback_connect& connect = {}, const callback_error& err = {}, const callback_close& close = {}, const std::string & auth = "");
    void open(arcirk::Uri& url, ssl::context& ctx, const callback_message& message = {}, const callback_status& status_changed = {}, const callback_connect& connect = {}, const callback_error& err = {}, const callback_close& close = {}, const std::string & auth = "");

    void send(const std::string &message, const boost::uuids::uuid &recipient = {}, const boost::uuids::uuid &uuid_form = {});
    void send_command(const std::string &cmd, const boost::uuids::uuid &uuid_form, const std::string &json_param);

    void close(bool exit_base = false);
    bool started();

    boost::uuids::uuid& session_uuid();
    std::string& app_name();

    boost::uuids::uuid& user_uuid();
    void set_user_uuid(const std::string& uuid);
    [[nodiscard]] std::string user_name() const;
    void set_user_name(const std::string& name);

    void on_connect(session_base * sess);
    void on_read(const std::string& message);
    void on_stop();
    void on_error(const std::string &what, const std::string &err, int code);

    void set_cert_file(const std::string& file);

private:
    callback_message _on_message;
    callback_status _on_status_changed;
    callback_connect _on_connect;
    callback_error  _on_error;
    callback_close _on_close;

    boost::asio::io_context &ioc;

    std::string _client_param;

    std::string _app_name;
    boost::uuids::uuid _session_uuid{};

    boost::uuids::uuid  _user_uuid{};
    std::string _user_name;
    bool _exit_parent = false;

    boost::filesystem::path cert_file;

    static void console_log(const std::string& log){
        std::cout << arcirk::local_8bit(log) << std::endl;
    }

    void send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param, session_base * sess);

    void set_param(ServerResponse& resp);

    void set_session_uuid(const std::string& uuid);

    void set_certificates(ssl::context& ctx);

};