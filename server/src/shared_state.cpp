#include <utility>
#include "../include/shared_state.hpp"
#include "../include/websocket_session.hpp"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <algorithm>
#include <codecvt>
#include <cwctype>
#include <locale>
//#include <nlohmann/json.hpp>

shared_state::shared_state(){

    using n_json = nlohmann::json;
    using namespace arcirk::server;

    sett = public_struct::server_settings();
    read_conf(sett);
    //n_json(arcirk::server::ServerPublicCommands::ServerVersion).get<std::string>();
    add_method(arcirk::server::synonym(ServerPublicCommands::ServerVersion), this, &shared_state::server_version);
    add_method(arcirk::server::synonym(ServerPublicCommands::ServerOnlineClientsList), this, &shared_state::get_clients_list);
//    add_method("ServerVersion", this, &shared_state::server_version);
//    add_method("ServerOnlineClientsList", this, &shared_state::get_clients_list);
}

void shared_state::join(subscriber *session) {

    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(std::pair<boost::uuids::uuid, subscriber*>(session->uuid_session(), session));
    std::cout << "client join: " << arcirk::uuids::uuid_to_string(session->uuid_session()) << std::endl;

}

void shared_state::leave(const boost::uuids::uuid& session_uuid, const std::string& user_name) {
    auto iter = sessions_.find(session_uuid);
    if (iter != sessions_.end() ){
        sessions_.erase(session_uuid);
    }
    std::cout << "client leave: " << user_name << " (" << arcirk::uuids::uuid_to_string(session_uuid) << ")" << std::endl;
}

void shared_state::deliver(const std::string &message, subscriber *session) {

    std::string result = message;

    if (result == "\n")
        return;

    if (result == "ping")
        result = "pong";

    if(use_authorization()){
        if(!session->authorized())
            return fail("shared_state::deliver", "Пользователь не авторизован! Команда отменена");
    }

    if(!is_cmd(message)){
        if(!session->is_ssl())
            send<plain_websocket_session>(result);
        else
            send<ssl_websocket_session>(result);
    }else{
        execute_command_handler(message, session);
    }
}

void shared_state::execute_command_handler(const std::string& message, subscriber *session) {

    arcirk::T_vec v = split(message, " ");

    if(v.size() < 2){
        fail("shared_state::execute_command_handler", "Не верный формат команды!");
        std::cout << message << std::endl;
        return;
    }

    long command_index = find_method(v[1]);
    if(command_index < 0){
        fail("shared_state::execute_command_handler", arcirk::str_sample("Команда (%1%) не найдена!", v[1]));
        std::cout << message << std::endl;
        return;
    }

    std::string json_params = "";
    if(v.size() > 2){
        try {
            json_params = arcirk::base64::base64_decode(v[2]);
        } catch (std::exception &e) {
            fail("shared_state::execute_command_handler:parse_params", e.what());
            return;
        }
    }

    std::vector<variant_t> params_v;
    if(!json_params.empty()){
        using nlohmann_json = nlohmann::json;
        auto params_ = nlohmann_json::parse(json_params);
        for (auto& el : params_.items()) {
            std::cout << el.key() << " : " << el.value() << "\n";
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
    variant_t return_value;
    call_as_func(command_index, &return_value, params_v);

    std::string result = std::get<std::string>(return_value);
    if(session){
        if(!result.empty()){
            if(!session->is_ssl())
                send<plain_websocket_session>(result);
            else
                send<ssl_websocket_session>(result);
        }
    }
}



bool shared_state::use_authorization() const{
    return sett.UseAuthorization;
}

template<typename T>
void shared_state::send(const std::string &message) {

    std::vector<std::weak_ptr<T>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            v.emplace_back(p.second->template derived<T>().weak_from_this());
    }
    for(auto const& wp : v)
        if(auto sp = wp.lock()){
            auto const ss = boost::make_shared<std::string const>(message);
            sp->send(ss);
        }

}

bool shared_state::verify_connection(const std::string &basic_auth) {

    std::cout << "verify_connection: " << basic_auth << std::endl;

    if(basic_auth.empty())
        return false;
    else{
        arcirk::T_vec v = split(basic_auth, " ");
        try {
            if(v.size() == 2){
                const std::string base64 = v[1];
                std::string auth = arcirk::base64::base64_decode(base64);
                arcirk::T_vec m_auth = split(auth, ":");
                return verify_auth(m_auth[0], m_auth[1]);
            }
        } catch (std::exception &e) {
            std::cerr << "shared_state::verify_connection:error " << e.what() << std::endl;
        }
    }

    return false;
}

bool shared_state::verify_auth(const std::string& usr, const std::string& pwd) const {

    using namespace boost::filesystem;
    using namespace soci;

    std::string hash = arcirk::get_hash(usr, pwd);
    if(sett.SQLFormat == DatabaseType::dbTypeSQLite){
        if(sett.ServerWorkingDirectory.empty())
        {
            std::cerr << "shared_state::verify_auth:error errors in server settings" << std::endl;
            return false;
        }

        path database(sett.ServerWorkingDirectory);
        database /= sett.Version;
        database /= "data";
        database /= "arcirk.sqlite";

        if(!exists(database)){
            std::cerr << "shared_state::verify_auth:error database file not found" << std::endl;
            return false;
        }

        try {
            std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", database.string());
            session sql(soci::sqlite3, connection_string);
            int count = -1;
            sql << "select count(*) from Users where hash = " <<  "'" << hash << "'" , into(count);
            return count > 0;
        } catch (std::exception &e) {
            std::cerr << "shared_state::verify_auth:error " << e.what() << std::endl;
        }

    }
    return false;
}

std::string shared_state::get_clients_list(const variant_t& msg, variant_t& custom_result, variant_t& error){
    //
    return "test get_clients_list";
}

std::string shared_state::server_version(){
    return sett.Version;
}

bool shared_state::call_as_proc(const long method_num, std::vector<variant_t> params) {

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

bool shared_state::call_as_func(const long method_num, variant_t *ret_value, std::vector<variant_t> params) {

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

void shared_state::command_to_server(ServerResponse& resp) {

}

//std::vector<variant_t> shared_state::parseParams(tVariant *params, long array_size) {
//    std::vector<variant_t> result;
//
//    auto size = static_cast<const unsigned long>(array_size);
//    result.reserve(size);
//    for (size_t i = 0; i < size; i++) {
//        result.emplace_back(toStlVariant(params[i]));
//    }
//
//    return result;
//}

//variant_t shared_state::toStlVariant(tVariant src) {
//    switch (src.vt) {
//        case VTYPE_EMPTY:
//            return UNDEFINED;
//        case VTYPE_I4: //int32_t
//            return src.lVal;
//        case VTYPE_R8: //double
//            return src.dblVal;
//        case VTYPE_PWSTR: { //std::string
//            return toUTF8String(std::basic_string_view(src.pwstrVal, src.wstrLen));
//        }
//        case VTYPE_BOOL:
//            return src.bVal;
//        case VTYPE_BLOB:
//            return std::vector<char>(src.pstrVal, src.pstrVal + src.strLen);
//        case VTYPE_TM:
//            return src.tmVal;
//        default:
//            throw std::bad_cast();
//    }
//}
//
//std::string shared_state::toUTF8String(std::basic_string_view<WCHAR_T> src) {
//#ifdef _WINDOWS
//    // VS bug
//    // https://social.msdn.microsoft.com/Forums/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
//    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt_utf8_utf16;
//    return cvt_utf8_utf16.to_bytes(src.data(), src.data() + src.size());
//#else
//    static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> cvt_utf8_utf16;
//    return cvt_utf8_utf16.to_bytes(reinterpret_cast<const char16_t *>(src.data()),
//                                   reinterpret_cast<const char16_t *>(src.data() + src.size()));
//#endif
//}