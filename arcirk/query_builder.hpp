#ifndef ARCIRK_QUERY_BUILDER
#define ARCIRK_QUERY_BUILDER

//#include <iostream>
//#include <nlohmann/json.hpp>
//#include <soci/soci.h>
//#include <soci/sqlite3/soci-sqlite3.h>
//#include <string>
//#include <boost/algorithm/string.hpp>
//#include <boost/format.hpp>
//#include <memory>
//#include <execution>
//#include <boost/uuid/uuid_generators.hpp>
////
////#include <boost/asio.hpp>
////#include <boost/exception/all.hpp>
////#include <boost/uuid/uuid.hpp>
////#include <boost/uuid/uuid_generators.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/uuid/uuid_io.hpp>

#include "database_struct.hpp"

namespace arcirk::database::builder {

    using json = nlohmann::json;
    using namespace soci;

    enum sql_type_of_comparison{
        Equals = 0,
        No_Equals,
        More,
        More_Or_Equal,
        Less_Or_Equal,
        Less,
        On_List,
        Not_In_List
    };

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

    class query_builder{

    public:
        explicit
        query_builder(){
            databaseType = sql_database_type::type_Sqlite3;
            queryType = Select;
        };
        ~query_builder()= default;

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
            result.append("\nwhere ");
            std::vector<std::pair<std::string, json>> where_values;
            auto items_ = values.items();
            for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                where_values.emplace_back(itr.key(), itr.value());
            }
            for (auto itr = where_values.cbegin(); itr != where_values.cend() ; ++itr) {
                result.append(itr->first);
                if(!use_values)
                    result.append("=?");
                else{
                    if(itr->second.is_string())
                        result.append(str_sample("='%1%'", itr->second.get<std::string>()));
                    else if(itr->second.is_number_float())
                        result.append(str_sample("='%1%'", std::to_string(itr->second.get<double>())));
                    else if(itr->second.is_number_integer())
                        result.append(str_sample("='%1%'", std::to_string(itr->second.get<int>())));
                }
                if(itr != (--where_values.cend())){
                    result.append(" AND\n");
                }
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

        [[nodiscard]] std::string prepare(const json& exists = {}, bool use_values = false) const{
            if (exists.empty())
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
                    std::string where_select = q->select({"*"}).from(table_name_).where(exists, use_values).prepare();
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

    private:
        std::string result;
        std::vector<std::pair<std::string, json>> m_list;
        sql_query_type queryType;
        sql_database_type databaseType;
        std::string table_name_;
    };


}

#endif