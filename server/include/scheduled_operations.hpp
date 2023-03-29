#ifndef SCHEDULED_OPERATIONS_HPP
#define SCHEDULED_OPERATIONS_HPP

#include <arcirk.hpp>
#include <shared_struct.hpp>
#include <database_struct.hpp>

class scheduled_operations{

public:
    explicit
    scheduled_operations(const arcirk::server::server_config& sett);

    bool perform_data_exchange();
    nlohmann::json exec_http_query(const std::string& command, const nlohmann::json& param);

private:
    arcirk::server::server_config sett_;

    template<typename T>
    void add_query(const nlohmann::json& object, std::vector<std::string>& transaction_arr, soci::session& sql, const std::string& table_name);

    template<typename T>
    void add_information_register_record(const nlohmann::json& object, std::vector<std::string>& transaction_arr, soci::session& sql, const std::string& table_name);

    template<typename T>
    T get_string_value(const nlohmann::json& object, const std::string& key) const;

    static bool field_is_exists(const nlohmann::json &object, const std::string& name);

    static void fail(const std::string& what, const std::string& error, bool conv = true){
        std::tm tm = arcirk::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        if(conv)
            std::cerr << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(error) << std::endl;
        else
            std::cerr << std::string(cur_date) << " " << what << ": " << error << std::endl;
    };

    soci::session soci_initialize() const{
        using namespace boost::filesystem;
        using namespace soci;

        auto result = arcirk::database::user_info();

        if(sett_.SQLFormat == arcirk::DatabaseType::dbTypeSQLite){
            if(sett_.ServerWorkingDirectory.empty())
            {
                throw native_exception("Ошибки в параметрах сервера!");
            }

            path database(sett_.ServerWorkingDirectory);
            database /= sett_.Version;
            database /= "data";
            database /= "arcirk.sqlite";

            if(!exists(database)){
                throw native_exception("Файл базы данных не найден!");
            }

            try {
                std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
                return session{soci::sqlite3, connection_string};
            } catch (native_exception &e) {
                fail("shared_state::soci_initialize:error", e.what(), false);
            }
        }
        return {};
    }
};

#endif