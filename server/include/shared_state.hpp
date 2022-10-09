//
// Created by arcady on 05.07.2021.
//

#ifndef BWEBSOCKETS_SHARED_STATE_HPP
#define BWEBSOCKETS_SHARED_STATE_HPP

#include <arcirk.hpp>
#include <shared_struct.hpp>
#include <database_struct.hpp>

#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <map>
#include <set>
#include <variant>
#include <vector>
#include <ctime>

#include "http.hpp"


template<class Derived>
class websocket_session;
class plain_websocket_session;
class ssl_websocket_session;
class subscriber;

template<class... Ts>
struct overloaded : Ts ... {
    using Ts::operator()...;
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

typedef std::variant<
        std::monostate,
        int32_t,
        double,
        bool,
        std::string,
        std::tm,
        std::vector<char>
> variant_t;

using namespace arcirk;
using namespace public_struct;

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
    static inline void read_conf(server::server_config & result){

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
                    result = pre::json::from_json<server::server_config>(str);
                }
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

    }

    static inline void write_conf(server::server_config & conf, const boost::filesystem::path& root_conf) {
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

    [[nodiscard]] bool use_authorization() const;

    bool verify_connection(const std::string& basic_auth);

    //команды сервера
    std::string get_clients_list(const variant_t& param, const variant_t& session_id);
    std::string server_version(const variant_t& session_id);
    std::string set_client_param(const variant_t& param, const variant_t& session_id);

    template<typename T, typename C, typename ... Ts>
    void add_method(const std::string &alias, C *c, T(C::*f)(Ts ...),
                   std::map<long, variant_t> &&def_args = {});

    bool call_as_proc(const long& method_num, std::vector<variant_t> params);

    bool call_as_func(const long& method_num, variant_t *ret_value, std::vector<variant_t> params);

    long find_method(const std::string& method_name);
    [[nodiscard]] std::string get_method_name(const long& num) const;

    [[nodiscard]] std::string base64_to_string(const std::string& base64str) const;

private:
    subscriber* get_session(boost::uuids::uuid &uuid);
    std::vector<subscriber *> get_sessions(boost::uuids::uuid &user_uuid);

    class MethodMeta;

    template<size_t... Indices>
    static auto ref_tuple_gen(std::vector<variant_t> &v, std::index_sequence<Indices...>);

    server::server_config sett;
    std::vector<MethodMeta> methods_meta;

    [[nodiscard]] bool verify_auth(const std::string& usr, const std::string& pwd) const;
    static bool is_cmd(const std::string& message) { return message.substr(0, 3) == "cmd";};
    static bool is_msg(const std::string& message) { return message.substr(0, 3) == "msg";};
    void execute_command_handler(const std::string& message, subscriber *session);
    void forward_message(const std::string& message, subscriber *session);

    static void fail(const std::string& what, const std::string& error, bool conv = true){
        if(conv)
            std::cerr << what << ": " << arcirk::local_8bit(error) << std::endl;
        else
            std::cerr << what << ": " << error << std::endl;
    };

    static void log(const std::string& what, const std::string& message, bool conv = true){
        if(conv)
            std::cout << what << ": " << arcirk::local_8bit(message) << std::endl;
        else
            std::cout << what << ": " <<message << std::endl;
    };

    [[nodiscard]] long param_count(const long& method_num) const;

};

class shared_state::MethodMeta{
public:
    MethodMeta &operator=(const MethodMeta &) = delete;
    MethodMeta(const MethodMeta &) = delete;
    MethodMeta(MethodMeta &&) = default;
    MethodMeta &operator=(MethodMeta &&) = default;

    std::string alias;
    long params_count;
    bool returns_value;
    std::map<long, variant_t> default_args;
    std::function<variant_t(std::vector<variant_t> &params)> call;
};

template<size_t... Indices>
auto shared_state::ref_tuple_gen(std::vector<variant_t> &v, std::index_sequence<Indices...>) {
    return std::forward_as_tuple(v[Indices]...);
}

template<typename T, typename C, typename ... Ts>
void shared_state::add_method(const std::string &alias, C *c, T(C::*f)(Ts ...),
                             std::map<long, variant_t> &&def_args) {

    MethodMeta meta{alias, sizeof...(Ts), !std::is_same<T, void>::value, std::move(def_args),
                    [f, c](std::vector<variant_t> &params) -> variant_t {
                        auto args = ref_tuple_gen(params, std::make_index_sequence<sizeof...(Ts)>());
                        if constexpr (std::is_same<T, void>::value) {
                            std::apply(f, std::tuple_cat(std::make_tuple(c), args));
                            return UNDEFINED;
                        } else {
                            return std::apply(f, std::tuple_cat(std::make_tuple(c), args));
                        }
                    }
    };

    methods_meta.push_back(std::move(meta));
};


#endif //BWEBSOCKETS_SHARED_STATE_HPP
