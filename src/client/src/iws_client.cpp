#include "../include/iws_client.h"
#ifdef _WINDOWS
    #pragma warning(disable:4061)
    #pragma warning(disable:4001)
    #pragma warning(disable:4100)
    #include  <SDKDDKVer.h>
    #include "../include/net.h"
    #include "../include/ws_client.h"
#else
#include "../include/ws_client.h"
#endif // _WINDOWS

#ifdef _WINDOWS
#include <iostream>
#include <thread>
#else
#include <boost/thread/thread.hpp>
#endif // _WINDOWS

IClient::IClient(_callback_message& callback)
{
    host = "localhost";
    port = 8080;
    callback_msg = callback;
    client = nullptr;
    app_name = "admin_console";
    user_uuid = nil_string_uuid();
    _exitParent = false;
    _isRun = false;
    machineUniqueId = "";
    product = "";
    m_no_notify = false;
    m_document_name = "";
}

IClient::IClient(const std::string& _host, const int& _port, _callback_message& callback)
{
    host = _host;
    port = _port;
    callback_msg = callback;
    client = nullptr;
    app_name = "admin_console";
    user_uuid = nil_string_uuid();
    _exitParent = false;
    _isRun = false;
    machineUniqueId = "";
    product = "";
    m_no_notify = false;
    m_document_name = "";
}
IClient::IClient(const std::string &_host, const int &_port, _callback_message &callback,
                 _callback_status &_status_changed_fun) {
    host = _host;
    port = _port;
    callback_msg = callback;
    client = nullptr;
    app_name = "admin_console";
    user_uuid = nil_string_uuid();
    _status_changed = _status_changed_fun;
    _exitParent = false;
    _isRun = false;
    machineUniqueId = "";
    product = "";
    m_no_notify = false;
    m_document_name = "";
}

void IClient::send_command(const std::string &cmd, const std::string &uuid_form, const std::string &param) {

    if (client){

        if (!started())
            return;

        const std::string& _cmd = cmd;
        std::string _uuid_form = uuid_form;
        std::string _param = param;

        if (_cmd.empty())
            return;
        if (_uuid_form.empty())
            _uuid_form = "00000000-0000-0000-0000-000000000000";

        if (_param.empty())
            _param = "{\"command\":\"" + _cmd + "\"}";

        client->send(_param, true, "00000000-0000-0000-0000-000000000000", _uuid_form, _cmd);
    }

}

void IClient::set_job(const std::string &job, const std::string &job_desc) {
    m_job = job;
    m_job_description = job_desc;

}

void IClient::ext_message(const std::string& msg) {

    if(callback_msg){
        callback_msg(msg);
    }

}

void IClient::on_connect()
{
    _isRun = true;
}

void IClient::close(bool exitParent) {

    _exitParent = exitParent;
    if (client)
    {
        if (started())
        {
            client->close(exitParent);
        }

        delete client;
        client = nullptr;
    }

    _isRun = false;
}

void IClient::start() {

    boost::asio::io_context ioc;

    close();

    _isRun = false;

//    if(client){
//        delete client;
//        client = nullptr;
//    }

    _callback_connect callback = std::bind(&IClient::on_connect, this);

    client = new ws_client(ioc, _client_param);

    try {
        client->open(host.c_str(), std::to_string(port).c_str(), callback_msg, _status_changed, callback);
    }
    catch (std::exception& e){
        std::cerr << "IClient::start::exception: " << e.what() <<std::endl;
    }

    std::cout << "IClient::start: exit client thread" << std::endl;


    if(client && !_exitParent){
        if(callback_msg){
            callback_msg("exit_thread");
        }        
        delete client;
        client = nullptr;
    }

    _isRun = false;

}

bool IClient::started() {


    if(!_isRun)
        return false;

    bool result = false;

    if (client){
        result = client->started();
    }

    return result;
}

tm IClient::currentDate() {
    using namespace std;
    tm current{};
    time_t t = time(nullptr);
#ifdef _WINDOWS
    localtime_s(&current, &t);
#else
    localtime_r(&t, &current);
#endif
    return current;
}

long int IClient::current_date_in_seconds() {
    return current_date_seconds();
}

long int IClient::get_tz_offset() {
    return tz_offset();
}

void IClient::get_messages(const std::string &uuid_sub, const long int &start_date, const long int &end_date, int &limit, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("recipient", uuid_sub);
        pt.add("start_date", (int)start_date);
        pt.add("end_date", (int)end_date);
        pt.add("limit", limit);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("get_messages", uuid_form, _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }

}

void IClient::get_user_info(const std::string &_user_uuid, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("uuid_user", _user_uuid);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("get_user_info", uuid_form, _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::get_group_list(const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        send_command("get_group_list", uuid_form, "");

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::add_group(const std::string &name, const std::string &presentation, const std::string &uuid_parent, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("name", name);
        pt.add("presentation", presentation);
        pt.add("parent", uuid_parent);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("add_group", uuid_form, _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::edit_group(const std::string &uuid_group, const std::string &name, const std::string &presentation,
                        const std::string &uuid_parent, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("name", name);
        pt.add("presentation", presentation);
        pt.add("parent", uuid_parent);
        pt.add("ref", uuid_group);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("edit_group", uuid_form, _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::remove_group(const std::string &uuid_group, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {
        pt.add("ref", uuid_group);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("remove_group", uuid_form, _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }

}

void IClient::get_users(const std::string &uuid_group, const std::string &uuid_form) {
    boost::property_tree::ptree pt;

    try {

        pt.add("channel", uuid_group);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("get_users", uuid_form, _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::set_parent(const std::string &_user_uuid, const std::string &uuid_group, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("parent", uuid_group);
        pt.add("user", _user_uuid);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("set_parent", uuid_form, _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::remove_user(const std::string &_user_uuid, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("ref", _user_uuid);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("remove_user", uuid_form, _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::kill_session(const std::string &_user_uuid, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        pt.add("ref", _user_uuid);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("kill_session", uuid_form, _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::open(bool new_thread){

    app_uuid = random_uuid();
    user_uuid = random_uuid();

    //boost::property_tree::ptree pt; //не шарит в типах данных

    auto pt = arcirk::bJson();
    pt.set_object();

    pt.addMember("uuid", app_uuid);
    pt.addMember("name", admin_name);
    pt.addMember("hash", hash);
    pt.addMember("app_name", app_name);
    pt.addMember("user_uuid", user_uuid);
    pt.addMember("user_name", user_name);
    pt.addMember("host_name", boost::asio::ip::host_name());
    pt.addMember("device_id", machineUniqueId);
    pt.addMember("product", product);
    pt.addMember("no_notify", m_no_notify);
    pt.addMember("notify_apps", m_notify_apps);
    pt.addMember("document_name", m_document_name);
    pt.addMember("job", m_job);
    pt.addMember("job_description", m_job_description);

    _client_param = pt.to_string(); // _ss.str();

    if (new_thread){
    #ifdef _WINDOWS
        std::thread(std::bind(&IClient::start, this)).detach();
    #else
        boost::thread(boost::bind(&IClient::start, this)).detach();
    #endif
    }else
        start();

}

std::string IClient::get_hash(const std::string &usr, const std::string &pwd) {
    return arcirk::get_hash(usr, pwd);
}

std::string IClient::get_app_uuid() const {
    if (client)
        return arcirk::uuid_to_string(client->get_uuid());
    else
        return app_uuid;
}

void IClient::set_app_name(const std::string &session_uuid, const std::string &new_app_name) {

    boost::property_tree::ptree pt;

    try {

        pt.add("uuid_set", session_uuid);
        pt.add("new_app_name", new_app_name);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("set_app_name", nil_string_uuid(), _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::get_users_catalog(const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        std::string _uuid_form = uuid_form;

        if (uuid_form.empty()){
            _uuid_form = nil_string_uuid();
        }
        pt.add("uuid_form", _uuid_form);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("get_users_catalog", nil_string_uuid(), _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

std::string IClient::get_user_uuid() const {

    if(client)
        return uuid_to_string(client->get_user_uuid());
    else
        return IClient::nil_string_uuid();
}

void IClient::get_user_cache(const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        std::string _uuid_form = uuid_form;

        if (uuid_form.empty()){
            _uuid_form = nil_string_uuid();
        }
        pt.add("uuid_form", _uuid_form);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("get_user_cache", nil_string_uuid(), _ss.str());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::set_user_cache(const std::string &cache, const std::string &uuid_form) {

    boost::property_tree::ptree pt;

    try {

        std::string _uuid_form = uuid_form;

        if (uuid_form.empty()){
            _uuid_form = nil_string_uuid();
        }
        pt.add("uuid_form", _uuid_form);
        pt.add("cache", arcirk::base64_encode(cache));

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("set_user_cache", nil_string_uuid(), _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::send(const std::string &msg, const std::string &sub_user_uuid, const std::string &uuid_form, const std::string& objectName, const std::string& msg_ref) {

    std::string _msg = msg;
    std::string _uuid_form = uuid_form;
    std::string _sub_user_uuid = sub_user_uuid;

    if (_msg.empty())
        return;
    if (_uuid_form.empty())
        _uuid_form = "00000000-0000-0000-0000-000000000000";
    if (_sub_user_uuid.empty())
        _sub_user_uuid = "00000000-0000-0000-0000-000000000000";

    if (client)
    {
        if (started())
        {
            client->send(_msg, false, _sub_user_uuid, _uuid_form, "message", objectName, msg_ref);
        }
    }
}

void IClient::get_user_status(const std::string &_user_uuid, const std::string &uuid_form, const std::string &param) {

    try {
        auto json = arcirk::bJson();
        if(param.empty()){
            json.SetObject();
            json.addMember("uuid_user", _user_uuid);
        }
        else{
            json.parse(param);
            if (json.is_parse()){
                bVariant var;
                if (!json.getMember("uuid_user", var))
                    json.addMember("uuid_user", _user_uuid);
            }
        }

        send_command("get_user_status", uuid_form, json.to_string());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}


void IClient::get_user_data(const std::string &_user_uuid, const std::string &uuid_form, const std::string &param) {

    try {
        auto json = arcirk::bJson();
        if(param.empty()){
            json.SetObject();
            json.addMember("uuid_user", _user_uuid);
        }
        else{
            json.parse(param);
            if (json.is_parse()){
                bVariant var;
                if (!json.getMember("uuid_user", var))
                    json.addMember("uuid_user", _user_uuid);
            }
        }

        send_command("get_user_data", uuid_form, json.to_string());

    }catch (std::exception&){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::reset_unread_messages(const std::string &user_sender, const std::string &uuid_form) {
    try {

        auto json = arcirk::bJson();
        json.SetObject();
        json.addMember("sender", user_sender);

        send_command("reset_unread_messages", uuid_form, json.to_string());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

std::string IClient::base64_encode(const std::string &s) {
    return  arcirk::base64_encode(s);
}

std::string IClient::base64_decode(const std::string &s) {
    return arcirk::base64_decode(s);
}

std::string IClient::nil_string_uuid() {
    return arcirk::nil_string_uuid();
}

std::string IClient::random_uuid() {
    return arcirk::random_uuid();
}

long int IClient::current_date_seconds() {
    return arcirk::current_date_seconds();
}

long int IClient::add_day(const long selDate, const int dayCount) {
    return arcirk::add_day(selDate, dayCount);
}

std::string IClient::crypt(const std::string &source, const std::string &key)
{
    return arcirk::_crypt(source, key);
}

void IClient::set_webdav_settings_on_client(const std::string& param) {
    if(client)
        client->set_webdav_settings_on_client(param);

}

std::string IClient::get_webdav_user() const {
    if(client)
        return client->get_webdav_user();
    else
        return "";
}

std::string IClient::get_webdav_pwd() const {
    if(client)
        return client->get_webdav_pwd();
    else
        return "";
}

std::string IClient::get_webdav_host() const {
    if(client)
        return client->get_webdav_host();
    else
        return "";
}

bool IClient::get_webdav_ssl() {
    if(client)
        return client->get_webdav_ssl();
    else
        return false;
}

void IClient::set_webdav_settings_on_server() {

    try {
        arcirk::bJson json{};
        json.set_object();
        json.addMember(arcirk::content_value("uuid_form", arcirk::nil_string_uuid()));
        json.addMember(arcirk::content_value(bConf::get_field_alias(bConfFields::WebDavHost), get_webdav_host()));
        json.addMember(arcirk::content_value(bConf::get_field_alias(bConfFields::WebDavUser), get_webdav_user()));
        json.addMember(arcirk::content_value(bConf::get_field_alias(bConfFields::WebDavPwd), get_webdav_pwd()));
        json.addMember(arcirk::content_value(bConf::get_field_alias(bConfFields::WebDavSSL), get_webdav_ssl()));

        send_command("set_webdav_settings", nil_string_uuid(), json.to_string());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }

}

void IClient::get_channel_token(const std::string &user_sender, const std::string &uuid_form) {
    boost::property_tree::ptree pt;

    try {
        pt.add("uuid_form", uuid_form);
        pt.add("user_uuid", get_user_uuid());
        pt.add("recipient_uuid", user_sender);

        std::stringstream _ss;
        boost::property_tree::json_parser::write_json(_ss, pt);

        send_command("get_channel_token", uuid_form, _ss.str());

    }catch (std::exception& e){
        //message("error: " + std::string (e.what()));
    }
}

void IClient::set_machineUniqueId(const std::string &value)
{
    machineUniqueId = value;
}

void IClient::set_product(const std::string &value)
{
    product = value;
}

std::string IClient::get_string_random_uuid() {
    return arcirk::random_uuid();
}

std::string IClient::get_parent_path() {
    return arcirk::bConf::parent_path();
}

void IClient::disable_notify(bool value) {
    m_no_notify = value;
}

void IClient::set_notify_apps(const std::string &value) {
    m_notify_apps = value;
}

void IClient::set_document_name(const std::string &value) {
    m_document_name = value;
}
void IClient::command_to_client(const std::string &recipient, const std::string &command, const std::string &param, const std::string &uuid_form) {

    if(!client->started())
        return;

    auto params = arcirk::bJson();
    params.set_object();
    params.addMember("command", command);
    params.addMember("param",  arcirk::base64_encode(param));
    _Value p = params.to_object();

    auto obj = arcirk::bJson();
    obj.set_object();
    obj.addMember("uuid_agent", recipient);
    obj.addMember("uuid_client", client->get_uuid());
    obj.addMember("param", p);


    std::string arg = obj.to_string();
    client->send_command("command_to_client", uuid_form, arg);

}

void IClient::command_to_server(const std::string &command, const std::string &param, const std::string& uuid_form) {
    if(!client->started())
        return;
    client->send_command(command, uuid_form, param);
}
