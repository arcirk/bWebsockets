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

template<class Derived>
class websocket_session;
class plain_websocket_session;
class ssl_websocket_session;
class subscriber;

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

//typedef std::function<void(const std::string&, const std::string&, const std::string&, const std::string&)> server_events;

namespace arcirk{
    enum DatabaseType{
        dbTypeSQLite = 0,
        dbTypeODBC
    };

    static inline boost::filesystem::path program_data(){
        using namespace boost::filesystem;

        std::string arcirk_dir = "arcirk";
#ifndef _WINDOWS
        arcirk_dir = "." + arcirk_dir;
#endif

        path app_conf(arcirk::standard_paths::this_server_conf_dir(arcirk_dir));

        return app_conf;
    }
    static inline void read_conf(server_settings & result){

        //файл конфигурации всегда лежит либо в домашней папке на линуксе либо в programdata на windows
        using namespace boost::filesystem;

        if(!exists(program_data()))
            return;

        try {
            path conf = program_data() /+ "server_conf.json";

            if(exists(conf)){
                std::ifstream file(conf.string(), std::ios_base::in);
                std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
                if(!str.empty()){
                    result = pre::json::from_json<server_settings>(str);
                }
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

    }

    static inline void write_conf(server_settings & conf, const boost::filesystem::path& root_conf) {
        using namespace boost::filesystem;

        if (!exists(arcirk::local_8bit(root_conf.string())))
            return;
        try {
            std::string result = to_string(pre::json::to_json(conf));
            std::ofstream out;
            path conf_file = program_data() / +"server_conf.json";
            out.open(arcirk::local_8bit(conf_file.string()));
            if (out.is_open()) {
                out << result;
                out.close();
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

    }
}


// Represents the shared server state
class shared_state
{
    std::map<boost::uuids::uuid const, subscriber*> sessions_;
    std::map<boost::uuids::uuid, std::vector<subscriber*>> user_sessions;
    std::mutex mutex_;

public:
    explicit
    shared_state();

    ~shared_state()= default;

    void join(subscriber* session);

    void leave(const boost::uuids::uuid& session_uuid, const std::string& user_name);
    void deliver(const std::string& message, subscriber* session);
    template<typename T>
    void send(const std::string& message);

    bool use_authorization() const;

    bool verify_connection(const std::string& basic_auth);
private:
    public_struct::server_settings sett;

};


#endif //BWEBSOCKETS_SHARED_STATE_HPP
