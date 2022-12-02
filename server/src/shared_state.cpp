
#include <utility>
#include "../include/shared_state.hpp"
#include "../include/websocket_session.hpp"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <algorithm>
#include <locale>

#include <query_builder.hpp>

shared_state::shared_state(){

    using namespace arcirk::server;

    sett = server::server_config();
    read_conf(sett, app_directory(), ARCIRK_SERVER_CONF);
    add_method(enum_synonym(server::server_commands::ServerVersion), this, &shared_state::server_version);
    add_method(enum_synonym(server::server_commands::ServerOnlineClientsList), this, &shared_state::get_clients_list);
    add_method(enum_synonym(server::server_commands::SetClientParam), this, &shared_state::set_client_param);
    add_method(enum_synonym(server::server_commands::ServerConfiguration), this, &shared_state::server_configuration);
    add_method(enum_synonym(server::server_commands::UserInfo), this, &shared_state::user_information);
    add_method(enum_synonym(server::server_commands::InsertOrUpdateUser), this, &shared_state::insert_or_update_user);
    add_method(enum_synonym(server::server_commands::CommandToClient), this, &shared_state::command_to_client);
    add_method(enum_synonym(server::server_commands::ServerUsersList), this, &shared_state::get_users_list);
    add_method(enum_synonym(server::server_commands::ExecuteSqlQuery), this, &shared_state::execute_sql_query);
    add_method(enum_synonym(server::server_commands::GetMessages), this, &shared_state::get_messages);

}

void shared_state::join(subscriber *session) {

    sessions_.insert(std::pair<boost::uuids::uuid, subscriber*>(session->uuid_session(), session));
    log("shared_state::join", "client join: " + arcirk::uuids::uuid_to_string(session->uuid_session()) + " " + session->address());

    //Оповещаем всех пользователей об подключении нового клиента
    if(use_authorization())
        if(session->authorized())
            send_notify("Client Join", session, "ClientJoin");
    else
        send_notify("Client Join", session, "ClientJoin");
}

void shared_state::send_notify(const std::string &message, subscriber *sender, const std::string& notify_command, const boost::uuids::uuid& sender_uuid) {

    server::server_response resp;
    resp.command = notify_command;
    resp.message = message;
    resp.result = "OK";
    if(sender){
        resp.sender = arcirk::uuids::uuid_to_string(sender->uuid_session());
        resp.app_name = sender->app_name();
    }else{
        if(sender_uuid != boost::uuids::nil_uuid())
            resp.sender = arcirk::uuids::uuid_to_string(sender_uuid);
    }
    std::string response =  pre::json::to_json(resp).dump();

    if(sender){
        if(sender->is_ssl())
            send<ssl_websocket_session>(response, sender);
        else
            send<plain_websocket_session>(response, sender);
    }else{
        send<ssl_websocket_session>(response);
        send<plain_websocket_session>(response);
    }

}

void shared_state::leave(const boost::uuids::uuid& session_uuid, const std::string& user_name) {
    auto iter = sessions_.find(session_uuid);
    if (iter != sessions_.end() ){
        sessions_.erase(session_uuid);
    }
    log("shared_state::leave", "client leave: " + user_name + " (" + arcirk::uuids::uuid_to_string(session_uuid) + ")" );

    //Оповещаем всех пользователей об отключении клиента
    send_notify("Client Leave", nullptr, "ClientLeave", session_uuid);
}

void shared_state::deliver(const std::string &message, subscriber *session) {

    std::string result = message;

    if (result == "\n")
        return;

    if (result == "ping")
        result = "pong";

    if(use_authorization()){
        if(!session->authorized() && message.find("SetClientParam") == std::string::npos)
            return fail("shared_state::deliver", "Пользователь не авторизован! Команда отменена.");
    }

    if(!is_cmd(message)){
        if(!is_msg(message)){
            if(!session->is_ssl())
                send<plain_websocket_session>(result);
            else
                send<ssl_websocket_session>(result);
        }else
            forward_message(message, session);
    }else{
        execute_command_handler(message, session);
    }
}

void shared_state::forward_message(const std::string &message, subscriber *session) {

    arcirk::T_vec v = split(message, " ");
    if(v.size() < 3)
    {
        fail("shared_state::forward_message:error", "Не верный формат сообщения!");
        return;
    }

    std::string receiver = v[1];
    std::string msg = v[2];
    std::string param;
    if(v.size() == 4){
        param = v[3];
    }

    server::server_response resp;
    resp.command = "message";
    resp.message = msg; //base64
    resp.param = param; //base64
    resp.result = "OK";
    resp.sender = arcirk::uuids::uuid_to_string(session->uuid_session());
    resp.receiver = receiver;
    resp.app_name = session->app_name();

    std::string response =  pre::json::to_json(resp).dump();

    boost::uuids::uuid receiver_{};
    if(!uuids::is_valid_uuid(receiver, receiver_)){
        fail("shared_state::forward_message:error", "Не верный идентификатор получателя!");
        return;
    }

    const auto itr = sessions_.find(receiver_);
    if(itr == sessions_.cend()){
        fail("shared_state::forward_message:error", "Не известный получатель!") ;
        return;
    }

    if(sett.AllowHistoryMessages){
        std::string content_type_ = arcirk::enum_synonym(database::text_type::dbText);
        if(!param.empty()){
            auto param_ = nlohmann::json::parse(arcirk::base64::base64_encode(param));
            content_type_ = param_.value("content_type", content_type_);
        }else
            log("shared_state::forward_message", "Не указан тип сообщения, будет установлен по умолчанию 'Text'");

        auto msg_struct = database::messages();
        msg_struct.ref = boost::to_string(uuids::random_uuid());
        msg_struct.first = resp.sender;
        msg_struct.second = receiver;
        msg_struct.message = msg;
        msg_struct.content_type = content_type_;
        msg_struct.date = (int)arcirk::current_date_seconds();
        try {
            auto sql = soci_initialize();
            msg_struct.token  = get_channel_token(sql, msg_struct.first, msg_struct.second);
            auto query = std::make_shared<database::builder::query_builder>();
            query->use(pre::json::to_json(msg_struct));
            query->insert("Messages", true);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    if(sett.ResponseTransferToBase64){
        auto const ss = boost::make_shared<std::string const>(base64::base64_encode(response));
        itr->second->send(ss);
    }else{
        auto const ss = boost::make_shared<std::string const>(response);
        itr->second->send(ss);
    }

}

std::string shared_state::get_channel_token(soci::session& sql, const std::string &first, const std::string &second) const{

    using namespace arcirk::database::builder;
    using namespace soci;

    auto builder = std::make_shared<query_builder>();
    std::vector<std::string> refs;

    try {
        nlohmann::json ref = {
                {"ref", sql_compare_value("ref", {
                        first,
                        second}, sql_type_of_comparison::On_List).to_object()}
        };
        auto result = builder->select({"_id","ref"}).from("Users").where(ref, true).order_by({"_id"}).exec(sql,{}, true);


        for (rowset<row>::const_iterator itr = result.begin(); itr != result.end(); ++itr) {
            row const& row = *itr;
            refs.push_back(row.get<std::string>(1));
        }

    } catch (std::exception &e) {
        fail("shared_state::get_channel_token", e.what());
        return "error";
    }

    if (refs.size() <= 1){//минимум 2 записи должно быть
        fail("shared_state::get_channel_token", "Ошибка генерации токена!");
        return "error";
    }

    std::string hash = arcirk::get_hash(refs[0], refs[1]);

    return hash;
}

void shared_state::execute_command_handler(const std::string& message, subscriber *session) {

    arcirk::T_vec v = split(message, " ");

    if(v.size() < 2){
        fail("shared_state::execute_command_handler:error", "Не верный формат команды!");
        return;
    }

    long command_index = find_method(v[1]);
    if(command_index < 0){
        fail("shared_state::execute_command_handler:error", arcirk::str_sample("Команда (%1%) не найдена!", v[1]));
        return;
    }

    int param_index = (int)v.size() - 1;

    std::string json_params;
    if(v.size() > 2){
        try {
            json_params = arcirk::base64::base64_decode(v[param_index]);
        } catch (std::exception &e) {
            fail("shared_state::execute_command_handler:parse_params:error", e.what(), false);
            return;
        }
    }

    std::string session_id_receiver; //указан получатель
    if(param_index == 3)
        session_id_receiver = v[2];

    std::vector<variant_t> params_v;
    if(!json_params.empty()){
        using nlohmann_json = nlohmann::json;
        auto params_ = nlohmann_json::parse(json_params);
        for (auto& el : params_.items()) {
            auto value = el.value();
            if(value.is_string()){
                params_v.emplace_back(value.get<std::string>());
            }else if(value.is_number()){
                params_v.emplace_back(value.get<double>());
            }else if(value.is_boolean()){
                params_v.emplace_back(value.get<bool>());
            }
        }
    }

    params_v.emplace_back(arcirk::uuids::uuid_to_string(session->uuid_session()));
    if(!session_id_receiver.empty())
        params_v.emplace_back(session_id_receiver); //идентификатор получателя

    long p_count = param_count(command_index);
    if(p_count != params_v.size())
        return fail("shared_state::execute_command_handler", "Не верное количество аргументов!");

    log("shared_state::execute_command_handler", get_method_name(command_index));

    arcirk::server::server_command_result return_value;
    try {
        call_as_func(command_index, &return_value, params_v);
    }
    catch (const server_commands_exception& ex){
        return_value.result = "error";
        return_value.uuid_form = ex.uuid_form();
        return_value.message = ex.what();
        fail("shared_state::execute_command_handler::" + std::string(ex.command()), ex.what());
    }
    catch (const std::exception& ex) {
        return_value.result = "error";
        return_value.uuid_form = arcirk::uuids::nil_string_uuid();
        return_value.message = ex.what();
        fail("shared_state::execute_command_handler", ex.what());
    }


    bool is_command_to_client = get_method_name(command_index) == enum_synonym(arcirk::server::server_commands::CommandToClient);

    using namespace arcirk::server;
    std::string response;
    server::server_response result_response;
    result_response.command = get_method_name(command_index);
    result_response.message = return_value.message.empty() ? return_value.result != "error" ? "OK" : "error" : return_value.message;
    result_response.result = is_command_to_client ? "" : return_value.result; //base64 string
    result_response.sender = arcirk::uuids::uuid_to_string(session->uuid_session());
    if(!is_command_to_client){
        result_response.receiver = arcirk::uuids::uuid_to_string(session->uuid_session());
        result_response.uuid_form = return_value.uuid_form;
        result_response.param = return_value.result;
    }else
        result_response.receiver = session_id_receiver;

    result_response.app_name = session->app_name();

    response = pre::json::to_json(result_response).dump();

    if(sett.ResponseTransferToBase64){
        response = arcirk::base64::base64_encode(response);
    }

    auto const ss = boost::make_shared<std::string const>(response);
    if(!is_command_to_client){
        session->send(ss);
    }else{
        auto session_receiver = get_session(arcirk::uuids::string_to_uuid(session_id_receiver));
        if(session_receiver)
            session_receiver->send(ss);
    }

}

bool shared_state::use_authorization() const{
    return sett.UseAuthorization;
}

template<typename T>
void shared_state::send(const std::string &message, subscriber* skip_session) {

    bool ssl_ = typeid(T) == typeid(ssl_websocket_session);

    std::vector<std::weak_ptr<T>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_){
            if(ssl_ != p.second->is_ssl())
                continue;
            if(p.second != skip_session)
                v.emplace_back(p.second->template derived<T>().weak_from_this());
        }

    }
    for(auto const& wp : v)
        if(auto sp = wp.lock()){
            auto const ss = boost::make_shared<std::string const>(message);
            sp->send(ss);
        }

}

bool shared_state::verify_connection(const std::string &basic_auth) {

    if(basic_auth.empty())
        return false;
    else{
        arcirk::T_vec v = split(basic_auth, " ");
        try {
            if(v.size() == 2){
                const std::string base64 = v[1];
                std::string auth = arcirk::base64::base64_decode(base64);
                arcirk::T_vec m_auth = split(auth, ":");
                if(m_auth.size() != 2)
                    return false;
                return verify_auth(m_auth[0], m_auth[1]);
            }
        } catch (std::exception &e) {
            fail("shared_state::verify_connection:error", e.what(), false);
        }
    }

    return false;
}
bool shared_state::verify_auth_from_hash(const std::string &usr, const std::string &hash, std::string& user_uuid) const {

    log("shared_state::verify_auth_from_hash", "verify_connection ... ");

    using namespace boost::filesystem;
    using namespace soci;

    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
        if(sett.ServerWorkingDirectory.empty())
        {
            fail("shared_state::verify_auth:error", "Ошибки в параметрах сервера!");
            return false;
        }

        path database(sett.ServerWorkingDirectory);
        database /= sett.Version;
        database /= "data";
        database /= "arcirk.sqlite";

        if(!exists(database)){
            fail("shared_state::verify_auth:error", "Файл базы данных не найден!");
            return false;
        }

        try {
            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
            session sql(soci::sqlite3, connection_string);
            int count = -1;
//            sql << "select count(*) from Users where hash = " <<  "'" << hash << "'" , into(count);
//            return count > 0;
            auto builder = std::make_shared<database::builder::query_builder>();
            auto rs = builder->select({"user_"})
        } catch (std::exception &e) {
            fail("shared_state::verify_auth:error", e.what(), false);
        }

    }
    return false;
}

bool shared_state::verify_auth(const std::string& usr, const std::string& pwd) const {

    using namespace boost::filesystem;
    using namespace soci;

    std::string hash = arcirk::get_hash(usr, pwd);
    return verify_auth_from_hash(usr, hash);

}

auto shared_state::parse_json(const std::string &json_text, bool is_base64) {

    using n_json = nlohmann::json;
    std::string json_param;

    if(json_text.empty())
        throw std::exception("Не верный формат json!");

    if(is_base64)
        json_param = arcirk::base64::base64_decode(json_text);
    else
        json_param = json_text;

    auto param_ = n_json::parse(json_param, nullptr, false);
    if(param_.is_discarded()){
        throw std::exception("Не верный формат json!");
    }

    return param_;

}

arcirk::server::server_command_result shared_state::command_to_client(const variant_t &param,
                                                                      const variant_t &session_id,
                                                                      const variant_t &session_id_receiver) {

    auto session_receiver = get_session(arcirk::uuids::string_to_uuid(std::get<std::string>(session_id_receiver)));
    if(!session_receiver)
        throw std::exception("Сессия получателя не найдена!");

    //ToDo: Парсинг параметров если необходимо

    //
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::CommandToClient);
    result.result = std::get<std::string>(param);

    return result;
}

arcirk::server::server_command_result shared_state::get_users_list(const variant_t &param,
                                                                   const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace arcirk::server;
    using n_json = nlohmann::json;
    using namespace boost::filesystem;
    using namespace soci;

    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ServerUsersList);

    try {
        auto param_ = parse_json(std::get<std::string>(param), true);
        result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

        boost::uuids::uuid uuid = arcirk::uuids::string_to_uuid(std::get<std::string>(session_id));
        bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
        if (!operation_available)
            throw server_commands_exception("Не достаточно прав доступа!", result.command, result.uuid_form);

        auto table_object = n_json::object();

        path db_path = sqlite_database_path();
        std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", db_path.string());
        session sql(soci::sqlite3, connection_string);
        soci::rowset<arcirk::database::user_info> rs = (sql.prepare << "select * from Users");
        //int count = -1;
        auto rows = n_json::array();
        for (auto it = rs.begin(); it != rs.end(); it++) {
            arcirk::database::user_info user_info_ = *it;
            //count++;
            user_info_.hash = ""; //хеш не показываем
            auto row = pre::json::to_json(user_info_);
            rows += row;
        }
        if(param_.value("table", false)) {
            auto columns = n_json::array();
            auto row_struct = pre::json::to_json(user_info());
            for (const auto& element : row_struct.items()) {
                columns += element.key();
            }
            table_object["columns"] = columns;
        }
        table_object["rows"] = rows;
        std::string table = arcirk::base64::base64_encode(table_object.dump());
        result.result = table;

    } catch (std::exception &ex) {
        throw server_commands_exception(ex.what(), result.command, result.uuid_form);
    }

    return result;

}

arcirk::server::server_command_result shared_state::get_clients_list(const variant_t& param, const variant_t& session_id){

    using namespace arcirk::database;
    using namespace arcirk::server;
    using n_json = nlohmann::json;

    boost::uuids::uuid uuid = arcirk::uuids::string_to_uuid(std::get<std::string>(session_id));
    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw std::exception("Не достаточно прав доступа!");

    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ServerOnlineClientsList);


    try {
        auto param_ = parse_json(std::get<std::string>(param), true);
        bool is_table = param_.value("table", false);
        result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

        auto table_object = n_json::object();

        if(is_table) {
            auto j = n_json(R"({"session_uuid", "user_name", "user_uuid", "start_date", "app_name", "role"})");
            table_object["columns"] = j;
        }

        auto rows = n_json::array();

        for (auto itr = sessions_.cbegin(); itr != sessions_.cend() ; ++itr) {
            char cur_date[100];
            auto tm = itr->second->start_date();
            std::strftime(cur_date, sizeof(cur_date), "%A %c", &tm);
            std::string dt = arcirk::to_utf(cur_date);
            n_json row = {
                    {"session_uuid", arcirk::uuids::uuid_to_string(itr->second->uuid_session())},
                    {"user_name", itr->second->user_name()},
                    {"user_uuid", arcirk::uuids::uuid_to_string(itr->second->user_uuid())},
                    {"start_date", dt},
                    {"app_name", itr->second->app_name()},
                    {"role", itr->second->role()}
            };
            rows += row;
        }
        table_object["rows"] = rows;

        std::string table = arcirk::base64::base64_encode(table_object.dump());
        std::string  uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

        result.uuid_form = uuid_form;
        result.result = table;

    } catch (std::exception &ex) {
        throw server_commands_exception(ex.what(), result.command, result.uuid_form);
    }

    return result;
}

arcirk::server::server_command_result shared_state::server_version(const variant_t& session_id){
    using namespace arcirk::server;
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ServerVersion);
    result.result = sett.Version;
    return result;
}

//bool shared_state::call_as_proc(const long& method_num, std::vector<variant_t> params) {
//
//    try {
//        //auto args = parseParams(params, array_size);
//        //methods_meta[method_num].call(args);
//        methods_meta[method_num].call(params);
////#ifdef OUT_PARAMS
////        storeParams(args, params);
////#endif
//    } catch (const std::exception &e) {
//        //AddError(ADDIN_E_FAIL, extensionName(), e.what(), true);
//        return false;
//    } catch (...) {
//        //AddError(ADDIN_E_FAIL, extensionName(), UNKNOWN_EXCP, true);
//        return false;
//    }
//
//    return true;
//}

void shared_state::call_as_func(const long& method_num, arcirk::server::server_command_result *ret_value, std::vector<variant_t> params) {

    *ret_value = methods_meta[method_num].call(params);

}

long shared_state::find_method(const std::string &method_name) {
    for (auto i = 0u; i < methods_meta.size(); ++i) {
        if (methods_meta[i].alias == method_name) {
            return static_cast<long>(i);
        }
    }
    return -1;
}

long shared_state::param_count(const long& method_num) const{
    return methods_meta[method_num].params_count;
}

std::string shared_state::get_method_name(const long &num) const {
    return methods_meta[num].alias;
}

arcirk::server::server_command_result shared_state::set_client_param(const variant_t &param, const variant_t& session_id) {

    auto uuid_ = uuids::string_to_uuid(std::get<std::string>(session_id));
    auto session = get_session(uuid_);

    using namespace arcirk::server;
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::SetClientParam);

    if(!session){
        result.result = "error";
        return result;
    }

    try {
        auto param_ = pre::json::from_json<client::client_param>(base64_to_string(std::get<std::string>(param)));
        if(!param_.user_name.empty())
            session->set_user_name(param_.user_name);
        if(!param_.user_uuid.empty()){
            boost::uuids::uuid user_uuid_{};
            arcirk::uuids::is_valid_uuid(param_.user_uuid, user_uuid_);
            session->set_user_uuid(user_uuid_);
        }
        param_.session_uuid = uuids::uuid_to_string(session->uuid_session());
        result.result = arcirk::base64::base64_encode(pre::json::to_json(param_).dump() );
        session->set_app_name(param_.app_name);
        if(use_authorization() && !session->authorized()){
            if(!param_.hash.empty()){
                bool result_auth = verify_auth_from_hash(param_.user_name, param_.hash);
                if(!result_auth){
                    fail("shared_state::set_client_param", "failed authorization");
                    result.message = "failed authorization";
                }
                else{
                    log("shared_state::set_client_param", "successful authorization");
                    session->set_authorized(true);
                    auto info = get_user_info(param_.hash);
                    //если используется авторизация устанавливаем параметры из базы данных
                    set_session_info(session, info);
                }

            }else{
                fail("shared_state::set_client_param", "failed authorization");
                result.message = "failed authorization";
            }


        }
    } catch (std::exception &e) {
        fail("shared_state::set_client_param:error", e.what(), false);
        result.result = "error";
    }

    return result;
}

subscriber*
shared_state::get_session(const boost::uuids::uuid &uuid) {
    auto iter = sessions_.find(uuid);
    if (iter != sessions_.end() ){
        return iter->second;
    }
    return nullptr;
}

//получение всех сессий пользователя
std::vector<subscriber *>
shared_state::get_sessions(const boost::uuids::uuid &user_uuid) {
    auto iter = user_sessions.find(user_uuid);
    if (iter != user_sessions.end() ){
        return iter->second;
    }
    return {};
}

std::string shared_state::base64_to_string(const std::string &base64str) {
    try {
        return arcirk::base64::base64_decode(base64str);
    } catch (std::exception &e) {
        return base64str;
    }
}

bool shared_state::allow_delayed_authorization() const {
    return sett.AllowDelayedAuthorization;
}

boost::filesystem::path shared_state::sqlite_database_path() const {
    using namespace boost::filesystem;
    path db_path(sett.ServerWorkingDirectory);
    db_path /= sett.Version;
    db_path /= "data";
    db_path /= "arcirk.sqlite";

    if(!exists(db_path)){
        throw std::exception("Файл базы данных не найден!");
    }

    return db_path;
}

arcirk::database::user_info shared_state::get_user_info(const boost::uuids::uuid &user_uuid) const{

    using namespace boost::filesystem;
    using namespace soci;

    auto result = arcirk::database::user_info();

    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
        if(sett.ServerWorkingDirectory.empty())
        {
            throw std::exception("Ошибки в параметрах сервера!");
        }

        try {
            path db_path = sqlite_database_path();

            std::string ref = arcirk::uuids::uuid_to_string(user_uuid);

            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", db_path.string());
            session sql(soci::sqlite3, connection_string);
            soci::rowset<arcirk::database::user_info> rs = (sql.prepare << "select * from Users where ref = " <<  "'" << ref << "'");
            int count = -1;
            for (auto it = rs.begin(); it != rs.end(); it++) {
                result = *it;
                count++;
                break;
            }
            if(count < 0)
                throw std::exception("Пользователь не найден!");
        } catch (std::exception &e) {
            fail("shared_state::verify_auth:error", e.what(), false);
        }

    }
    return result;
}

arcirk::database::user_info shared_state::get_user_info(const std::string &hash) const{

    using namespace boost::filesystem;
    using namespace soci;

    auto result = arcirk::database::user_info();

    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
        if(sett.ServerWorkingDirectory.empty())
        {
            //fail("shared_state::get_user_info:error", "Ошибки в параметрах сервера!");
            throw std::exception("Ошибки в параметрах сервера!");
        }

        path database(sett.ServerWorkingDirectory);
        database /= sett.Version;
        database /= "data";
        database /= "arcirk.sqlite";

        if(!exists(database)){
            //fail("shared_state::get_user_info:error", "Файл базы данных не найден!");
            throw std::exception("Файл базы данных не найден!");
        }

        if(hash.empty())
            throw std::exception("Хеш пользователя не указан!");

        try {
            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
            session sql(soci::sqlite3, connection_string);
            soci::rowset<arcirk::database::user_info> rs = (sql.prepare << "select * from Users where hash = " <<  "'" << hash << "'");
            int count = -1;
            for (auto it = rs.begin(); it != rs.end(); it++) {
                result = *it;
                count++;
                break;
            }
            if(count < 0)
                throw std::exception("Пользователь не найден!");
        } catch (std::exception &e) {
            fail("shared_state::get_user_info:error", e.what(), false);
        }

    }
    return result;
}

void shared_state::set_session_info(subscriber* session, const arcirk::database::user_info &info) {
    if(!session)
        return;
    session->set_role(info.role);
    session->set_user_name(info.first);
    session->set_full_name(info.second);
    session->set_user_uuid(arcirk::uuids::string_to_uuid(info.ref));
}

arcirk::server::server_command_result shared_state::server_configuration(const variant_t& param, const variant_t &session_id) {
    using namespace arcirk::database;
    using n_json = nlohmann::json;

    boost::uuids::uuid uuid = arcirk::uuids::string_to_uuid(std::get<std::string>(session_id));
    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw std::exception("Не достаточно прав доступа!");

    std::string conf_json = pre::json::to_json(sett).dump();
    server::server_command_result result;
    result.result = arcirk::base64::base64_encode(conf_json);
    result.command = enum_synonym(server::server_commands::ServerConfiguration);
    result.message = "OK";
    try {
        auto param_ = parse_json(std::get<std::string>(param), true);
        result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    } catch (std::exception &ex) {
        result.uuid_form = uuids::nil_string_uuid();
    }
    return result;
}

bool shared_state::is_operation_available(const boost::uuids::uuid &uuid, arcirk::database::roles level) {

    auto session = get_session(uuid);
    if(!session)
        return false;
    if(use_authorization() && !session->authorized())
        return false;

    return session->role() == enum_synonym(level);

}

arcirk::server::server_command_result shared_state::user_information(const variant_t &param,
                                                                     const variant_t &session_id) {
    using namespace arcirk::database;
    //using n_json = nlohmann::json;

    server::server_command_result result;

    auto param_ = parse_json(std::get<std::string>(param), true);

    std::string session_uuid_str = param_.value("session_uuid", "");
    std::string user_uuid_str = param_.value("user_uuid", "");

    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

    if(session_uuid_str.empty() && user_uuid_str.empty()){
        result.result = "error";
        result.message = arcirk::base64::base64_encode("Не указан идентификатор пользователя или сессии");
        return result;
    }

    auto user_uuid = boost::uuids::nil_uuid();

    if(!user_uuid_str.empty())
        user_uuid = uuids::string_to_uuid(user_uuid_str);
    else{
        auto session_uuid = uuids::string_to_uuid(session_uuid_str);
        auto session = get_session(session_uuid);
        if(!session){
            result.result = "error";
            result.message = arcirk::base64::base64_encode("Запрашиваемая сессия не найдена");
            return result;
        }
        user_uuid = session->user_uuid();
    }

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    auto current_session = get_session(uuid);

    if(user_uuid != current_session->user_uuid()){
        bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
        if (!operation_available)
            throw std::exception("Не достаточно прав доступа!");
    }

    auto usr_info = get_user_info(user_uuid);
    std::string info_json = pre::json::to_json(usr_info).dump();
    result.result = arcirk::base64::base64_encode(info_json);
    result.command = enum_synonym(server::server_commands::UserInfo);
    result.message = "OK";

    return result;
}

arcirk::server::server_command_result shared_state::execute_sql_query(const variant_t &param,
                                                                      const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace boost::filesystem;
    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ExecuteSqlQuery);

    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw std::exception("Не достаточно прав доступа!");

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    std::string query_text = param_.value("query_text", "");
    if(query_text.empty())
        throw std::exception("Не задан текст запроса!");

    auto query = std::make_shared<database::builder::query_builder>();
    auto sql = soci_initialize();
    nlohmann::json table = {};
    query->execute(query_text, sql, table);

    if(!table.empty()){
        result.result = base64::base64_encode(table.dump());
    }

    return result;
}

arcirk::server::server_command_result shared_state::insert_or_update_user(const variant_t &param,
                                                                          const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace boost::filesystem;
    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::InsertOrUpdateUser);

    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw std::exception("Не достаточно прав доступа!");

    std::string param_json = base64_to_string(std::get<std::string>(param));

    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    auto values = param_.value("record", nlohmann::json{});

    if(values.empty() || !values.is_object()){
        throw server_commands_exception("Не заданы параметры запроса!", result.command, result.uuid_form);
    }

    std::string ref, role, first;
    ref = values.value("ref", "");
    role = values.value("role", "");
    first = values.value("first", "");

    if(ref.empty() || role.empty() || first.empty()){
        const std::string err_message = "Не указаны все значения обязательных полей (ref, first, role)!";
        throw server_commands_exception(err_message, result.command, result.uuid_form);
    }

    auto p_info = values_from_param<user_info>(values);
    auto query = std::make_shared<builder::query_builder>();

    path db_path = sqlite_database_path();
    std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", db_path.string());
    session sql(soci::sqlite3, connection_string);

    int count = -1;
    sql << query->select({"count(*)"}).from("Users").where({{"ref", ref}}, true).prepare(), into(count);

    query->use(p_info);
    if(count <= 0){
        query->insert("Users", true).execute(sql);
    }else{
        query->update("Users", true).where({{"ref", ref}}, true).execute(sql);
    }

    result.result = "success";
    result.message = "OK";

    return result;
}

soci::session shared_state::soci_initialize() const {
    using namespace boost::filesystem;
    using namespace soci;

    auto result = arcirk::database::user_info();

    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
        if(sett.ServerWorkingDirectory.empty())
        {
            throw std::exception("Ошибки в параметрах сервера!");
        }

        path database(sett.ServerWorkingDirectory);
        database /= sett.Version;
        database /= "data";
        database /= "arcirk.sqlite";

        if(!exists(database)){
            throw std::exception("Файл базы данных не найден!");
        }

        try {
            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
            return session{soci::sqlite3, connection_string};
        } catch (std::exception &e) {
            fail("shared_state::get_user_info:error", e.what(), false);
        }

    }
    return {};
}

arcirk::server::server_command_result shared_state::get_messages(const variant_t &param, const variant_t &session_id) {

    using namespace arcirk::database;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));

    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::InsertOrUpdateUser);

    std::string param_json = base64_to_string(std::get<std::string>(param));

    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

    std::string sender = param_.value("sender", "");
    std::string recipient = param_.value("recipient", "");

    if(sender.empty() || recipient.empty())
        throw std::exception("Не заданы параметры запроса!");

    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available){
        if(sender != boost::to_string(uuid) && recipient != boost::to_string(uuid))
            throw std::exception("Не достаточно прав доступа!");
    }

    auto sql = soci_initialize();
    std::string token = get_channel_token(sql, sender, recipient);

    auto query = std::make_shared<database::builder::query_builder>();
    nlohmann::json table = {};

    query->select({"*"}).from("Messages").where({{"token", token}}, true).order_by({"date"});
    query->execute(query->prepare(), sql, table);

    result.message = base64::base64_encode(table.dump());

    return result;
}