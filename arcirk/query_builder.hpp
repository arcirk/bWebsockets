#ifndef ARCIRK_QUERY_BUILDER
#define ARCIRK_QUERY_BUILDER

#include "database_struct.hpp"
#include <map>
#include <functional>

namespace arcirk::database::builder {

    using json = nlohmann::json;
    using namespace soci;

    //typedef std::function<void(const std::string& /*result*/,const std::string& /*uuid_session*/)> callback_query_result;

    enum sql_query_type{
        Insert = 0,
        Update,
        Delete,
        Select
    };

    enum sql_database_type{
        type_Sqlite3 = 0,
        type_ODBC
    };

    enum sql_type_of_comparison{
        Equals,
        Not_Equals,
        More,
        More_Or_Equal,
        Less_Or_Equal,
        Less,
        On_List,
        Not_In_List
    };

    static std::map<sql_type_of_comparison, std::string> sql_compare_template = {
            std::pair<sql_type_of_comparison, std::string>(Equals, "%1%=%2%"),
            std::pair<sql_type_of_comparison, std::string>(Not_Equals, "not %1%=%2%"),
            std::pair<sql_type_of_comparison, std::string>(More, "%1%>%2%"),
            std::pair<sql_type_of_comparison, std::string>(More_Or_Equal, "%1%>=%2%"),
            std::pair<sql_type_of_comparison, std::string>(Less_Or_Equal, "%1%<=%2%"),
            std::pair<sql_type_of_comparison, std::string>(Less, "%1%<%2%"),
            std::pair<sql_type_of_comparison, std::string>(On_List, "%1% in (%2%)"),
            std::pair<sql_type_of_comparison, std::string>(Not_In_List, "not %1% in (%2%)")
    };

    struct sql_compare_value{

        std::string key;
        sql_type_of_comparison compare;
        json value;

        sql_compare_value() {
            compare = Equals;
        };
        explicit sql_compare_value(const json& object){
            key = object.value("key", "");
            compare = object.value("compare", Equals);
            value = object.value("value", json{});
        }
        explicit sql_compare_value(const std::string& key_, const json& value_, sql_type_of_comparison compare_ = Equals){
            key = key_;
            compare = compare_;
            value = value_;
        }

        template<class T>
        T get() {
            return sql_compare_value::get<T>(value);
        }

        template<class T>
        static T get(const nlohmann::json& json_object) {
            T object;
            pre::json::detail::dejsonizer dejsonizer(json_object);
            dejsonizer(object);
            return object;
        }

        static std::string array_to_string(const nlohmann::json& json_object, bool quotation_marks = true){

            if(!json_object.is_array())
                return {};

            std::string result;
            int i = 0;
            for (const auto& value : json_object) {
                if(quotation_marks)
                    result.append("'");
                if(value.is_string())
                    result.append(get<std::string>(value));
                if(quotation_marks)
                    result.append("'");
                i++;
                if(i != json_object.size())
                    result.append(",");
            }

            return result;
        }

        [[nodiscard]] std::string to_string(bool quotation_marks = true) const{
            std::string template_value = sql_compare_template[compare];
            std::string result;
            if(!value.is_array()){
                std::string s_val = quotation_marks ? "'%1%'" : "%1%";
                std::string v;
                if(value.is_number_integer()){
                    v = std::to_string(get<int>(value));
                }else if(value.is_number_float()){
                    v = std::to_string(get<double>(value));
                }else if(value.is_string()){
                    v = get<std::string>(value);
                }
                result.append(str_sample(template_value, key, str_sample(s_val, v)));
            }else{
                result.append(str_sample(template_value, key, array_to_string(value, quotation_marks)));
            }
            return result;
        }

        [[nodiscard]] json to_object() const{
            json result = {
                    {"key", key},
                    {"compare", compare},
                    {"value", value}
            };
            return result;
        }

    };

    class sql_where_values{
    public:
        sql_where_values() {m_list = {};};
        void add(const std::string& key, const json& value, sql_type_of_comparison compare = Equals){
            m_list.push_back(std::make_shared<sql_compare_value>(key, value, compare));
        }
        void clear(){
            m_list.clear();
        }

        [[nodiscard]] json to_json_object() const{
            json result = {};
            if(!m_list.empty()){
                for (const auto& v: m_list) {
                    result.push_back(v->to_object());
                }
            }
            return result;
        }

    private:
        std::vector<std::shared_ptr<sql_compare_value>> m_list;
    };

    class query_builder{

    public:

        explicit
        query_builder(){
            databaseType = sql_database_type::type_Sqlite3;
            queryType = Select;
        };
        ~query_builder()= default;

        static bool is_select_query(const std::string& query_text){

            std::string tmp(query_text);
            trim(tmp);
            to_lower(tmp);
            if(tmp.length() < 6)
                return false;
            return tmp.substr(6) == "select";

        }

        query_builder& select(const json& fields) {
            use(fields);
            return select();
        }

        query_builder& from(const std::string& table_name){
            table_name_ = table_name;
            result.append("\nfrom ");
            result.append(table_name);
            return *this;
        }

        query_builder& where(const json& values, bool use_values){

            std::vector<sql_compare_value> where_values;
            auto items_ = values.items();
            for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                sql_compare_value val{};
                val.key = itr.key();
                const json& f_value = itr.value();
                if(!f_value.is_object()){
                    val.compare = Equals;
                    if(use_values)
                        val.value = f_value;
                    else
                        val.value = "?";
                }else{
                    val.compare = (sql_type_of_comparison)f_value.value("compare", 0);
                    if(use_values)
                        val.value = f_value.value("value", json{});
                    else
                        val.value = "?";
                }
                where_values.push_back(val);
            }

            result.append("\nwhere ");

            for (auto itr = where_values.begin(); itr != where_values.end() ; ++itr) {
                result.append(itr->to_string(use_values));
                if(itr != --where_values.end())
                    result.append("\nAND\n");
            }
            return *this;
        }

        void use(const json& source){
            m_list.clear();
            if(source.is_object()){
                auto items_ = source.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    m_list.emplace_back(itr.key(), itr.value());
                }
            }else if(source.is_array()){
                for (auto itr = source.begin(); itr != source.end(); ++itr) {
                    m_list.emplace_back(*itr, "");
                }
            }

        }

        query_builder& select(){
            queryType = Select;
            result = "select ";
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                result.append(itr->first);
                if(itr != (--m_list.cend())){
                    result.append(",\n");
                }
            }
            return *this;
        }

        query_builder& update(const std::string& table_name, bool use_values){
            queryType = Update;
            table_name_ = table_name;
            result = str_sample("update %1% set ", table_name);
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                result.append(itr->first);
                if(use_values){
                    if(itr->second.is_string())
                        result.append(str_sample("='%1%'", itr->second.get<std::string>()));
                    else if(itr->second.is_number_float())
                        result.append(str_sample("='%1%'", std::to_string(itr->second.get<double>())));
                    else if(itr->second.is_number_integer())
                        result.append(str_sample("='%1%'", std::to_string(itr->second.get<int>())));
                }else
                    result.append("=?");
                if(itr != (--m_list.cend())){
                    result.append(",\n");
                }
            }

            return *this;
        }

        query_builder& insert(const std::string& table_name, bool use_values){
            queryType = Insert;
            table_name_ = table_name;
            result = str_sample("insert into %1% (", table_name);
            std::string string_values;
            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                result.append(itr->first);
                if(use_values){
                    if(itr->second.is_string())
                        string_values.append(str_sample("'%1%'", itr->second.get<std::string>()));
                    else if(itr->second.is_number_float())
                        string_values.append(str_sample("'%1%'", std::to_string(itr->second.get<double>())));
                    else if(itr->second.is_number_integer())
                        string_values.append(str_sample("'%1%'", std::to_string(itr->second.get<int>())));
                }else
                    string_values.append("?");

                if(itr != (--m_list.cend())){
                    result.append(",\n");
                    string_values.append(",\n");
                }
            }
            result.append(")\n");
            result.append("values(");
            result.append(string_values);
            result.append(")");

            return *this;
        }

        [[nodiscard]] std::string prepare(const json& where_not_exists = {}, bool use_values = false) const{
            if (where_not_exists.empty())
                return result;
            else{
                using namespace boost::algorithm;
                bool odbc = databaseType == type_ODBC;
                if(queryType == Insert){
                    std::string query_sample;
                    if (odbc) {
                        query_sample = "IF NOT EXISTS\n"
                                       "(%1%)\n"
                                       "BEGIN\n"
                                       "%2%\n"
                                       "END";
                    } else {
                        query_sample = "%2% \n"
                                       "WHERE NOT EXISTS (%1%)";
                    }
                    std::string tmp;
                    if(databaseType == type_Sqlite3){
                        tmp = replace_first_copy(result, "values(", "select ");
                        if(tmp[tmp.length() -1] == ')')
                            tmp = tmp.substr(0, tmp.length()-1);
                    }
                    else
                        tmp = result;
                    auto q = std::make_shared<query_builder>();
                    std::string where_select = q->select({"*"}).from(table_name_).where(where_not_exists, use_values).prepare();
                    return str_sample(query_sample, where_select, tmp);
                }
            }

            return result;
        }

        sql_database_type database_type(){
            return databaseType;
        }

        void set_databaseType(sql_database_type dbType){
            databaseType = dbType;
        }

        soci::rowset<soci::row> exec(soci::session& sql, const json& exists = {}, bool use_values = false) const{
            std::string q = prepare(exists, use_values);
            //std::cout << q << std::endl;
            return (sql.prepare << q);
        }

        void execute(soci::session& sql, const json& exists = {}, bool use_values = false) const{
            std::string q = prepare(exists, use_values);
            soci::statement st = (sql.prepare << q);
            st.execute(true);
        }

        static void execute(const std::string& query_text, soci::session& sql, json& result_table, const std::vector<std::string>& column_ignore = {}){

            soci::rowset<soci::row> rs = (sql.prepare << query_text);

            json columns = {"line_number"};
            json roms = {};
            int line_number = 0;

            for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
            {
                line_number++;
                row const& row = *it;
                json j_row = {{"line_number", line_number}};

                for(std::size_t i = 0; i != row.size(); ++i)
                {
                    const column_properties & props = row.get_properties(i);
                    std::string column_name = props.get_name();

                    if((columns.size() -1) != row.size()){
                        columns.push_back(column_name);
                    }

                    if(std::find(column_ignore.begin(), column_ignore.end(), column_name) != column_ignore.end()){
                        j_row += {column_name, ""};
                        continue;
                    }

                    switch(props.get_data_type())
                    {
                        case dt_string:
                            j_row += {column_name, row.get<std::string>(i)} ;
                            break;
                        case dt_double:
                            j_row += {column_name, row.get<double>(i)} ;
                            break;
                        case dt_integer:
                            j_row += {column_name, row.get<int>(i)} ;
                            break;
                        case dt_long_long:
                            j_row += {column_name, row.get<long long>(i)} ;
                            break;
                        case dt_unsigned_long_long:
                            j_row += {column_name, row.get<unsigned long long>(i)} ;
                            break;
                        case dt_date:
                            //std::tm when = r.get<std::tm>(i);
                            break;
                        case dt_blob:
                            break;
                        case dt_xml:
                            break;
                    }

                }

                roms += j_row;
            }

            result_table = {
                    {"columns", columns},
                    {"rows", roms}
            };

        }

    private:
        std::string result;
        std::vector<std::pair<std::string, json>> m_list;
        sql_query_type queryType;
        sql_database_type databaseType;
        std::string table_name_;
    };


}

#endif