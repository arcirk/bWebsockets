//
// Created by admin on 10.09.2022.
//
#include <iostream>
#include <arcirk.hpp>
#include "../include/client.hpp"
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>
#include <boost/beast/ssl.hpp>

const std::string version = "1.1.0";
//bClient * client;

boost::filesystem::path m_root_conf;

namespace ssl = boost::asio::ssl;

BOOST_FUSION_DEFINE_STRUCT(
        (), settings,
        (std::string, ServerHost)
        (int, ServerPort)
        (std::string, ServerUser)
        (std::string, ServerUserHash)
        (std::string, ServerName)
        (std::string, ServerHttpRoot)
        (std::string, AutoConnect)
        (bool, UseLocalWebDavDirectory)
        (std::string, LocalWebDavDirectory)
        (std::string, WebDavHost)
        (std::string, WebDavUser)
        (std::string, WebDavPwd)
        (bool, WebDavSSL)
        (std::string, SQLFormat)
        (std::string, SQLHost)
        (std::string, SQLUser)
        (std::string, SQLPassword)
        (std::string, HSHost)
        (std::string, HSUser)
        (std::string, HSPassword)
        (bool, ServerSSL)
        (std::string, SSL_csr_file)
        (std::string, SSL_key_file)
        (bool, UseAuthorization)
        )

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

void read_conf(settings & result){

    using namespace boost::filesystem;

    if(!exists(arcirk::local_8bit(m_root_conf.string())))
        return;

    path conf = m_root_conf /+ "client_conf.json";

    if(exists(conf)){
        std::ifstream file(conf.string(), std::ios_base::in);
        std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        if(!str.empty()){
            result = pre::json::from_json<settings>(str);
        }
    }
}

void write_conf(settings & conf){
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
    std::cerr << what << "(" << code << "): " << arcirk::local_8bit(err) << std::endl;
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

//int
//main(int argc, char* argv[]){

//    setlocale(LC_ALL, "Russian");

//    InputParser input(argc, argv);
//
//    if(input.cmdOptionExists("-v") || input.cmdOptionExists("-version")){
//        std::cout << arcirk::local_8bit("arcirk.websocket.server v.") << version << std::endl;
//        return EXIT_SUCCESS;
//    }
//
//    const std::string &_url = input.getCmdOption("-url");
//    const std::string &_usr = input.getCmdOption("-usr");
//    const std::string &_pwd = input.getCmdOption("-pwd");
//
//
//    callback_connect _connect = std::bind(&on_connect);
//    callback_error _err = std::bind(&on_error, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
//    callback_message _message = std::bind(&on_message, std::placeholders::_1);
//    callback_status _status = std::bind(&on_status_changed, std::placeholders::_1);
//    callback_close _on_close = std::bind(&on_stop);
//
//    verify_directories();
//
//    settings app_conf;
//    read_conf(app_conf);
//
//    std::string host = app_conf.ServerHost;
//    int port = app_conf.ServerPort;
//
//    ssl::context ctx{ssl::context::tlsv12_client};
//    //client = new bClient(ctx, host, 8080);
//
//    if(_url.empty()){
//        if(host.empty())
//            host = arcirk::bIp::get_default_host("0.0.0.0", "192.168.43");
//
//        if(port <= 0)
//            port = 8080;
//        client = new bClient(host, 8080, ctx);
//    }else{
//        client = new bClient(_url, ctx);
//    }
//    client->connect(bClient::bClientEvent::wsMessage, _message);
//    client->connect(bClient::bClientEvent::wsStatusChanged, _status);
//    client->connect(bClient::bClientEvent::wsError, _err);
//    client->connect(bClient::bClientEvent::wsConnect, _connect);
//    client->connect(bClient::bClientEvent::wsClose, _on_close);
//
//    app_conf.ServerPort = port;
//    app_conf.ServerHost = host;
//    write_conf(app_conf);
//
//    client->set_certificate("C:\\src\\bWebsockets\\src\\client\\console\\ssl\\arcirk_ru.crt");
//
//    std::string line;

//    std::string auth_cred;
//    if(_usr.empty()){
//        auth_cred = "admin:admin";
//    }else{
//        auth_cred = _usr + ":" + _pwd;
//    }
//    std::string encoded_auth_cred = "Basic " + arcirk::base64::base64_encode(auth_cred);



//    while (getline(std::cin, line)) {
//        if (line.empty()) {
//            continue;
//        }
//        else if (line == "start")
//        {
//            client->open(_url,  _usr, _pwd);
//        }
//        else if (line == "stop")
//        {
//            client->close(false);
//        }else if(line == "exit")
//            break;
//        else if (line == "send")
//        {
//            client->send("test message", "", "");
//        }
//    }
//    return EXIT_SUCCESS;
//}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "Russian");
    // Check command line arguments.
    if(argc != 4 && argc != 5)
    {
        std::cerr <<
                  "Usage: http-client-sync-ssl <host> <port> <target>\n" <<
                  "Example:\n" <<
                  "    http-client-sync-ssl jsonplaceholder.typicode.com 80 /\n" <<
                  "    http-client-sync-ssl jsonplaceholder.typicode.com 443 /\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    uint16_t const port = static_cast<uint16_t>(std::stoi(argv[2]));
    auto const target = argv[3];

    try {
        boost::asio::io_context io_ctx;
        boost::asio::ssl::context ssl_ctx(boost::asio::ssl::context::tlsv12_client);
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_verify_mode(boost::asio::ssl::verify_none);
        /*
         * todo: configure the ssl context here
         */


        arcirk::ws::client::request req = {
                .host = host,
                .port = port,
                .target = target,
                .http_version = 11,
                .encryption = (port == 443 ? arcirk::ws::client::request::encryption::tls : arcirk::ws::client::request::encryption::none),
        };

        arcirk::ws::client client(io_ctx, ssl_ctx);
        auto response = client.synchronous_get(req);

        std::cout << "response = " << response.body_string() << std::endl;

    }
    catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}