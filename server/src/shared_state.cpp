#include <utility>
#include "../include/shared_state.hpp"
#include "../include/websocket_session.hpp"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <algorithm>
#include <locale>

shared_state::shared_state(){

    using namespace arcirk::server;

    sett = server::server_config();
    read_conf(sett);
    add_method(arcirk::server::synonym(server::server_commands::ServerVersion), this, &shared_state::server_version);
    add_method(arcirk::server::synonym(server::server_commands::ServerOnlineClientsList), this, &shared_state::get_clients_list);
    add_method(arcirk::server::synonym(server::server_commands::SetClientParam), this, &shared_state::set_client_param);
}

void shared_state::join(subscriber *session) {

    sessions_.insert(std::pair<boost::uuids::uuid, subscriber*>(session->uuid_session(), session));
    std::tm tm = arcirk::current_date();
    char cur_date[100];
    std::strftime(cur_date, sizeof(cur_date), "%A %c", &tm);
    log("shared_state::join", "client join: " + arcirk::uuids::uuid_to_string(session->uuid_session()) + " " + std::string(cur_date));

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
    std::string response =  to_string(pre::json::to_json(resp));

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
    std::tm tm = arcirk::current_date();
    char cur_date[100];
    std::strftime(cur_date, sizeof(cur_date), "%A %c", &tm);
    log("shared_state::leave", "client leave: " + user_name + " (" + arcirk::uuids::uuid_to_string(session_uuid) + ")" + " " + std::string(cur_date));

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
    resp.message = msg;
    resp.param = param;
    resp.result = "OK";
    resp.sender = arcirk::uuids::uuid_to_string(session->uuid_session());
    resp.receiver = receiver;
    resp.app_name = session->app_name();

    std::string response =  to_string(pre::json::to_json(resp));

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

    if(sett.ResponseTransferToBase64){
        response = arcirk::base64::base64_encode(response);
    }

    auto const ss = boost::make_shared<std::string const>(response);
    itr->second->send(ss);

}

void shared_state::execute_command_handler(const std::string& message, subscriber *session) {

    arcirk::T_vec v = split(message, " ");

    if(v.size() < 2){
        fail("shared_state::execute_command_handler:error", "Не верный формат команды!");
        //std::cout << message << std::endl;
        return;
    }

    long command_index = find_method(v[1]);
    if(command_index < 0){
        fail("shared_state::execute_command_handler:error", arcirk::str_sample("Команда (%1%) не найдена!", v[1]));
       // std::cout << message << std::endl;
        return;
    }

    std::string json_params;
    if(v.size() > 2){
        try {
            json_params = arcirk::base64::base64_decode(v[2]);
        } catch (std::exception &e) {
            fail("shared_state::execute_command_handler:parse_params:error", e.what(), false);
            return;
        }
    }

    std::vector<variant_t> params_v;
    if(!json_params.empty()){
        using nlohmann_json = nlohmann::json;
        auto params_ = nlohmann_json::parse(json_params);
        for (auto& el : params_.items()) {
            //std::cout << el.key() << " : " << el.value() << "\n";
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

    long p_count = param_count(command_index);
    if(p_count != params_v.size())
        return fail("shared_state::execute_command_handler", "Не верное количество аргументов!");

    log("shared_state::execute_command_handler", get_method_name(command_index));

    arcirk::server::server_command_result return_value;
    call_as_func(command_index, &return_value, params_v);

    using namespace arcirk::server;
    std::string response;
    server::server_response result_response;
    result_response.command = get_method_name(command_index);
    result_response.message = return_value.result != "error" ? "OK" : "error";
    result_response.result = return_value.result; //base64 string
    result_response.sender = arcirk::uuids::uuid_to_string(session->uuid_session());
    result_response.receiver = arcirk::uuids::uuid_to_string(session->uuid_session());
    result_response.uuid_form = return_value.uuid_form;
    result_response.app_name = session->app_name();

    response = to_string(pre::json::to_json(result_response));

    if(sett.ResponseTransferToBase64){
        response = arcirk::base64::base64_encode(response);
    }

    auto const ss = boost::make_shared<std::string const>(response);
    session->send(ss);

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

    //std::cout << "verify_connection: " << basic_auth << std::endl;


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
bool shared_state::verify_auth_from_hash(const std::string &usr, const std::string &hash) const {

    std::cout << "verify_connection ... " << std::endl;

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
            sql << "select count(*) from Users where hash = " <<  "'" << hash << "'" , into(count);
            return count > 0;
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

//    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
//        if(sett.ServerWorkingDirectory.empty())
//        {
//            fail("shared_state::verify_auth:error", "Ошибки в параметрах сервера!");
//            return false;
//        }
//
//        path database(sett.ServerWorkingDirectory);
//        database /= sett.Version;
//        database /= "data";
//        database /= "arcirk.sqlite";
//
//        if(!exists(database)){
//            fail("shared_state::verify_auth:error", "Файл базы данных не найден!");
//            return false;
//        }
//
//        try {
//            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
//            session sql(soci::sqlite3, connection_string);
//            int count = -1;
//            sql << "select count(*) from Users where hash = " <<  "'" << hash << "'" , into(count);
//            return count > 0;
//        } catch (std::exception &e) {
//            fail("shared_state::verify_auth:error", e.what(), false);
//        }
//
//    }
//    return false;
}



arcirk::server::server_command_result shared_state::get_clients_list(const variant_t& param, const variant_t& session_id){

    using n_json = nlohmann::json;

    std::string base64 = std::get<std::string>(param);
    std::string json_param;

    using namespace arcirk::server;
    server::server_command_result result;
    result.command = arcirk::server::synonym(server::server_commands::ServerOnlineClientsList);

    try {
        json_param = arcirk::base64::base64_decode(base64);
    } catch (std::exception &e) {
        fail("hared_state::get_clients_list", e.what(), false);
        result.result = "error";
        return result;
    }
    auto param_ = n_json::parse(json_param, nullptr, false);
    if(param_.is_discarded()){
        fail("shared_state::get_clients_list", "Не верные параметры!");
        result.result = "error";
        return result;
    }

    bool is_table = param_.value("table", false);

    auto table_object = n_json::object();

    if(is_table) {
        auto j = n_json(R"({"session_uuid", "user_name", "user_uuid"})");
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
            {"app_name", itr->second->app_name()}
        };
        rows += row;
    }
    table_object["rows"] = rows;

    std::string table = arcirk::base64::base64_encode(table_object.dump());
    std::string  uuid_form = param_.value("uuid_form", arcirk::uuids::nil_string_uuid());

    result.uuid_form = uuid_form;
    result.result = table;
    return result; //arcirk::base64::base64_encode(to_string(pre::json::to_json(result)));

}

arcirk::server::server_command_result shared_state::server_version(const variant_t& session_id){
    using namespace arcirk::server;
    server::server_command_result result;
    result.command = arcirk::server::synonym(server::server_commands::ServerVersion);
    result.result = sett.Version;
    return result;
}

bool shared_state::call_as_proc(const long& method_num, std::vector<variant_t> params) {

    try {
        //auto args = parseParams(params, array_size);
        //methods_meta[method_num].call(args);
        methods_meta[method_num].call(params);
//#ifdef OUT_PARAMS
//        storeParams(args, params);
//#endif
    } catch (const std::exception &e) {
        //AddError(ADDIN_E_FAIL, extensionName(), e.what(), true);
        return false;
    } catch (...) {
        //AddError(ADDIN_E_FAIL, extensionName(), UNKNOWN_EXCP, true);
        return false;
    }

    return true;
}
//bool shared_state::call_as_func(const long& method_num, variant_t *ret_value, std::vector<variant_t> params) {
bool shared_state::call_as_func(const long& method_num, arcirk::server::server_command_result *ret_value, std::vector<variant_t> params) {

    try {
        //auto args = parseParams(params, array_size);
        //variant_t result = methods_meta[method_num].call(args);
        //storeVariable(result, *ret_value);
        *ret_value = methods_meta[method_num].call(params);
//#ifdef OUT_PARAMS
//        storeParams(args, params);
//#endif
    } catch (const std::exception &e) {
        //AddError(ADDIN_E_FAIL, extensionName(), e.what(), true);
        return false;
    } catch (...) {
        //AddError(ADDIN_E_FAIL, extensionName(), UNKNOWN_EXCP, true);
        return false;
    }

    return true;

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

//std::string shared_state::set_client_param(const variant_t &param, const variant_t& session_id) {
arcirk::server::server_command_result shared_state::set_client_param(const variant_t &param, const variant_t& session_id) {

    auto uuid_ = uuids::string_to_uuid(std::get<std::string>(session_id));
    auto session = get_session(uuid_);

    using namespace arcirk::server;
    server::server_command_result result;
    result.command = arcirk::server::synonym(server::server_commands::SetClientParam);

    if(!session){
        result.result = "error";
        return result;
    }

   // std::string result_;
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
        result.result = arcirk::base64::base64_encode(to_string(pre::json::to_json(param_)) );
        session->set_app_name(param_.app_name);
        if(use_authorization() && !session->authorized()){
            if(!param_.hash.empty()){
                bool result_auth = verify_auth_from_hash(param_.user_name, param_.hash);
                if(!result_auth)
                    fail("shared_state::set_client_param", "failed authorization");
                else{
                    log("shared_state::set_client_param", "successful authorization");
                    session->set_authorized(true);
                }

            }

        }
    } catch (std::exception &e) {
        fail("shared_state::set_client_param:error", e.what(), false);
        result.result = "error";
    }

    return result;//arcirk::base64::base64_encode(result);
}

subscriber*
shared_state::get_session(boost::uuids::uuid &uuid) {
    auto iter = sessions_.find(uuid);
    if (iter != sessions_.end() ){
        return iter->second;
    }
    return nullptr;
}

//получение всех сессий пользователя
std::vector<subscriber *>
shared_state::get_sessions(boost::uuids::uuid &user_uuid) {
    auto iter = user_sessions.find(user_uuid);
    if (iter != user_sessions.end() ){
        return iter->second;
    }
    return {};
}

std::string shared_state::base64_to_string(const std::string &base64str) const {
    try {
        return arcirk::base64::base64_decode(base64str);
    } catch (std::exception &e) {
        return base64str;
    }
}

bool shared_state::allow_delayed_authorization() const {
    return sett.AllowDelayedAuthorization;
}
