#ifndef ARCIRK_QUERY_BUILDER
#define ARCIRK_QUERY_BUILDER

#include "database_struct.hpp"
#include <map>
#include <functional>
#include <exception>

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

    enum sql_order_type{
        dbASC = 0,
        dbDESC
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(sql_order_type, {
        {dbASC, "asc"},
        {dbDESC, "desc"}
    });

    enum sql_join_type{
        joinCross = 0,
        joinLeft,
        joinRight,
        joinInner,
        joinFull
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(sql_join_type, {
        {joinCross, "cross"},
        {joinLeft, "left"},
        {joinRight, "right"},
        {joinInner, "inner"},
        {joinFull, "full"}
    });

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

    typedef struct sql_compare_value{

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

    }sql_compare_value;

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

        query_builder& join(const std::string& table_name, const json& fields, std::string, sql_join_type join_type, const json& join_options = {}){

            if(table_name_.empty())
                throw std::exception("Используйте 'from' сначала для первой таблицы.");

            std::string table_first_alias = table_name_ + "First";
            std::string table_second_alias = table_name + "Second";

            if(fields.is_object()){
                auto items_ = fields.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    result.append(table_name + "." + itr.key() + " as " + table_second_alias + "_" + itr.key());
                    result.append(",\n");
                }
            }else if(fields.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    std::string key = *itr;
                    result.append(table_name + "." + key + " as " + table_second_alias + "_" +  key);
                    result.append(",\n");
                }
            }

            if(m_list.empty()){
                result = "select " + table_name_ + ".*";
            }else{
                result = "select ";
                for (auto itr = m_list.begin(); itr != m_list.end() ; ++itr) {
                    result.append(table_name_ + "." + itr->first + " as " + table_first_alias + "_" + itr->first);
                    if(itr != --m_list.end())
                        result.append(",\n");
                }
            }

            result.append("\nfrom " + table_name_ + " as " + table_first_alias + " ");
            result.append(enum_synonym(join_type));
            result.append(" join " + table_name + " as " + table_second_alias);

            if(join_type != sql_join_type::joinCross && join_options.empty()){
                throw std::exception("Не заданы параметры соединения таблиц!");
            }

            std::vector<std::pair<std::string, std::string>> inner_join;
            if(join_options.is_object()){
                auto items = join_options.items();
                for (auto itr = items.begin(); itr != items.end(); ++itr) {
                    inner_join.emplace_back(itr.key(), itr.value().get<std::string>());
                }
            }else if(join_options.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    std::string key = *itr;
                    inner_join.emplace_back(key, key);
                }
            }else
                throw std::exception("Не верный тип массива параметров соединения!");

            result.append(" on ");
            for (auto itr = inner_join.begin(); itr != inner_join.end(); ++itr) {
                result.append(table_name_ + "." + itr->first + "=" + table_name + "." + itr->second);
                if(itr != --inner_join.end()){
                    result.append("\nand ");
                }
            }

            return *this;
        }

        query_builder& from(const std::string& table_name){
            table_name_ = table_name;
            result.append("\nfrom ");
            result.append(table_name);
            return *this;
        }

        query_builder& where(const json& values, bool use_values, bool use_table_name = true){

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
                if(use_table_name)
                    result.append(table_name_ + ".");
                result.append(itr->to_string(use_values));
                if(itr != --where_values.end())
                    result.append("\nand\n");
            }
            return *this;
        }

        query_builder& with_temp_table(){
           result.insert(0, "with temp_table as(");
           result.append(")");
           return *this;
        }

        query_builder& join_temp_table(sql_join_type join_type){
            result.append(enum_synonym(join_type));
            result.append(" join temp_table");
            return *this;
        }

        query_builder& where_join(const json& values, const std::string& join_table_name, bool use_values){
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
            result.append("\nand ");
            for (auto itr = where_values.begin(); itr != where_values.end() ; ++itr) {
                result.append(join_table_name + ".");
                result.append(itr->to_string(use_values));
                if(itr != --where_values.end())
                    result.append("\nand\n");
            }
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

        std::string ref() const{
            for (auto itr = m_list.cbegin(); itr != m_list.cend(); ++itr) {
                if(itr->first == "ref")
                    return itr->second;
            }
            return {};
        }

        //0=asc, 1=desc
        //fields = {{"field1", 0}, {"field2", 1}}
        query_builder& order_by(const json& fields){

            result +="\norder by ";
            std::vector<std::pair<std::string, sql_order_type>> m_order_list;
            if(fields.is_object()){
                auto items_ = fields.items();
                for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                    if(itr.value().is_number_integer()){
                        int order = itr.value().get<int>();
                        auto t = sql_order_type::dbASC;
                        if(order <= 1 && order > 0)
                            t = (sql_order_type)order;
                        m_order_list.emplace_back(itr.key(), t);
                    }else
                        m_order_list.emplace_back(itr.key(), sql_order_type::dbASC);
                }
            }else if(fields.is_array()){
                for (auto itr = fields.begin(); itr != fields.end(); ++itr) {
                    m_order_list.emplace_back(*itr, sql_order_type::dbASC);
                }
            }

            for (auto itr = m_order_list.cbegin(); itr != m_order_list.cend() ; ++itr) {
                result.append(itr->first);
                result.append(" ");
                result.append(enum_synonym(itr->second));
                if(itr != (--m_order_list.cend())){
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
                if(itr->first == "_id")
                    continue;
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
                if(itr->first == "_id")
                    continue;
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
            std::cout << q << std::endl;
            return (sql.prepare << q);
        }

        void execute(soci::session& sql, const json& exists = {}, bool use_values = false) const{
            std::string q = prepare(exists, use_values);
            soci::statement st = (sql.prepare << q);
            st.execute(true);
        }

        template<typename T>
        static T get_value(soci::row const& row, const std::size_t& column_index){
            //не знаю как правильно проверить на null поэтому вот так ...
            try {
                return row.get<T>(column_index);
            }catch (...){
                return {};
            }
        }

        static void execute(const std::string& query_text, soci::session& sql, json& result_table, const std::vector<std::string>& column_ignore = {}){

            soci::rowset<soci::row> rs = (sql.prepare << query_text);

            std::cout << query_text << std::endl;

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
                        case dt_string:{
                                auto val = get_value<std::string>(row, i);
                                j_row += {column_name, val};
                            }
                            break;
                        case dt_double:{
                                auto val = get_value<double>(row, i);
                                j_row += {column_name, val};
                            }
                            break;
                        case dt_integer:{
                                auto val = get_value<int>(row, i);
                                j_row += {column_name, val};
                            }
                            break;
                        case dt_long_long:{
                                auto val = get_value<long long>(row, i);
                                j_row += {column_name, val};
                            }
                            break;
                        case dt_unsigned_long_long:{
                                auto val = get_value<unsigned long long>(row, i);
                                j_row += {column_name, val};
                            }
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

        bool is_valid(){
            return !result.empty();
        }

        void clear(){
            result = "";
            m_list.clear();
            table_name_ = "";
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