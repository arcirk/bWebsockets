//
// Created by admin on 10.09.2022.
//
#include <iostream>
#include <arcirk.hpp>
#include <client.hpp>
#include <functional>
#include <boost/beast/ssl.hpp>

const std::string version = "1.1.0";

boost::filesystem::path m_root_conf;

namespace ssl = boost::asio::ssl;


void verify_directories(){
    using namespace boost::filesystem;

    std::string arcirk_dir = "arcirk";
#ifndef _WINDOWS
    arcirk_dir = "." + arcirk_dir;
#endif

    path app_conf(arcirk::standard_paths::this_application_conf_dir(arcirk_dir));
    app_conf /= version;

    bool is_conf = arcirk::standard_paths::verify_directory(app_conf);

    m_root_conf = app_conf;

    if(!is_conf){
        std::cerr << arcirk::local_8bit("Ошибка верификации директории приложения!") << std::endl;
    }
}

void read_conf(server::server_config & result){

    using namespace boost::filesystem;

    if(!exists(arcirk::local_8bit(m_root_conf.string())))
        return;

    path conf = m_root_conf /+ "client_conf.json";

    if(exists(conf)){
        std::ifstream file(conf.string(), std::ios_base::in);
        std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        if(!str.empty()){
            result = pre::json::from_json<server::server_config>(str);
        }
    }
}

void write_conf(server::server_config & conf){
    using namespace boost::filesystem;

    if(!exists(arcirk::local_8bit(m_root_conf.string())))
        return;
    try {
        std::string result = to_string(pre::json::to_json(conf)) ;
        std::ofstream out;
        path conf_file = m_root_conf /+ "client_conf.json";
        out.open(arcirk::local_8bit(conf_file.string()));
        if(out.is_open()){
            out << result;
            out.close();
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

void on_connect(){
    std::cout << "websocket on_connect" << std::endl;
}

void on_message(const std::string& message){
    std::cout << "websocket on_message: " << message << std::endl;
}

void on_stop(){
    std::cout << "websocket on_stop" << std::endl;
}
void
on_error(const std::string &what, const std::string &err, int code){
    std::cerr << "websocket on_error: " <<  what << "(" << code << "): " << arcirk::local_8bit(err) << std::endl;
}

void on_status_changed(bool status){
    std::cout << "websocket on_status_changed: " << status << std::endl;
}

class InputParser{
public:
    InputParser (int &argc, char **argv){
        for (int i=1; i < argc; ++i)
            this->tokens.emplace_back(argv[i]);
    }
    /// @author iain
    [[nodiscard]] const std::string& getCmdOption(const std::string &option) const{
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()){
            return *itr;
        }
        static const std::string empty_string;
        return empty_string;
    }
    /// @author iain
    [[nodiscard]] bool cmdOptionExists(const std::string &option) const{
        return std::find(this->tokens.begin(), this->tokens.end(), option)
               != this->tokens.end();
    }
private:
    std::vector <std::string> tokens{};
};

void get_online_users(const std::shared_ptr<websocket_client>& client){

    using json_nl = nlohmann::json;

    json_nl param = {
            {"table", true}
    };

    std::string alias = json_nl(arcirk::server::server_commands::ServerOnlineClientsList).get<std::string>();
    std::string param_ = param.dump();
    client->send_command(alias, param_);
}

int
main(int argc, char* argv[]){

    setlocale(LC_ALL, "Russian");

    InputParser input(argc, argv);

    if(input.cmdOptionExists("-v") || input.cmdOptionExists("-version")){
        std::cout << arcirk::local_8bit("arcirk.websocket.server v.") << version << std::endl;
        return EXIT_SUCCESS;
    }

    const std::string &_url = input.getCmdOption("-url");
    const std::string &_usr = input.getCmdOption("-usr");
    const std::string &_pwd = input.getCmdOption("-pwd");


    callback_connect _connect = std::bind(&on_connect);
    callback_error _err = std::bind(&on_error, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    callback_message _message = std::bind(&on_message, std::placeholders::_1);
    callback_status _status = std::bind(&on_status_changed, std::placeholders::_1);
    callback_close _on_close = std::bind(&on_stop);

    verify_directories();

    server::server_config app_conf;
    read_conf(app_conf);

    std::string host = app_conf.ServerHost;
    int port = app_conf.ServerPort;

    ssl::context ctx{ssl::context::tlsv12_client};

    std::shared_ptr<websocket_client> m_client;

    client::client_param client_param = client::client_param();
    client_param.app_name = "websocket_client";
    client_param.user_name = "admin";
    client_param.password = "admin";

    m_client = std::make_shared<websocket_client>(ctx, client_param);

    std::string url = _url;

    if(url.empty()){
        if(host.empty())
            host = arcirk::bIp::get_default_host("0.0.0.0", "192.168.10");

        if(port <= 0)
            port = 8080;
        url = "wss://" + host + ":" + boost::to_string(port);
    }

    if(!_usr.empty())
        client_param.user_name = _usr;
    if(!_pwd.empty())
        client_param.password = _pwd;

    std::cout << url << std:: endl;

    m_client->connect(client::client_events::wsMessage, _message);
    m_client->connect(client::client_events::wsStatusChanged, _status);
    m_client->connect(client::client_events::wsError, _err);
    m_client->connect(client::client_events::wsConnect, _connect);
    m_client->connect(client::client_events::wsClose, _on_close);

    app_conf.ServerPort = port;
    app_conf.ServerHost = host;
    write_conf(app_conf);

    m_client->set_certificate_file("C:\\src\\bWebsockets\\src\\client\\console\\ssl\\arcirk_ru.crt");

    std::string line;

    while (getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        else if (line == "start")
        {
            m_client->open(arcirk::Uri::Parse(url));
        }
        else if (line == "stop")
        {
            m_client->close(false);
        }else if(line == "exit")
            break;
        else if (line == "send")
        {
            m_client->send_message("test message");
        }else if (line == "get_users") {
            get_online_users(m_client);
        }else {
            m_client->send_message(line);
        }
    }

    return EXIT_SUCCESS;
}