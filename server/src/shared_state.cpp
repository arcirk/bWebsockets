
#include <utility>
#include "../include/shared_state.hpp"
#include "../include/websocket_session.hpp"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/odbc/soci-odbc.h>

#include <algorithm>
#include <locale>

#include <query_builder.hpp>
#include <boost/thread.hpp>

#include "../include/scheduled_operations.hpp"

#include <wdclient.hpp>

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
    add_method(enum_synonym(server::server_commands::UpdateServerConfiguration), this, &shared_state::update_server_configuration);
    add_method(enum_synonym(server::server_commands::HttpServiceConfiguration), this, &shared_state::get_http_service_configuration);
    add_method(enum_synonym(server::server_commands::WebDavServiceConfiguration), this, &shared_state::get_dav_service_configuration);
    add_method(enum_synonym(server::server_commands::InsertToDatabaseFromArray), this, &shared_state::insert_to_database_from_array);
    add_method(enum_synonym(server::server_commands::SetNewDeviceId), this, &shared_state::set_new_device_id);
    add_method(enum_synonym(server::server_commands::ObjectSetToDatabase), this, &shared_state::object_set_to_database);
    add_method(enum_synonym(server::server_commands::ObjectGetFromDatabase), this, &shared_state::object_get_from_database);
    add_method(enum_synonym(server::server_commands::SyncGetDiscrepancyInData), this, &shared_state::sync_get_discrepancy_in_data);
    add_method(enum_synonym(server::server_commands::SyncUpdateDataOnTheServer), this, &shared_state::sync_update_data_on_the_server);
    add_method(enum_synonym(server::server_commands::SyncUpdateBarcode), this, &shared_state::sync_update_barcode);
    add_method(enum_synonym(server::server_commands::DownloadAppUpdateFile), this, &shared_state::download_app_update_file);
    add_method(enum_synonym(server::server_commands::GetInformationAboutFile), this, &shared_state::get_information_about_file);
    add_method(enum_synonym(server::server_commands::CheckForUpdates), this, &shared_state::check_for_updates);
    add_method(enum_synonym(server::server_commands::UploadFile), this, &shared_state::upload_file);
    add_method(enum_synonym(server::server_commands::GetDatabaseTables), this, &shared_state::get_database_tables);

    run_server_tasks();

    sql_sess = new soci::session();// soci_initialize();

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
    resp.version = ARCIRK_VERSION;
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
    resp.sender_name = session->user_name();
    resp.sender_uuid = arcirk::uuids::uuid_to_string(session->user_uuid());
    resp.version = ARCIRK_VERSION;

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

    resp.receiver_name = itr->second->user_name();
    resp.receiver_uuid = arcirk::uuids::uuid_to_string(itr->second->user_uuid());

    if(sett.AllowHistoryMessages){
        std::string content_type_ = arcirk::enum_synonym(database::text_type::dbText);
        if(!param.empty()){
            auto param_ = nlohmann::json::parse(arcirk::base64::base64_decode(param));
            content_type_ = param_.value("content_type", content_type_);
        }else
            log("shared_state::forward_message", "Не указан тип сообщения, будет установлен по умолчанию 'Text'");

        auto msg_struct = database::messages();
        msg_struct.ref = boost::to_string(uuids::random_uuid());
        msg_struct.first = boost::to_string(session->user_uuid());
        msg_struct.second = boost::to_string(itr->second->user_uuid());
        msg_struct.message = msg;
        msg_struct.content_type = content_type_;
        msg_struct.date = (int) arcirk::date_to_seconds();
        msg_struct.unread_messages = 1;
        try {
            auto sql = soci_initialize();
            msg_struct.token  = get_channel_token(*sql, boost::to_string(session->user_uuid()), boost::to_string(itr->second->user_uuid()));
            if(msg_struct.token == "error"){
                fail("shared_state::forward_message", "Ошибка генерации токена!");
                return;
            }
            auto query = database::builder::query_builder((database::builder::sql_database_type)sett.SQLFormat);
            query.use(pre::json::to_json(msg_struct));
            query.insert("Messages", true).execute(*sql);
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

    auto builder = query_builder();
    std::vector<std::string> refs;

    try {
        nlohmann::json ref = {
                {"ref", sql_compare_value("ref", {
                        first,
                        second}, sql_type_of_comparison::On_List).to_object()}
        };
        auto result = builder.select({"_id","ref"}).from("Users").where(ref, true).order_by({"_id"}).exec(sql,{}, true);


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
//            if(json_params.substr(json_params.length() - 1, 1) != "}")
//                json_params.append("\"}");
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
        try {
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
        } catch (const std::exception &ex) {
            fail("shared_state::execute_command_handler", ex.what());
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
    result_response.version = ARCIRK_VERSION;
    if(!is_command_to_client){
        result_response.receiver = arcirk::uuids::uuid_to_string(session->uuid_session());
        result_response.uuid_form = return_value.uuid_form;
        result_response.param = return_value.result;
    }else
        result_response.receiver = session_id_receiver;

    result_response.app_name = session->app_name();
    result_response.param = return_value.param;
    result_response.data = return_value.data;

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
                if(m_auth.size() == 1){
                    auto source = v[0];
                    arcirk::trim(source);
                    arcirk::to_lower(source);
                    if( source == "token"){
                        return verify_auth_from_hash(m_auth[0]);
                    }else
                        return false;
                }
                else if(m_auth.size() == 2)
                    return verify_auth(m_auth[0], m_auth[1]);
                else
                    return false;
            }
        } catch (std::exception &e) {
            fail("shared_state::verify_connection:error", e.what(), false);
        }
    }

    return false;
}
bool shared_state::verify_auth_from_hash(const std::string &hash) {

    log("shared_state::verify_auth_from_hash", "verify_connection ... ");

    using namespace boost::filesystem;
    using namespace soci;
    using namespace arcirk::database;

        try {
            auto sql = soci_initialize();
            int count = 0;
             *sql <<  builder::query_builder((builder::sql_database_type)sett.SQLFormat).row_count().from(enum_synonym(tables::tbUsers)).where(nlohmann::json{{"hash", hash}}, true).prepare(), soci::into(count);
            return count > 0;
        } catch (std::exception &e) {
            fail("shared_state::verify_auth:error", e.what(), false);
        }

    //}
    return false;
}

bool shared_state::verify_auth(const std::string& usr, const std::string& pwd ) {

    using namespace boost::filesystem;
    using namespace soci;

    std::string hash = arcirk::get_hash(usr, pwd);
    return verify_auth_from_hash(hash);

}

auto shared_state::parse_json(const std::string &json_text, bool is_base64) {

    using n_json = nlohmann::json;
    std::string json_param;

    if(json_text.empty())
        throw native_exception("Не верный формат json!");

    if(is_base64)
        json_param = arcirk::base64::base64_decode(json_text);
    else
        json_param = json_text;

    auto param_ = n_json::parse(json_param, nullptr, false);
    if(param_.is_discarded()){
        throw native_exception("Не верный формат json!");
    }

    return param_;

}

arcirk::server::server_command_result shared_state::command_to_client(const variant_t &param,
                                                                      const variant_t &session_id,
                                                                      const variant_t &session_id_receiver) {

    auto session_receiver = get_session(arcirk::uuids::string_to_uuid(std::get<std::string>(session_id_receiver)));
    if(!session_receiver)
        throw native_exception("Сессия получателя не найдена!");

    //ToDo: Парсинг параметров если необходимо

    //
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::CommandToClient);
    result.param = std::get<std::string>(param);

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
    bool operation_available = is_operation_available(uuid, roles::dbUser);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ServerOnlineClientsList);

    try {
        auto param_ = parse_json(std::get<std::string>(param), true);
        bool is_table = param_.value("table", false);
        bool empty_column = param_.value("empty_column", false); //пустой первый столбец в модели QT
        result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

        auto table_object = n_json::object();

        auto columns = n_json::array();

        if(is_table) {
            std::vector<std::string> m_cols{"session_uuid", "user_name", "user_uuid", "start_date", "app_name", "role", "device_id", "address"};
            if(empty_column){
                m_cols.insert(m_cols.cbegin(), "empty");
            }
            for (auto const& itr : m_cols) {
                columns += itr;
            }
            //auto j = n_json{"empty", "session_uuid", "user_name", "user_uuid", "start_date", "app_name", "role", "device_id", "address"};
            table_object["columns"] = columns;
        }

        auto rows = n_json::array();

        for (auto itr = sessions_.cbegin(); itr != sessions_.cend() ; ++itr) {
            if(sett.UseAuthorization && !itr->second->authorized())
                continue;
            char cur_date[100];
            auto tm = itr->second->start_date();
            std::strftime(cur_date, sizeof(cur_date), "%A %c", &tm);
            std::string dt = arcirk::to_utf(cur_date);

            n_json row = n_json::object();
            if(empty_column){
                row["empty"] = " ";
            }
            row["session_uuid"] = arcirk::uuids::uuid_to_string(itr->second->uuid_session());
            row["user_name"] = itr->second->user_name();
            row["user_uuid"] = arcirk::uuids::uuid_to_string(itr->second->user_uuid());
            row["start_date"] = dt;
            row["app_name"] = itr->second->app_name();
            row["role"] = itr->second->role();
            row["device_id"] = arcirk::uuids::uuid_to_string(itr->second->device_id());
            row["address"] = itr->second->address();

//            n_json row = {
//                    {"empty", " "},
//                    {"session_uuid", arcirk::uuids::uuid_to_string(itr->second->uuid_session())},
//                    {"user_name", itr->second->user_name()},
//                    {"user_uuid", arcirk::uuids::uuid_to_string(itr->second->user_uuid())},
//                    {"start_date", dt},
//                    {"app_name", itr->second->app_name()},
//                    {"role", itr->second->role()},
//                    {"device_id", arcirk::uuids::uuid_to_string(itr->second->device_id())},
//                    {"address", itr->second->address()}
//            };
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
        session->set_device_id(uuids::string_to_uuid(param_.device_id));
        if(use_authorization() && !session->authorized()){
            if(!param_.hash.empty()){
                bool result_auth = verify_auth_from_hash(param_.hash);
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
                    info.hash = "";
                    param_.user_uuid = info.ref;
                    result.result = base64::base64_encode(pre::json::to_json(info).dump());
                }

            }else{
                fail("shared_state::set_client_param", "failed authorization");
                result.message = "failed authorization";
            }
        }

        result.param = base64::base64_encode(pre::json::to_json(param_).dump());

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
        throw native_exception("Файл базы данных не найден!");
    }

    return db_path;
}

arcirk::database::user_info shared_state::get_user_info(const boost::uuids::uuid &user_uuid){

    using namespace boost::filesystem;
    using namespace soci;

    auto result = arcirk::database::user_info();

//    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
//        if(sett.ServerWorkingDirectory.empty())
//        {
//            throw native_exception("Ошибки в параметрах сервера!");
//        }

        try {
//            path db_path = sqlite_database_path();
//
            std::string ref = arcirk::uuids::uuid_to_string(user_uuid);
//
//            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", db_path.string());
//            session sql(soci::sqlite3, connection_string);
            auto sql = soci_initialize();
            soci::rowset<arcirk::database::user_info> rs = (sql->prepare << "select * from Users where ref = " <<  "'" << ref << "'");
            int count = -1;
            for (auto it = rs.begin(); it != rs.end(); it++) {
                result = *it;
                count++;
                break;
            }
            if(count < 0)
                throw native_exception("Пользователь не найден!");
        } catch (std::exception &e) {
            fail("shared_state::verify_auth:error", e.what(), false);
        }

    //}
    return result;
}

arcirk::database::user_info shared_state::get_user_info(const std::string &hash){

    using namespace boost::filesystem;
    using namespace soci;

    auto result = arcirk::database::user_info();

//    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
//        if(sett.ServerWorkingDirectory.empty())
//        {
//            //fail("shared_state::get_user_info:error", "Ошибки в параметрах сервера!");
//            throw native_exception("Ошибки в параметрах сервера!");
//        }
//
//        path database(sett.ServerWorkingDirectory);
//        database /= sett.Version;
//        database /= "data";
//        database /= "arcirk.sqlite";
//
//        if(!exists(database)){
//            //fail("shared_state::get_user_info:error", "Файл базы данных не найден!");
//            throw native_exception("Файл базы данных не найден!");
//        }

        if(hash.empty())
            throw native_exception("Хеш пользователя не указан!");

        try {
            //std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
            //session sql(soci::sqlite3, connection_string);
            auto sql = soci_initialize();
            soci::rowset<arcirk::database::user_info> rs = (sql->prepare << "select * from Users where hash = " <<  "'" << hash << "'");
            int count = -1;
            for (auto it = rs.begin(); it != rs.end(); it++) {
                result = *it;
                count++;
                break;
            }
            if(count < 0)
                throw native_exception("Пользователь не найден!");
        } catch (std::exception &e) {
            fail("shared_state::get_user_info:error", e.what(), false);
        }

   //}
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

arcirk::server::server_command_result shared_state::update_server_configuration(const variant_t& param, const variant_t &session_id) {

    using namespace arcirk::database;
    using n_json = nlohmann::json;

    boost::uuids::uuid uuid = arcirk::uuids::string_to_uuid(std::get<std::string>(session_id));
    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    auto param_ = parse_json(std::get<std::string>(param), true);
    auto p = param_.value("config", n_json::object());
    if(!p.empty()){
        sett = pre::json::from_json<arcirk::server::server_config>(p.dump());
        write_conf(sett, app_directory(), ARCIRK_SERVER_CONF);
    }

    server::server_command_result result;
    result.result = "";
    result.command = enum_synonym(server::server_commands::UpdateServerConfiguration);
    result.message = "OK";

    return result;
}

arcirk::server::server_command_result shared_state::server_configuration(const variant_t& param, const variant_t &session_id) {
    using namespace arcirk::database;
    using n_json = nlohmann::json;

    boost::uuids::uuid uuid = arcirk::uuids::string_to_uuid(std::get<std::string>(session_id));
    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

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

    using json = nlohmann::json;
    auto session = get_session(uuid);
    if(!session)
        return false;
    if(use_authorization() && !session->authorized())
        return false;


    json role_ = session->role();
    int u_role = (int)role_.get<arcirk::database::roles>();
    int d_role = (int)level;

    //return session->role() == enum_synonym(level);
    return u_role >= d_role;
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
            throw native_exception("Не достаточно прав доступа!");
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
    result.param = std::get<std::string>(param);

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

    std::string query_text = param_.value("query_text", "");
    auto sql = soci_initialize();
    if(!query_text.empty()){
        //произвольный запрос только с правами администратора
        bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
        if (!operation_available)
            throw native_exception("Не достаточно прав доступа!");

        result.result = base64::base64_encode(execute_random_sql_query(*sql, query_text)); //текст запроса передан в параметрах
    }
    else{
        std::string base64_query_param = param_.value("query_param", "");
        if(!base64_query_param.empty()){
            std::string str_query_param = base64_to_string(base64_query_param);
            //std::cout << str_query_param << std::endl;
            auto query_param = nlohmann::json::parse(str_query_param);
            std::string table_name = query_param.value("table_name", "");
            std::string query_type = query_param.value("query_type", "");
            bool line_number = query_param.value("line_number", false);
            bool empty_column = query_param.value("empty_column", false);

            if(query_type.empty())
                throw native_exception("Не указан тип запроса!");

            if (query_type == "insert" || query_type == "update" || query_type == "update_or_insert" || query_type == "delete"){
                if(table_name != arcirk::enum_synonym(arcirk::database::tables::tbDevices) &&
                   table_name != arcirk::enum_synonym(arcirk::database::tables::tbMessages)
                   &&
                   table_name != arcirk::enum_synonym(arcirk::database::tables::tbDocuments)&&
                   table_name != arcirk::enum_synonym(arcirk::database::tables::tbDocumentsTables)){
                    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
                    if (!operation_available)
                        throw native_exception("Не достаточно прав доступа!");
                }else{
                    bool operation_available = is_operation_available(uuid, roles::dbUser);
                    if (!operation_available)
                        throw native_exception("Не достаточно прав доступа!");
                }
            }

            auto values = query_param.value("values", nlohmann::json::object());
            auto where_values = query_param.value("where_values", nlohmann::json::object());
            auto order_by = query_param.value("order_by", nlohmann::json::object());
            auto not_exists = query_param.value("not_exists", nlohmann::json::object());

            if(!table_name.empty()){
                bool return_table = false;
                auto query = database::builder::query_builder((builder::sql_database_type)sett.SQLFormat);
                if(query_type == "select"){
                    if(!values.empty())
                        query.select(values).from(table_name);
                    else
                        query.select({"*"}).from(table_name);
                    return_table = true;
                }else if(query_type == "insert"){
                    query.use(values);
                    query.insert(table_name, true);
                }else if(query_type == "update"){
                    query.use(values);
                    query.update(table_name, true);
                }else if(query_type == "update_or_insert"){
                    query.use(values);
                    std::string ref = query.ref();
                    if(ref.empty())
                        throw native_exception("Не найдено значение идентификатора для сравнения!");
                    int count = 0;
                    auto query_temp = database::builder::query_builder((builder::sql_database_type)sett.SQLFormat);
                    *sql << query_temp.select({"count(*)"}).from(table_name).where({{"ref", ref}}, true).prepare(), into(count);
                    if(count <= 0){
                        query.insert(table_name, true);
                    }else{
                        query.update(table_name, true).where({{"ref", ref}}, true);
                    }
                }else if(query_type == "delete"){
                    query.remove().from(table_name);
                }
                if(query.is_valid()){
                    if(!where_values.empty()){
                        query.where(where_values, true);
                    }
                    if(!order_by.empty()){
                        query.order_by(order_by);
                    }
                    if(not_exists.empty())
                        query_text = query.prepare();
                    else{
                        query_text = query.prepare(not_exists, true);
                    }
                }
                //std::cout << "shared_state::execute_sql_query: \n" << query_text << std::endl;
                if(return_table)
                    result.result = base64::base64_encode(execute_random_sql_query(*sql, query_text, line_number, empty_column));
                else{
                    query.execute(*sql, {}, true);
                    result.result = "{}";
                }
            }
        }
    }

    return result;
}

std::string shared_state::execute_random_sql_query(soci::session &sql, const std::string &query_text, bool add_line_number, bool add_empty_column) {

    if(query_text.empty())
        throw native_exception("Не задан текст запроса!");

    auto query = std::make_shared<database::builder::query_builder>();
    nlohmann::json table = {};
    query->execute(query_text, sql, table, {}, add_line_number, add_empty_column);
    if(!table.empty()){
        return table.dump();
    }else
        return {};
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
        throw native_exception("Не достаточно прав доступа!");

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
    auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);
    auto sql = soci_initialize();

//    path db_path = sqlite_database_path();
//    std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", db_path.string());
//    session sql(soci::sqlite3, connection_string);
//
    int count = -1;
//    sql << query.select({"count(*)"}).from("Users").where({{"ref", ref}}, true).prepare(), into(count);
    *sql << query.row_count().from(arcirk::enum_synonym(database::tables::tbUsers)).where({{"ref", ref}}, true).prepare(), into(count);
    query.use(p_info);
    if(count <= 0){
        query.insert(arcirk::enum_synonym(database::tables::tbUsers), true).execute(*sql);
    }else{
        query.update(arcirk::enum_synonym(database::tables::tbUsers), true).where({{"ref", ref}}, true).execute(*sql);
    }

    result.result = "success";
    result.message = "OK";

    return result;
}
soci::session * shared_state::soci_initialize(){

    using namespace boost::filesystem;
    using namespace soci;

    auto version = arcirk::server::get_version();
    std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.major), std::to_string(version.minor), std::to_string(version.path));

    if(sql_sess->is_connected()){
        if(sett.SQLFormat == DatabaseType::dbTypeODBC){
            *sql_sess << "use " + db_name;
        }
        return sql_sess;
    }

    //auto result = arcirk::database::user_info();

    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
        if(sett.ServerWorkingDirectory.empty())
        {
            throw native_exception("Ошибки в параметрах сервера!");
        }

        path database(sett.ServerWorkingDirectory);
        database /= sett.Version;
        database /= "data";
        database /= "arcirk.sqlite";

        if(!exists(database)){
            throw native_exception("Файл базы данных не найден!");
        }

        try {
            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
            //return session{soci::sqlite3, connection_string};
            sql_sess->open(soci::sqlite3, connection_string);
            log("soci_initialize", "Open sqlite3 database.");
            return sql_sess;
        } catch (native_exception &e) {
            fail("shared_state::soci_initialize:error", e.what(), false);
        }
    }else{
        std::string connection_string = arcirk::str_sample("DRIVER={SQL Server};"
                                                           "SERVER=%1%;Persist Security Info=true;"
                                                           "uid=%2%;pwd=%3%", sett.SQLHost, sett.SQLUser, sett.SQLPassword);
        sql_sess->open(soci::odbc, connection_string);
        if(sql_sess->is_connected())
            *sql_sess << "use " + db_name;
        log("soci_initialize", "Open odbc driver database.");
        return sql_sess;
    }
    return nullptr;
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

    log("shared_state::get_messages", "sender: " + sender + " receiver:" + recipient);

    if(sender.empty() || recipient.empty())
        throw native_exception("Не заданы параметры запроса!");

    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    auto current_session = get_session(uuid);
    if(!current_session)
        throw native_exception("Не достаточно прав доступа!");
    if (!operation_available){
        if(sender != boost::to_string(current_session->user_uuid()) && recipient != boost::to_string(current_session->user_uuid()))
            throw native_exception("Не достаточно прав доступа!");
    }

    auto sql = soci_initialize();
    std::string token = get_channel_token(*sql, sender, recipient);

    auto query = database::builder::query_builder((builder::sql_database_type)sett.SQLFormat);
    nlohmann::json table = {};

    query.select({"*"}).from("Messages").where({{"token", token}}, true).order_by({"date"});
    query.execute(query.prepare(), *sql, table);

    result.message = base64::base64_encode(table.dump());

    return result;
}

arcirk::server::server_command_result shared_state::get_http_service_configuration(const variant_t &param,
                                                                                   const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace boost::filesystem;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::HttpServiceConfiguration);

    bool operation_available = is_operation_available(uuid, roles::dbUser);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    nlohmann::json res = {
            {"HSHost", sett.HSHost},
            {"HSUser", sett.HSUser},
            {"HSPassword", sett.HSPassword}
    };

    result.result = arcirk::base64::base64_encode(res.dump());
    result.message = "OK";

    return result;
}

arcirk::server::server_command_result shared_state::get_dav_service_configuration(const variant_t &param,
                                                                                   const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace boost::filesystem;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::WebDavServiceConfiguration);

    bool operation_available = is_operation_available(uuid, roles::dbUser);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    nlohmann::json res = {
            {"WebDavHost", sett.WebDavHost},
            {"WebDavUser", sett.WebDavUser},
            {"WebDavPwd", sett.WebDavPwd},
            {"WebDavSSL", sett.WebDavSSL}
    };

    result.result = arcirk::base64::base64_encode(res.dump());
    result.message = "OK";

    return result;
}

arcirk::server::server_command_result shared_state::set_new_device_id(const variant_t &param,
                                                                      const variant_t &session_id) {
    using namespace arcirk::database;
    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::SetNewDeviceId);

    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    std::string param_json = base64_to_string(std::get<std::string>(param));

    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    std::string remote_session_ = param_.value("remote_session", "");
    std::string new_uuid_device = param_.value("device_id", "");

    if(new_uuid_device.empty())
        throw native_exception("Не указан новый идентификатор устройства!");

    auto uuid_device_ = uuids::string_to_uuid(new_uuid_device);
    if(uuid_device_ == uuids::nil_uuid())
        throw native_exception("Указан не корректный идентификатор устройства!");

    if(remote_session_.empty())
        throw native_exception("Не указана удаленная сессия клиента!");

    auto uuid_remote_session = uuids::string_to_uuid(remote_session_);
    auto remote_session = get_session(uuid_remote_session);

    if(!remote_session)
        throw native_exception("Не найдена удаленная сессия клиента!");

    remote_session->set_device_id(uuid_device_);

    nlohmann::json remote_param = {
            {"command", enum_synonym(server::server_commands::SetNewDeviceId)},
            {"param", base64::base64_encode(nlohmann::json({
                                                                   {"device_id", new_uuid_device}
                                                           }).dump())},
    };

    nlohmann::json cmd_param = {
            {"param", base64::base64_encode(remote_param.dump())}
    };

    //эмитируем команду клиенту
    execute_command_handler("cmd " + enum_synonym(server::server_commands::CommandToClient) + " " + remote_session_ + " " +
                            base64::base64_encode(cmd_param.dump()), get_session(uuid));

    result.message = "OK";
    result.result = "success";

    return result;
}

arcirk::server::server_command_result shared_state::insert_to_database_from_array(const variant_t &param,
                                                                                  const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace boost::filesystem;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::InsertToDatabaseFromArray);

    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    std::string param_json = base64_to_string(std::get<std::string>(param));

    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

    std::string base64_query_param = param_.value("query_param", "");
    if(!base64_query_param.empty()) {
        auto query_param = nlohmann::json::parse(base64_to_string(base64_query_param));
        std::string table_name = query_param.value("table_name", "");
        auto values_array = query_param.value("values_array", nlohmann::json{});
        //устарела, оставлена для совместимости
        std::string where_is_exists_field = query_param.value("where_is_exists_field", "");

        //если where_values = Структура, тогда параметры единые для всего массива записей
        //если where_values = Массив, тогда where_values - это массив полей, значения в каждой записи свои
        auto where_values = query_param.value("where_values", nlohmann::json{});

        bool delete_is_exists = query_param.value("delete_is_exists", false);

        if (table_name.empty())
            throw native_exception("Не указана таблица.");

        if (values_array.empty() || !values_array.is_array()) {
            throw server_commands_exception("Не заданы параметры запроса!", result.command, result.uuid_form);
        }

        auto sql = soci_initialize();
        auto tr = soci::transaction(*sql);
        auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);
        auto items = values_array.items();

        if(delete_is_exists){
            if(where_values.is_object()){
                //удалить все записи удовлетворяющие условиям
                *sql << query.remove().from(table_name).where(where_values, true).prepare();
            }else if(where_values.is_array()){
                //удалить выбранные записи удовлетворяющие условиям
                for (auto itr = items.begin(); itr != items.end(); ++itr) {
                    if (!itr.value().is_object())
                        throw native_exception("Не верная запись в массиве.");
                    query.clear();
                    nlohmann::json where{};
                    for (auto itr_ = where_values.begin();  itr_ != where_values.end() ; ++itr_) {
                        where += {
                                {itr, itr.value().value(*itr_, "")}
                        };
                    }
                    *sql << query.remove().from(table_name).where(where, true).prepare();
                }
            }
        }

        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            if (!itr.value().is_object())
                throw native_exception("Не верная запись в массиве.");

            query.clear();
            query.use(itr.value());
            query.insert(table_name, true);
            std::string query_text;
            if(where_values.is_object()) {
                query_text = query.prepare(where_values, true);
            }else if(where_values.is_array()){
                nlohmann::json where{};
                for (auto itr_ = where_values.begin();  itr_ != where_values.end() ; ++itr_) {
                    where += {
                            {*itr_, itr.value().value(*itr_, "")}
                    };
                }
                query_text = query.prepare(where, true);
            }

            *sql << query_text;

//            if (!where_is_exists_field.empty()) {
//                std::string is_exists_val = itr.value().value(where_is_exists_field, "");
//                if (is_exists_val.empty())
//                    throw native_exception("Не верная запись в поле сравнения.");
//                query_text = query->prepare({
//                                                    {where_is_exists_field, is_exists_val}
//                                            }, true);
//
//                sql << query_text;
//            }
        }
        tr.commit();
    }else
        throw server_commands_exception("Не заданы параметры запроса!", result.command, result.uuid_form);

    result.message = "OK";
    result.result = "success";

    return result;
}

bool shared_state::edit_table_only_admin(const std::string &table_name) {

    std::vector<std::string> vec;
    vec.emplace_back("Users");
    vec.emplace_back("Organizations");
    vec.emplace_back("Subdivisions");
    vec.emplace_back("Warehouses");
    vec.emplace_back("PriceTypes");
    vec.emplace_back("Workplaces");
    vec.emplace_back("DevicesType");

    return std::find(vec.begin(), vec.end(), table_name) != vec.end();
}

void shared_state::data_synchronization_set_object(const nlohmann::json &object, const std::string& table_name) {

    using namespace arcirk::database;
    using namespace soci;

    auto standard_attributes = object.value("StandardAttributes", nlohmann::json{});
    nlohmann::json enm_json = table_name;
    auto enm_val = enm_json.get<arcirk::database::tables>();
    auto table_json = table_default_json(enm_val);
    auto items = table_json.items();
    for (auto itr = items.begin();  itr != items.end() ; ++itr) {
        table_json[itr.key()] = standard_attributes[itr.key()];
    }

    auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);
    query.use(table_json);
    std::string ref = query.ref();

    if(ref.empty())
        throw native_exception("Не указан идентификатор объекта!");

    auto sql = soci_initialize();
    int count = -1;
    *sql << query.select({"count(*)"}).from(table_name).where({{"ref", ref}}, true).prepare(), into(count);

    auto tr = soci::transaction(*sql);

    query.clear();
    query.use(table_json);

    if(count > 0)
        *sql << query.update(table_name, true).where({{"ref", ref}}, true).prepare();
    else
        *sql << query.insert(table_name, true).prepare();

    if(enm_val == tbDocuments){
        query.clear();
        *sql << query.remove().from("DocumentsTables").where({{"parent", ref}}, true).prepare();
    }

    auto tabular_sections = object.value("TabularSections", nlohmann::json{});
    if(tabular_sections.is_array()){
        for (auto itr = tabular_sections.begin();  itr != tabular_sections.end() ; ++itr) {
            nlohmann::json table_section = *itr;
            if(table_section.is_object()){
                std::string name = table_section.value("name", "");
                auto rows = table_section.value("strings", nlohmann::json{});
                if(rows.is_array()){
                    //auto rows_items = rows.items();
                    for (auto itr_row = rows.begin();  itr_row != rows.end() ; ++itr_row) {
                        nlohmann::json row_ = *itr_row;
                        if(row_.is_object()){
                            query.clear();
                            query.use(row_);
                            if(enm_val == tbDocuments){
                                std::string query_text = query.insert("DocumentsTables", true).prepare();
                                *sql << query_text;
                            }

                        }
                    }
                }
            }
        }
    }

    tr.commit();
}

arcirk::server::server_command_result shared_state::object_set_to_database(const variant_t &param,
                                                                           const variant_t &session_id) {
    using namespace arcirk::database;
//    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ObjectSetToDatabase);

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    std::string base64_object = param_.value("base64_object", "");

    if(!base64_object.empty()) {
        auto object_struct = nlohmann::json::parse(base64_to_string(base64_object));
        std::string table_name = object_struct.value("table_name", "");

        if(edit_table_only_admin(table_name) ){
            bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
            if (!operation_available)
                throw native_exception("Не достаточно прав доступа!");
        }else{
            bool operation_available = is_operation_available(uuid, roles::dbUser);
            if (!operation_available)
                throw native_exception("Не достаточно прав доступа!");

        }

        auto object = object_struct.value("object", nlohmann::json{});

        if(object.empty() || !object.is_object())
            throw native_exception("Не верная структура объекта!");

        data_synchronization_set_object(object, table_name);
    }


    result.message = "OK";
    result.result = "success";

    return result;

}

nlohmann::json shared_state::data_synchronization_get_object(const std::string& table_name, const std::string& ref) {

    using namespace arcirk::database;
    using namespace soci;

    auto sql = soci_initialize();
    auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);

    nlohmann::json j_table = table_name;
    nlohmann::json j_object{};
    auto o_table = j_table.get<database::tables>();
    if(o_table == database::tbDocuments){
        std::vector<database::documents> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                            true).rows_to_array<database::documents>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object["object"]["StandardAttributes"] = pre::json::to_json<database::documents>(r);

            query.clear();
            std::vector<database::document_table> m_vec_table = query.select({"*"}).from(
                    arcirk::enum_synonym(database::tables::tbDocumentsTables)).where({{"parent", ref}},
                                                                                     true).rows_to_array<database::document_table>(
                    *sql);
            nlohmann::json n_json_table{};
            for (const auto itr : m_vec_table) {
                n_json_table += pre::json::to_json<database::document_table>(itr);
            }
            j_object["object"]["TabularSections"] = n_json_table;
//            j_object["object"] += {
//                    {"TabularSections", n_json_table}
//            };
        }
    }else if(o_table == database::tbDevices){
        std::vector<database::devices> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                          true).rows_to_array<database::devices>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes", pre::json::to_json<database::devices>(r)}}
            };
        }
    }else if(o_table == database::tbMessages){
        std::vector<database::messages> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                           true).rows_to_array<database::messages>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes" ,pre::json::to_json<database::messages>(r)}}
            };
        }
    }else if(o_table == database::tbOrganizations){
        std::vector<database::organizations> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                                true).rows_to_array<database::organizations>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes" ,pre::json::to_json<database::organizations>(r)}}
            };
        }
    }else if(o_table == database::tbPriceTypes){
        std::vector<database::price_types> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                              true).rows_to_array<database::price_types>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes" ,pre::json::to_json<database::price_types>(r)}}
            };
        }
    }else if(o_table == database::tbSubdivisions){
        std::vector<database::subdivisions> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                               true).rows_to_array<database::subdivisions>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes" ,pre::json::to_json<database::subdivisions>(r)}}
            };
        }
    }else if(o_table == database::tbWarehouses){
        std::vector<database::warehouses> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                             true).rows_to_array<database::warehouses>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes" ,pre::json::to_json<database::warehouses>(r)}}
            };
        }
    }else if(o_table == database::tbWorkplaces){
        std::vector<database::workplaces> m_vec = query.select({"*"}).from(table_name).where({{"ref", ref}},
                                                                                             true).rows_to_array<database::workplaces>(
                *sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                    {"object", {"StandardAttributes" ,pre::json::to_json<database::workplaces>(r)}}
            };
        }
    }else
        throw native_exception("Сериализация выбранной таблицы не поддерживается");

    j_object["table_name"] = table_name;

    return j_object;
}

arcirk::server::server_command_result shared_state::object_get_from_database(const variant_t &param,
                                                                             const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::ObjectGetFromDatabase);

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    std::string ref = param_.value("object_ref", "");
    std::string table_name = param_.value("table_name", "");

    if(ref.empty() || table_name.empty())
        throw native_exception("Не заданы параметры запроса!");


    auto j_object = data_synchronization_get_object(table_name, ref);

    result.result = base64::base64_encode(j_object.dump());
    result.message = "OK";

    return result;
}

arcirk::server::server_command_result shared_state::sync_get_discrepancy_in_data(const variant_t &param,
                                                                                 const variant_t &session_id) {
    using namespace arcirk::database;
    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::SyncGetDiscrepancyInData);

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    std::string device_id = param_.value("device_id", "");
    std::string table_name = param_.value("table_name", "");
    std::string workplace = param_.value("workplace", "");
    std::string parent = param_.value("parent", "");
    std::string base64_param = param_.value("base64_param", ""); //упакованная таблица с локального устройства
    nlohmann::json ext_table{};

    if (table_name.empty() || device_id.empty())
        throw native_exception("Не достаточно параметров для выполнения запроса!");

    if (table_name == arcirk::enum_synonym(tables::tbDocumentsTables) && parent.empty())
        throw native_exception("Не достаточно параметров для выполнения запроса!");

    bool operation_available = is_operation_available(uuid, roles::dbUser);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    auto sql = soci_initialize();

    //nlohmann::json result_table{};
    //nlohmann::json t = table_name;
    //auto table_type = t.get<tables>();
    auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);

    int count = -1;
    //Проверим зарегистрировано ли устройство
    query.row_count().from(enum_synonym(tables::tbDevices)).where(nlohmann::json{
            {"ref", device_id}
    }, true);
    *sql << query.prepare(), into(count);

    if (count <= 0)
        throw native_exception(str_sample("Устройство с идентификатором %1% не зарегистрировано!", device_id).c_str());

    query.clear();

    if (!base64_param.empty()) {
        ext_table = nlohmann::json::parse(arcirk::base64::base64_decode(base64_param));
    }

    //if(table_type == tables::tbDocuments){

    auto tr = soci::transaction(*sql);
    //Заполняем временную таблицу данными с клиента
    std::string temp_table = table_name + "_temp";
    std::string sql_ddl;
    std::string temp_pref = "";
    std::string temp_alias = "tmp";

    if(sett.SQLFormat == 0)
        sql_ddl = str_sample("CREATE TEMP TABLE IF NOT EXISTS  %1% (\n"
                                     "ref TEXT,\n"
                                     "version INTEGER\n"
                                     ");\n", temp_table);
    else{
        //sql_ddl = str_sample("IF NOT EXISTS (SELECT * FROM sysobjects WHERE name='#%1%' and xtype='U')\n", temp_table);
        try {
            *sql << arcirk::str_sample("DROP TABLE ##%1%", table_name + "_temp");
        }catch (...){
            //
        }
        sql_ddl.append(str_sample("CREATE TABLE ##%1% (\n"
                             "[ref] [char](36),\n"
                             "[version] [int] \n"
                             ");\n", temp_table));

        temp_pref = "##";
    }

    *sql << sql_ddl;
    try {
        if (ext_table.is_array() && !ext_table.empty()) {
            for (auto itr = ext_table.begin(); itr != ext_table.end(); ++itr) {
                nlohmann::json r = *itr;
                query.clear();
                query.use(r);
                sql_ddl = query.insert(temp_pref + temp_table, true).prepare();
                *sql << sql_ddl;
            }
        }
    } catch (std::exception const &e) {
        fail("shared_state::sync_get_discrepancy_in_data", e.what());
        result.message = "error";
        return result;
    }


    //std::cout << sql_ddl << std::endl;
    *sql << sql_ddl;
    tr.commit();

    sql_ddl = "";

    nlohmann::json where_values = {
            {"device_id", device_id}
    };
    if (!workplace.empty())
        where_values["workplace"] = workplace;

    auto q = builder::query_builder((builder::sql_database_type)sett.SQLFormat);

    q.select(nlohmann::json{
            {"ref", temp_alias + "." + "ref"},
            {"ver1", "max(" + temp_alias + "." + "Ver1)"},
            {"ver2", "max(" + temp_alias + "." + "Ver2)"}
    }).from(builder::query_builder((builder::sql_database_type)sett.SQLFormat).select(nlohmann::json{
                    {"ref",  "ref"},
                    {"ver1", "version"},
                    {"ver2", "-1"}}).from(table_name).where(where_values, true).union_all(
                    builder::query_builder((builder::sql_database_type)sett.SQLFormat).select(nlohmann::json{
                            {"ref",  "ref"},
                            {"ver1", "-1"},
                            {"ver2", "version"}
                    }).from(temp_pref + table_name + "_temp")
            )
    , temp_alias).group_by("ref");

    sql_ddl.append(q.prepare());

    //std::cout << sql_ddl << std::endl;

//        sql_ddl.append("SELECT ref,\n"
//                       "       max(Ver1) AS ver1,\n"
//                       "       max(Ver2) AS ver2\n"
//                       "  FROM (\n"
//                       "           SELECT ref,\n"
//                       "                  version AS Ver1,\n"
//                       "                  -1 AS Ver2\n"
//                       "             FROM Documents\n"
//                       "           UNION ALL\n"
//                       "           SELECT ref,\n"
//                       "                  -1,\n"
//                       "                  version\n"
//                       "             FROM Documents_temp\n"
//                       "       )\n"
//                       " GROUP BY ref;");

    nlohmann::json result_j{};
    //std::cout << sql_ddl << std::endl;
    soci::rowset<soci::row> rs = (sql->prepare << sql_ddl);

    for (auto it = rs.begin(); it != rs.end(); it++) {
        const soci::row& row_ = *it;
        int ver1, ver2;
        std::string ref = builder::query_builder::get_value<std::string>(row_, 0);
        if(sett.SQLFormat == 0){
            std::string ver1_ = builder::query_builder::get_value<std::string>(row_, 1);
            std::string ver2_ = builder::query_builder::get_value<std::string>(row_, 2);
            if(ver1_.empty())
                ver1_ = "0";
            if(ver2_.empty())
                ver2_ = "0";
            ver1 = std::stoi(ver1_) ; //версия сервера
            ver2 = std::stoi(ver2_); //версия клиента
        }else{
            ver1 = builder::query_builder::get_value<int>(row_, 1); //версия сервера
            ver2 = builder::query_builder::get_value<int>(row_, 2);
        }

        if (ver1 > ver2) {
            result_j["objects"] += {
                    {ref, data_synchronization_get_object(table_name, ref)}
            };
        }
        result_j["comparison_table"] += nlohmann::json{
                {"ref",  ref},
                {"ver1", ver1},
                {"ver2", ver2}
        };
    }

    //sql.close();
    *sql << arcirk::str_sample("DROP TABLE %1%", temp_pref + table_name + "_temp");

    result.result = base64::base64_encode(result_j.dump());
    result.message = "OK";

    return result;

}

arcirk::server::server_command_result shared_state::sync_update_data_on_the_server(const variant_t &param,
                                                                                   const variant_t &session_id) {


    using namespace arcirk::database;
    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::SyncUpdateDataOnTheServer);

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    std::string device_id = param_.value("device_id", "");
    std::string table_name = param_.value("table_name", "");
//    std::string workplace = param_.value("workplace", "");
//    std::string parent = param_.value("parent", "");
    std::string base64_param = param_.value("base64_param", ""); //упакованная таблица с локального устройства
    nlohmann::json ext_objects{};

    if (table_name.empty() || device_id.empty())
        throw native_exception("Не достаточно параметров для выполнения запроса!");

    bool operation_available = is_operation_available(uuid, roles::dbUser);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    //auto sql = soci_initialize();

    if (!base64_param.empty()) {
        ext_objects = nlohmann::json::parse(arcirk::base64::base64_decode(base64_param));
        if(ext_objects.is_array()){
            for (auto itr = ext_objects.begin(); itr != ext_objects.end(); ++itr) {
                if(itr->is_object()){
                    nlohmann::json obj = *itr;
                    data_synchronization_set_object(obj["object"], table_name);
                }
            }
        }
    }



    result.result = "success";
    result.message = "OK";

    return result;
}

std::string shared_state::handle_request(const std::string &body, const std::string &basic_auth) {

    using namespace boost::filesystem;
    using namespace soci;

    info("handle_request", "Request from http client");

    auto http_session = std::make_shared<http_client>() ;
    http_session->set_uuid_session(arcirk::uuids::random_uuid());

    arcirk::T_vec v = split(basic_auth, " ");

        if(v.size() == 2){
            const std::string base64 = v[1];
            std::string auth = arcirk::base64::base64_decode(base64);
            arcirk::T_vec m_auth = split(auth, ":");
            std::string hash;
            if(m_auth.size() == 1){
                auto source = v[0];
                arcirk::trim(source);
                arcirk::to_lower(source);
                if( source == "token"){
                    hash =m_auth[0];
                }else
                    return "fail authorization";
            }else if(m_auth.size() == 2)
                hash = arcirk::get_hash(m_auth[0], m_auth[1]);
            else
                return "fail authorization";

            auto sql = soci_initialize();
            soci::rowset<soci::row> rs = (sql->prepare << "select * from Users where hash = " <<  "'" << hash << "'");
            for (auto it = rs.begin(); it != rs.end(); it++) {
                const soci::row &row_ = *it;
                http_session->set_role(row_.get<std::string>("role"));
                http_session->set_user_name(row_.get<std::string>("first"));
                http_session->set_authorized(true);
                http_session->set_app_name("http_client");
            }

            }

    sessions_.insert(std::pair<boost::uuids::uuid, subscriber*>(http_session->uuid_session(), http_session.get()));

//    try {
        auto http_body = nlohmann::json::parse(body);
        std::string command = http_body["command"];
        std::string param = http_body["param"];
        std::string recipient = http_body.value("recipient", "");

        nlohmann::json parameters = {
                {"parameters", param}
        };

        if(recipient.empty())
            execute_command_handler("cmd " + command + " " + arcirk::base64::base64_encode(parameters.dump()), http_session.get());
        else
            execute_command_handler("cmd " + command + " " + recipient + " " + arcirk::base64::base64_encode(parameters.dump()), http_session.get());

//    } catch (std::exception &e) {
//        return std::string("shared_state::verify_auth:error ") + e.what();
//    }

    sessions_.erase(http_session->uuid_session());

    info("handle_request", "close http client");

    auto result = http_session->get_result();

    if(result->empty())
        return "error";

    return result->c_str();
}

//Запуск планировщика задач
void shared_state::run_server_tasks() {

    if(task_manager){
        task_manager->stop();
        task_manager->clear();
    }

    task_manager = std::make_shared<arcirk::services::task_scheduler>();

    using namespace boost::filesystem;

    auto root_conf = app_directory();
    path file_name = "server_tasks.json";
    nlohmann::json result{};

    path conf = root_conf /+ file_name.c_str();

    try {
        if(exists(conf)){
            std::ifstream file(conf.string(), std::ios_base::in);
            std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
            if(!str.empty()){
                result = nlohmann::json::parse(str);
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<arcirk::services::task_options> vec;
    if(result.empty()){
        arcirk::services::task_options opt{};
        opt.end_task = 0;
        opt.start_task = 0;
        opt.interval = 600;
        opt.name = "EraseDeletedMarkObjects";
        opt.uuid = boost::to_string(arcirk::uuids::random_uuid());
        opt.allowed = true;
        opt.synonum = "Очистка помеченных на удаление объектов.";
        vec.push_back(opt);
        arcirk::services::task_options opt1{};
        opt1.end_task = 0;
        opt1.start_task = 0;
        opt1.interval = 60;
        opt1.name = "ExchangePlan";
        opt1.uuid = boost::to_string(arcirk::uuids::random_uuid());
        opt1.allowed = true;
        opt1.synonum = "Обмен по плану обмена.";
        vec.push_back(opt1);
    }else{
        for (auto itr = result.begin(); itr != result.end(); ++itr) {
            auto opt = pre::json::from_json<arcirk::services::task_options>(*itr);
            vec.push_back(opt);
        }
    }
    for (const auto& itr : vec) {
        if(itr.allowed)
            task_manager->add_task(itr, std::bind(&shared_state::exec_server_task, this, std::placeholders::_1));
    }
//    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
//    std::cout << threadId << std::endl;
    // Запуск планировщика задач в отдельном потоке

    auto tr = boost::thread([this](){
//        std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
//        std::cout << threadId << std::endl;
        task_manager->run();
    });

    tr.detach();
}

void shared_state::exec_server_task(const arcirk::services::task_options &details) {

    info("exec_server_task::exec", details.name);
    if(details.name == "EraseDeletedMarkObjects"){
        erase_deleted_mark_objects();
    }else if(details.name == "ExchangePlan"){
        synchronize_objects_from_1c();
    }

}

void shared_state::erase_deleted_mark_objects() {

    using namespace soci;
    using namespace arcirk::database;

    auto sql = soci_initialize();
    auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);
    auto tr = soci::transaction(*sql);
    *sql <<  "delete from DocumentsTables where DocumentsTables.parent in (select Documents.ref from Documents where Documents.deleted_mark = '1');";
    *sql << "delete from Documents where Documents.deleted_mark = '1';";
    tr.commit();
    log("shared_state::erase_deleted_mark_objects", "Регламентная операция успешно завершена!");
}

void shared_state::synchronize_objects_from_1c() {

    auto sh_oper = scheduled_operations(sett);
    try {
        bool result = sh_oper.perform_data_exchange();
        if(result)
            log("shared_state::synchronize_objects_from_1c", "Регламентная операция успешно завершена!");
    } catch (const std::exception &err) {
        fail("shared_state::synchronize_objects_from_1c", err.what());
    }

}

arcirk::server::server_command_result
shared_state::sync_update_barcode(const variant_t &param, const variant_t &session_id) {

    using namespace arcirk::database;
    using namespace soci;

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    server::server_command_result result;
    result.command = enum_synonym(server::server_commands::SyncUpdateBarcode);

    bool operation_available = is_operation_available(uuid, roles::dbUser);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    std::string param_json = base64_to_string(std::get<std::string>(param));
    auto param_ = nlohmann::json::parse(param_json);
    result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

    auto barcodes_j = param_["barcodes"];
    std::vector<std::string> barcodes;
    std::vector<std::string> queryas;

    if(barcodes_j.is_string()){
        barcodes.push_back(barcodes_j.get<std::string>());
    }else if(barcodes_j.is_array()){
        for (auto itr = barcodes_j.begin();  itr != barcodes_j.end() ; ++itr) {
            barcodes.push_back(*itr);
        }
    }

    if(barcodes.empty())
        throw native_exception("Ошибка в параметрах команды!");

    std::string table_name = arcirk::enum_synonym(tables::tbBarcodes);
    auto sql = soci_initialize();
    auto http_service = scheduled_operations(sett);

    std::string script = arcirk::_1c::scripts::get_text(arcirk::_1c::scripts::local_1c_script::barcode_information, sett);
    if(script.empty()){
        throw native_exception("Не найден файл скрипта 1с!");
    }

    nlohmann::json result_for_client{};

    for (const auto br : barcodes) {

        nlohmann::json param_http{
                {"barcode", br},
                {"command", "ExecuteScript"},
                {"script", arcirk::base64::base64_encode(script)},
                {"privileged_mode", true}
        };

        auto http_result = http_service.exec_http_query("ExecuteScript", param_http);

        if(http_result.is_object()){



            int count = 0;
            auto obj = http_result.value("barcode", nlohmann::json{});
            auto stuct_br = pre::json::from_json<database::barcodes>(obj);
            auto stuct_n = pre::json::from_json<database::nomenclature>(http_result["nomenclature"]);

            auto rs = builder::query_builder((builder::sql_database_type)sett.SQLFormat).select().from(table_name).where(nlohmann::json{
                    {"barcode", br}
            }, true).exec(*sql, {}, true);

            for (rowset<row>::const_iterator itr = rs.begin(); itr != rs.end(); ++itr) {
                count++;
                const soci::row &row_ = *itr;
                stuct_br.ref = row_.get<std::string>("ref");
            }

            if(stuct_br.ref.empty())
                stuct_br.ref = arcirk::uuids::uuid_to_string(arcirk::uuids::random_uuid());

            auto query = builder::query_builder((builder::sql_database_type)sett.SQLFormat);

            query.use(pre::json::to_json(stuct_br));
            if(count > 0){
                queryas.push_back(query.update(table_name, true).where(nlohmann::json{
                        {"ref", stuct_br.ref}
                }, true).prepare());
            }
            else{
                queryas.push_back(query.insert(table_name, true).prepare());
            }

            if(stuct_n.ref.empty())
                continue;

            count = 0;
            query.clear();
            table_name = arcirk::enum_synonym(arcirk::database::tables::tbNomenclature);
            rs = builder::query_builder((builder::sql_database_type)sett.SQLFormat).select().from(table_name).where(nlohmann::json{
                    {"ref", stuct_n.ref}
            }, true).exec(*sql, {}, true);

            for (rowset<row>::const_iterator itr = rs.begin(); itr != rs.end(); ++itr) {
                count++;
            }

            query.use(pre::json::to_json(stuct_n));
            if(count > 0){
                queryas.push_back(query.update(table_name, true).where(nlohmann::json{
                        {"ref", stuct_n.ref}
                }, true).prepare());
            }
            else{
                queryas.push_back(query.insert(table_name, true).prepare());
            }

            result_for_client += nlohmann::json {
                    {"barcode", pre::json::to_json(stuct_br)},
                    {"nomenclature", pre::json::to_json(stuct_n)}
            };
        }

    }

    if(!queryas.empty()){
        auto tr = soci::transaction(*sql);
        for (const auto& q : queryas) {
            *sql << q;
        }
        tr.commit();
    }

    result.result = result_for_client.dump();
    result.message = "OK";

    return result;
}

arcirk::server::server_command_result
shared_state::download_app_update_file(const variant_t &param, const variant_t &session_id) {

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    nlohmann::json param_{};
    arcirk::server::server_command_result result;
    try {
        init_default_result(result, uuid, arcirk::server::DownloadAppUpdateFile, arcirk::database::roles::dbAdministrator
                ,param_, param);
    } catch (const std::exception &e) {
        throw native_exception(e.what());
    }

    std::string url_file = param_["file_name"];

    if(url_file.empty())
        throw native_exception("Не верные параметры команды!");

    auto url = arcirk::Uri::Parse(url_file);

    using namespace boost::filesystem;

    path catalog(sett.ServerWorkingDirectory);
    catalog /= sett.Version;
    catalog /= "bin";

    path file(url.Protocol != "file" ? url_file : url.Path);
    auto dest = catalog  /  file.filename();

    if(!exists(catalog)){
        throw native_exception("Не верная структура каталогов на сервере!");
    }

    std::string protocol = url.Protocol;
    if(protocol == "file"){

        if(!exists(file)){
            throw native_exception("Файл не найден!");
        }else{
            if(exists(dest))
                remove(dest);

            copy_file(
                    file,
                    dest
            );
        }
    }else{
        using namespace WebDAV;
        auto dav_custom_param = param_.value("dav_param", nlohmann::json{});
        dict_t dav_param;
        if(!dav_custom_param.empty()){
            dav_param.emplace("webdav_hostname", dav_custom_param["webdav_hostname"]);
            dav_param.emplace("webdav_root", dav_custom_param["webdav_root"]);
            dav_param.emplace("webdav_username", dav_custom_param["webdav_username"]);
            dav_param.emplace("webdav_password", dav_custom_param["webdav_password"]);
        }else{
            dav_param.emplace("webdav_hostname", sett.WebDavHost);
            dav_param.emplace("webdav_root", arcirk::str_sample("/remote.php/dav/files/%1%/", sett.WebDavUser));
            dav_param.emplace("webdav_username", sett.WebDavUser);
            dav_param.emplace("webdav_password", sett.WebDavPwd);
        }

        auto dav = Client(dav_param);
        auto is_exists = dav.check(file.filename().string());

        if(!is_exists)
            throw native_exception("Файл на удаленном ресурсе не найден!");

        if(exists(dest))
            remove(dest);

        //Синхронно копируем файл
        auto res = dav.download(file.filename().string(), dest.string());

        if(!res){
            result.message = "error";
            return result;
        }
    }

    result.message = "OK";
    return result;
}

bool shared_state::init_default_result(arcirk::server::server_command_result &result,
                                       const boost::uuids::uuid &uuid, server::server_commands cmd,
                                       arcirk::database::roles role, nlohmann::json& param, const variant_t& param_) {

    result.command = enum_synonym(cmd);

    bool operation_available = is_operation_available(uuid, role);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    std::string param_json = base64_to_string(std::get<std::string>(param_));
    param = nlohmann::json::parse(param_json);
    result.uuid_form = param.value("uuid_form", arcirk::uuids::nil_string_uuid());
    return true;
}

arcirk::server::server_command_result
        shared_state::get_information_about_file(const variant_t &param, const variant_t &session_id) {

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    nlohmann::json param_{};
    arcirk::server::server_command_result result;
    try {
        init_default_result(result, uuid, arcirk::server::GetInformationAboutFile, arcirk::database::roles::dbUser
                ,param_, param);
    } catch (const std::exception &e) {
        throw native_exception(e.what());
    }

    std::string file_name = param_["file_name"];

    if(file_name.empty())
        throw native_exception("Не верные параметры команды!");

    using namespace boost::filesystem;

    path catalog(sett.ServerWorkingDirectory);
    catalog /= sett.Version;
    catalog /= "bin";
    path file = catalog;
    file /= file_name;

    if(!exists(file)){
        throw native_exception("Файл не найден!");
    }else{
        auto sz = file_size(file);
        auto t = last_write_time(file);
        char cur_date[100];
        std::tm tm = *std::localtime(&t);
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);
        nlohmann::json info{
                {"size", (int)sz},
                {"time", cur_date}
        };
        result.result = info.dump();
    }

    result.message = "OK";
    return result;
}

arcirk::server::server_command_result
        shared_state::check_for_updates(const variant_t &param, const variant_t &session_id) {

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    nlohmann::json param_{};
    arcirk::server::server_command_result result;
    try {
        init_default_result(result, uuid, arcirk::server::CheckForUpdates, arcirk::database::roles::dbUser
                ,param_, param);
    } catch (const std::exception &e) {
        throw native_exception(e.what());
    }

    auto current_version = param_.value("current_version", nlohmann::json{});
    std::string release_dir = param_["release_dir"];
    std::string extension = param_["extension"];

    if(current_version.empty() || release_dir.empty() || extension.empty())
        throw native_exception("Не верные параметры!");

    namespace fs = boost::filesystem;

    fs::path dir = sett.ServerWorkingDirectory;
    dir /= sett.Version;
    dir /= release_dir;

    if(!fs::exists(dir))
        throw native_exception("Каталог не найден!");

    bool version_set = false;

    for(const auto& filename : fs::directory_iterator(dir)){
        if(is_directory(filename))
            continue;
        std::string::size_type index = filename.path().string().find("_v",0);
        if(index != std::string::npos){
            std::string::size_type index_ext = filename.path().string().find("." + extension,0);
            std::string str_version = filename.path().string().substr(index + 2, filename.path().string().length() - index_ext + 1);
            T_vec vec = arcirk::split(str_version, "_");
            if(vec.size() == 3){
                int v_major = std::stoi(vec[0]);
                int v_minor = std::stoi(vec[1]);
                int v_path  = std::stoi(vec[0]);
                bool value_is_greater = false;
                if(v_major > current_version.value("major", 0)){
                    value_is_greater = true;
                }
                if(!value_is_greater && v_minor > current_version.value("minor", 0)){
                    value_is_greater = true;
                }
                if(!value_is_greater && v_path > current_version.value("path", 0)){
                    value_is_greater = true;
                }
                if(value_is_greater){
                    current_version["major"] = v_major;
                    current_version["minor"] = v_minor;
                    current_version["path"] = v_path;
                    version_set = true;
                }
            }
        }
    }

    if(version_set){
        result.result = arcirk::base64::base64_encode(nlohmann::json{
                {"new_version", current_version}
        }.dump());
    }
    result.message = "OK";
    return result;
}

arcirk::server::server_command_result
        shared_state::upload_file(const variant_t &param, const variant_t &session_id) {

    auto uuid = uuids::string_to_uuid(std::get<std::string>(session_id));
    nlohmann::json param_{};
    arcirk::server::server_command_result result;
    try {
        init_default_result(result, uuid, arcirk::server::UploadFile, arcirk::database::roles::dbUser
                ,param_, param);
    } catch (const std::exception &e) {
        throw native_exception(e.what());
    }

    auto file_name = param_.value("file_name", "");

    if(file_name.empty())
        throw native_exception("Ошибка в параметрах!");

    namespace fs = boost::filesystem;
    fs::path file(sett.ServerWorkingDirectory);
    file /= sett.Version;
    file /= file_name;

    if(!fs::exists(file))
        throw native_exception("Файл не найден!");

    if(fs::is_directory(file))
        throw native_exception("Файл является директорией!");

    arcirk::read_file(file.string(), result.data);

    result.result = arcirk::base64::base64_encode(nlohmann::json{
            {"file_name", file.filename().string()}
    }.dump());
    result.message = "OK";
    return result;
}

arcirk::server::server_command_result shared_state::get_database_tables(const variant_t &param,
                                                                        const variant_t &session_id) {

    using namespace arcirk::database;
    using n_json = nlohmann::json;

    boost::uuids::uuid uuid = arcirk::uuids::string_to_uuid(std::get<std::string>(session_id));
    bool operation_available = is_operation_available(uuid, roles::dbAdministrator);
    if (!operation_available)
        throw native_exception("Не достаточно прав доступа!");

    std::string conf_json = pre::json::to_json(sett).dump();
    server::server_command_result result;
    auto sql = soci_initialize();
    auto version = arcirk::server::get_version();
    auto database_tables = arcirk::database::get_database_tables(*sql, arcirk::DatabaseType(sett.SQLFormat), pre::json::to_json(version));
    auto j_res = nlohmann::json(database_tables);
    result.result = arcirk::base64::base64_encode(j_res.dump());
    result.command = enum_synonym(server::server_commands::GetDatabaseTables);
    result.message = "OK";
    try {
        auto param_ = parse_json(std::get<std::string>(param), true);
        result.uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());
    } catch (std::exception &ex) {
        result.uuid_form = uuids::nil_string_uuid();
    }
    return result;

}