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
    arcirk::server::server_command_result sync_update_barcode(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result download_file(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_information_about_file(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result check_for_updates(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result upload_file(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_database_tables(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result file_to_database(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result profile_directory_file_list(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result delete_file(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result device_get_full_info(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result get_tasks(const variant_t& param, const variant_t& session_id);
    arcirk::server::server_command_result update_task_options(const variant_t& param, const variant_t& session_id);

    //tasks
    void erase_deleted_mark_objects();
    void synchronize_objects_from_1c();

    void data_synchronization_set_object(const nlohmann::json& object, const std::string& table_name);
    [[nodiscard]] nlohmann::json data_synchronization_get_object(const std::string& table_name, const std::string& ref);

    static std::string execute_random_sql_query(soci::session& sql, const std::string& query_text, bool add_line_number = false, bool add_empty_column = false) ;

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

    void start_tasks();

    std::string save_file(const std::string& content_disp, ByteArray& data) const{

        arcirk::server::server_response resp;
        resp.command = arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile);
        resp.version = ARCIRK_VERSION;

        try {
            T_vec vec = arcirk::split(content_disp, ";");
            std::string file_name;
            std::string destantion;
            for (auto const& itr : vec) {
                T_vec val = arcirk::split(itr, "=");
                if(val.size() == 2){
                    if(val[0] == "file_name"){
                        file_name = val[1];
                        boost::erase_all(file_name, "\"");
                    }else if(val[0] == "destantion"){
                        destantion = val[1];
                        boost::erase_all(destantion, "\"");
                    }
                }
            }
            namespace fs = boost::filesystem;
            fs::path dir(sett.ServerWorkingDirectory);
            dir /= sett.Version;
            fs::path file(dir);
            file /= destantion;
            file /= file_name;

            arcirk::write_file(file.string(), data);

            auto row = nlohmann::json::object();
            row["name"] = file.filename().string();
            row["path"] = file.string().substr(dir.string().length(), file.string().length() - dir.string().length());
            row["is_group"] = 0;
            row["parent"] = file.parent_path().string().substr(dir.string().length(), file.parent_path().string().length() - dir.string().length());
            row["size"] = fs::is_directory(file) ? 0 : (int)fs::file_size(file);


            resp.message = "OK";
            resp.result = arcirk::base64::base64_encode(row.dump());

        } catch (const std::exception &e) {
            resp.message = e.what();
            resp.result = "error";
        }

        return pre::json::to_json(resp).dump();
    }

private:

    soci::session * sql_sess;

    std::shared_ptr<arcirk::services::task_scheduler> task_manager;

    void run_server_tasks();
    void exec_server_task(const arcirk::services::task_options& details);

    subscriber* get_session(const boost::uuids::uuid &uuid);
    std::vector<subscriber *> get_sessions(const boost::uuids::uuid &user_uuid);
    [[nodiscard]] arcirk::database::user_info get_user_info(const boost::uuids::uuid &user_uuid);
    [[nodiscard]] arcirk::database::user_info get_user_info(const std::string &hash);
    static void set_session_info(subscriber* session, const arcirk::database::user_info& info);

    bool is_operation_available(const boost::uuids::uuid &uuid, arcirk::database::roles level);

    [[nodiscard]] boost::filesystem::path sqlite_database_path() const;
    std::string get_channel_token(soci::session& sql, const std::string &first, const std::string &second) const;

    class MethodMeta;

    template<size_t... Indices>
    static auto ref_tuple_gen(std::vector<variant_t> &v, std::index_sequence<Indices...>);

    server::server_config sett;
    std::vector<MethodMeta> methods_meta;

    [[nodiscard]] bool verify_auth(const std::string& usr, const std::string& pwd);
    [[nodiscard]] bool verify_auth_from_hash(const std::string& hash);
    static bool is_cmd(const std::string& message) { return message.substr(0, 3) == "cmd";};
    static bool is_msg(const std::string& message) { return message.substr(0, 3) == "msg";};
    void execute_command_handler(const std::string& message, subscriber *session);
    void forward_message(const std::string& message, subscriber *session);

    bool init_default_result(arcirk::server::server_command_result& result,
                             const boost::uuids::uuid &uuid, server::server_commands cmd,
                             arcirk::database::roles role, nlohmann::json& param, const variant_t& param_);


   soci::session * soci_initialize();
//
//    static void fail(const std::string& what, const std::string& error, bool conv = true, const std::string& log_folder = ""){
//        std::tm tm = arcirk::current_date();
//        char cur_date[100];
//        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);
//
//        std::string res = std::string(cur_date);
//        res.append(" " + what + ": ");
//        if(conv)
//            res.append(arcirk::local_8bit(error));
//        else
//            res.append(error);
//
//        std::cerr << res << std::endl;
//
//        if(log_folder.empty())
//            return;
//
//        namespace fs = boost::filesystem;
//
//        fs::path log_dir(log_folder);
//        log_dir /= "errors";
//        if(!fs::exists(log_dir)){
//            try {
//                fs::create_directories(log_dir);
//            } catch (const std::exception &e) {
//                std::cerr << e.what() << std::endl;
//                return;
//            }
//        }
//        char date_string[100];
//        strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);
//
//        fs::path file = log_dir / (std::string(date_string) + ".log");
//
//        std::ofstream out;			// поток для записи
//        out.open(file.string(), std::ios::app); 		// открываем файл для записи
//        if (out.is_open())
//        {
//            out << res << '\n';
//        }
//        out.close();
//
//        //std::cerr << file << std::endl;
////        if(conv)
////            std::cerr << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(error) << std::endl;
////        else
////            std::cerr << std::string(cur_date) << " " << what << ": " << error << std::endl;
//    };
//
//    static void log(const std::string& what, const std::string& message, bool conv = true, const std::string& log_folder = ""){
//        std::tm tm = arcirk::current_date();
//        char cur_date[100];
//        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);
//
//        std::string res = std::string(cur_date);
//        res.append(" " + what + ": ");
//        if(conv)
//            res.append(arcirk::local_8bit(message));
//        else
//            res.append(message);
//
//        std::cout << res << std::endl;
//
//        if(log_folder.empty())
//            return;
//
//        namespace fs = boost::filesystem;
//
//        fs::path log_dir(log_folder);
//        log_dir /= "days";
//        if(!fs::exists(log_dir)){
//            try {
//                fs::create_directories(log_dir);
//            } catch (const std::exception &e) {
//                std::cerr << e.what() << std::endl;
//                return;
//            }
//        }
//        char date_string[100];
//        strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);
//
//        fs::path file = log_dir / (std::string(date_string) + ".log");
//        //std::cout << file << std::endl;
//
//        std::ofstream out;			// поток для записи
//        out.open(file.string(), std::ios::app); 		// открываем файл для записи
//        if (out.is_open())
//        {
//            out << res  << '\n';
//        }
//        out.close();
//
////        if(conv)
////            std::cout << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(message) << std::endl;
////        else
////            std::cout << std::string(cur_date) << " " << what << ": " <<message << std::endl;
//    };

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

namespace arcirk::_1c::scripts{

            enum local_1c_script{
                barcode_information,
                SCRIPT1C_INVALID=-1,
            };

            static std::string get_text(local_1c_script type, server::server_config& sett){

                using namespace boost::filesystem;
                path scripts_(sett.ServerWorkingDirectory);
                scripts_ /= sett.Version;
                if(type == barcode_information){
                    scripts_ /= "1c/bsl/get_barcode_information.bsl";
                }
                if(!exists(scripts_)){
                    return {};
                }
                std::ifstream in(scripts_.string());
                std::stringstream ss;
                ss << in.rdbuf();
                in.close();
                return ss.str();

            }

        }
#endif //BWEBSOCKETS_SHARED_STATE_HPP
