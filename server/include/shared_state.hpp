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
#include "http_sync_client.hpp"

#include <task.hpp>

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

namespace arcirk{
    static inline boost::filesystem::path program_data(){
        using namespace boost::filesystem;

        std::string arcirk_dir = "arcirk";
#ifndef _WINDOWS
        arcirk_dir = "." + arcirk_dir;
#endif

        path app_conf(arcirk::standard_paths::this_server_conf_dir(arcirk_dir));

        return app_conf;
    }

    static inline boost::filesystem::path app_directory() {

        return boost::filesystem::path(program_data()) /+ ARCIRK_VERSION;

    }

    static inline void read_conf(server::server_config & result, const boost::filesystem::path& root_conf, const std::string& file_name){

        using namespace boost::filesystem;

        try {
            path conf = root_conf /+ file_name.c_str();

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

    static inline void write_conf(server::server_config & conf, const boost::filesystem::path& root_conf, const std::string& file_name) {
        using namespace boost::filesystem;

        if (!exists(arcirk::local_8bit(root_conf.string())))
            return;
        try {
            std::string result = to_string(pre::json::to_json(conf));
            std::ofstream out;
            path conf_file = root_conf /+ file_name.c_str();
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

    //arcirk::services::service_task_scheduler server_task_ = arcirk::services::service_task_scheduler();

public:
    explicit
    shared_state();

    ~shared_state()= default;

    void join(subscriber* session);

    void leave(const boost::uuids::uuid& session_uuid, const std::string& user_name);

    void deliver(const std::string& message, subscriber* session);

    template<typename T>
    void send(const std::string& message, subscriber* skip_session = nullptr);

    void send_notify(const std::string& message, subscriber* skip_session = nullptr, const std::string& notify_command = "notify", const boost::uuids::uuid& sender_uuid = boost::uuids::nil_uuid());

    [[nodiscard]] bool use_authorization() const;

    [[nodiscard]] bool allow_delayed_authorization() const;

    bool verify_connection(const std::string& basic_auth);

    [[nodiscard]] std::string handle_request(const std::string& body, const std::string& basic_auth);

    //команды сервера
    arcirk::server::server_command_result get_clients_list(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_users_list(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result server_version(const variant_t& session_id);
    arcirk::server::server_command_result set_client_param(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result server_configuration(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result update_server_configuration(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result user_information(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result insert_or_update_user(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result command_to_client(const variant_t& param, const variant_t& session_id, const variant_t& session_id_receiver);
    arcirk::server::server_command_result execute_sql_query(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_messages(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_http_service_configuration(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_dav_service_configuration(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result insert_to_database_from_array(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result set_new_device_id(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result object_set_to_database(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result object_get_from_database(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result sync_get_discrepancy_in_data(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result sync_update_data_on_the_server(const variant_t& param, const variant_t& session_id);

    //tasks
    void erase_deleted_mark_objects();
    void synchronize_objects_from_1c();

    void data_synchronization_set_object(const nlohmann::json& object, const std::string& table_name) const;
    [[nodiscard]] nlohmann::json data_synchronization_get_object(const std::string& table_name, const std::string& ref) const;

    static std::string execute_random_sql_query(soci::session& sql, const std::string& query_text) ;

    static auto parse_json(const std::string& json_text, bool is_base64 = false);

    template<typename T, typename C, typename ... Ts>
    void add_method(const std::string &alias, C *c, T(C::*f)(Ts ...),
                    std::map<long, variant_t> &&def_args = {});

    //bool call_as_proc(const long& method_num, std::vector<variant_t> params);
    void call_as_func(const long& method_num, arcirk::server::server_command_result *ret_value, std::vector<variant_t> params);

    long find_method(const std::string& method_name);
    [[nodiscard]] std::string get_method_name(const long& num) const;

    [[nodiscard]] static std::string base64_to_string(const std::string& base64str) ;

    bool edit_table_only_admin(const std::string& table_name);

private:
    std::shared_ptr<arcirk::services::task_scheduler> task_manager;

    void run_server_tasks();
    void exec_server_task(const arcirk::services::task_options& details);

    subscriber* get_session(const boost::uuids::uuid &uuid);
    std::vector<subscriber *> get_sessions(const boost::uuids::uuid &user_uuid);
    [[nodiscard]] arcirk::database::user_info get_user_info(const boost::uuids::uuid &user_uuid) const;
    [[nodiscard]] arcirk::database::user_info get_user_info(const std::string &hash) const;
    static void set_session_info(subscriber* session, const arcirk::database::user_info& info);

    bool is_operation_available(const boost::uuids::uuid &uuid, arcirk::database::roles level);

    [[nodiscard]] boost::filesystem::path sqlite_database_path() const;
    std::string get_channel_token(soci::session& sql, const std::string &first, const std::string &second) const;

    class MethodMeta;

    template<size_t... Indices>
    static auto ref_tuple_gen(std::vector<variant_t> &v, std::index_sequence<Indices...>);

    server::server_config sett;
    std::vector<MethodMeta> methods_meta;

    [[nodiscard]] bool verify_auth(const std::string& usr, const std::string& pwd) const;
    [[nodiscard]] bool verify_auth_from_hash(const std::string& usr, const std::string& hash) const;
    static bool is_cmd(const std::string& message) { return message.substr(0, 3) == "cmd";};
    static bool is_msg(const std::string& message) { return message.substr(0, 3) == "msg";};
    void execute_command_handler(const std::string& message, subscriber *session);
    void forward_message(const std::string& message, subscriber *session);


    [[nodiscard]] soci::session soci_initialize() const;

    static void fail(const std::string& what, const std::string& error, bool conv = true){
        std::tm tm = arcirk::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        if(conv)
            std::cerr << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(error) << std::endl;
        else
            std::cerr << std::string(cur_date) << " " << what << ": " << error << std::endl;
    };

    static void log(const std::string& what, const std::string& message, bool conv = true){
        std::tm tm = arcirk::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        if(conv)
            std::cout << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(message) << std::endl;
        else
            std::cout << std::string(cur_date) << " " << what << ": " <<message << std::endl;
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
    std::function<arcirk::server::server_command_result(std::vector<variant_t> &params)> call;
};

template<size_t... Indices>
auto shared_state::ref_tuple_gen(std::vector<variant_t> &v, std::index_sequence<Indices...>) {
    return std::forward_as_tuple(v[Indices]...);
}

template<typename T, typename C, typename ... Ts>
void shared_state::add_method(const std::string &alias, C *c, T(C::*f)(Ts ...),
                              std::map<long, variant_t> &&def_args) {

//    MethodMeta meta{alias, sizeof...(Ts), !std::is_same<T, void>::value, std::move(def_args),
//                    [f, c](std::vector<variant_t> &params) -> variant_t {
//                        auto args = ref_tuple_gen(params, std::make_index_sequence<sizeof...(Ts)>());
//                        if constexpr (std::is_same<T, void>::value) {
//                            std::apply(f, std::tuple_cat(std::make_tuple(c), args));
//                            return UNDEFINED;
//                        } else {
//                            return std::apply(f, std::tuple_cat(std::make_tuple(c), args));
//                        }
//                    }
//    };
    MethodMeta meta{alias, sizeof...(Ts), !std::is_same<T, void>::value, std::move(def_args),
                    [f, c](std::vector<variant_t> &params) -> arcirk::server::server_command_result {
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
