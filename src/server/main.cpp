//
// Created by Борисоглебский on 05.09.2022.
//
//#include "./include/listener.hpp"
#include <soci/soci.h>

#include <arcirk.hpp>

//#include <soci/sqlite3/soci-sqlite3.h>

#include "./include/shared_state.hpp"

#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include <vector>


#include <string>

#include <boost/beast/ssl.hpp>
#include "include/server_certificate.hpp"

#include "include/listener_ssl.hpp"
#include "include/listener.hpp"



#include <soci/boost-fusion.h>
//#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>

#include "include/table_users.hpp"

//BOOST_FUSION_DEFINE_STRUCT(
//                (), TableUsers,
//                (int, _id)
//                (std::string, first)
//                (std::string, second)
//                (std::string, ref)
//                (std::string, hash)
//                (std::string, role)
//                (std::string, performance)
//                (std::string, parent)
//                (std::string, cache))

const std::string version = "1.1.0";

const std::string index_html_text = "<!DOCTYPE html>\n"
                                    "<html>\n"
                                    "<head>\n"
                                    "    <meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                                    "    <title>arcirk websockets</title>\n"
                                    "</head>\n"
                                    "<body>\n"
                                    "WebSocket Server Is Run\n"
                                    "</body>\n"
                                    "</html>";

std::vector<std::string> conf_aliases = {
        "ServerHost",
        "ServerPort",
        "ServerUser",
        "ServerUserHash",
        "ServerName",
        "ServerHttpRoot",
        "AutoConnect",
        "UseLocalWebDavDirectory",
        "LocalWebDavDirectory",
        "WebDavHost",
        "WebDavUser",
        "WebDavPwd",
        "WebDavSSL",
        "SQLFormat",
        "SQLHost",
        "SQLUser",
        "SQLPassword",
        "HSHost",
        "HSUser",
        "HSPassword",
        "ServerSSL",
        "SSL_csr_file",
        "SSL_key_file"
};

boost::filesystem::path m_root_conf;

namespace ssl = boost::asio::ssl;

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
    std::vector <std::string> tokens;
};

void verify_directories(){

    using namespace boost::filesystem;

    std::string arcirk_dir = "arcirk";
#ifndef _WINDOWS
    arcirk_dir = "." + arcirk_dir;
#endif

    path app_conf(arcirk::standard_paths::this_server_conf_dir(arcirk_dir));
    app_conf /= version;

    bool is_conf = arcirk::standard_paths::verify_directory(app_conf);

    m_root_conf = app_conf;

    if(is_conf){
        path html = app_conf;
        html /= "html";
        if(arcirk::standard_paths::verify_directory(html)){
            path index_html = html;
            index_html /= "index.html";
            if(!exists(index_html)){
                std::ofstream out;
                out.open(index_html.string());
                if(out.is_open()){
                    out << index_html_text << std::endl;
                    out.close();
                }
            }
        }

        path data = app_conf;
        data /= "data";
        arcirk::standard_paths::verify_directory(data);

    }
}

void install_ssl_certificate(const std::string& dir){

    using namespace boost::filesystem;
    if(!exists(m_root_conf))
        return;

    path ssl_dir = m_root_conf;
    ssl_dir /= "ssl";

    if(arcirk::standard_paths::verify_directory(ssl_dir)){
        path source(dir);
        if(exists(dir) && is_directory(dir)){
            for(directory_iterator file(source); file != directory_iterator(); ++file){
                try {
                    path current(file->path());
                    if(!is_directory(current))
                    {
                        copy_file(
                                current,
                                ssl_dir / current.filename()
                        );
                    }
                }catch (filesystem_error const &error) {
                    std::cerr << arcirk::local_8bit(error.what()) << std::endl;
                }
            }

        }
    }

}

void load_certs(boost::asio::ssl::context& ctx, const std::string& cert, const std::string& key){

    using namespace boost::filesystem;
    path ssl_dir = m_root_conf;
    ssl_dir /= "ssl";
    if(!exists(ssl_dir))
        return;

    path cert_file = ssl_dir;
    cert_file /= cert;

    path key_file = ssl_dir;
    key_file /= key;

    if(!exists(cert_file) || !exists(key_file))
        return;

    std::string _cert;
    std::ifstream c_in(cert_file.string());
    std::ostringstream c_oss;
    c_oss << c_in.rdbuf();
    _cert = c_oss.str();

    std::string _key;
    std::ifstream k_in(key_file.string());
    std::ostringstream k_oss;
    k_oss << k_in.rdbuf();
    _key = k_oss.str();

    if(_cert.empty() || _key.empty()){
        std::cerr << "error read certificate files" << std::endl;
        return;
    }

    load_server_certificate(ctx, _cert, _key);
}

void verify_database(){

    using namespace boost::filesystem;
    using namespace soci;

    const std::string table_ddl = "CREATE TABLE NOT EXISTS Users (\n"
                                  "    _id         INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                  "    first  TEXT,\n"
                                  "    second TEXT,\n"
                                  "    ref         TEXT (36) UNIQUE\n"
                                  "                          NOT NULL,\n"
                                  "    hash        TEXT      UNIQUE\n"
                                  "                          NOT NULL,\n"
                                  "    role        TEXT,\n"
                                  "    performance TEXT,\n"
                                  "    parent      TEXT (36),\n"
                                  "    cache       TEXT\n"
                                  ");";

    path data = m_root_conf /+ "data" /+ "arcirk.sqlite";
    //if(!exists(data)){
//        db_pool db;
//        db.connect(data.string());
//        soci::session sql(*db.get_pool());
//        session sql("sqlite3", data.string());
//        sql << table_ddl;

//        user_info u;
//        u.ref = to_string(uuids::random_uuid());
//        u.first = "admin";
//        u.hash = arcirk::get_hash("admin", "admin");
//        u.parent = arcirk::uuids::nil_string_uuid();
//        u.role = "admin";
//
//        sql << "INSERT INTO Users(ref, first, hash, parent, role) VALUES(:ref, :first, :hash, :parent, :role)", soci::use(u);
    //}
}

int
main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    using namespace boost::filesystem;

    InputParser input(argc, argv);

    if(input.cmdOptionExists("-v") || input.cmdOptionExists("-version")){
        std::cout << arcirk::local_8bit("arcirk.websocket.server v.") << version << std::endl;
        return EXIT_SUCCESS;
    }

    const std::string &_host = input.getCmdOption("-h");
    const std::string &_port = input.getCmdOption("-p");
    const std::string &_doc_root  = input.getCmdOption("-d");
    const std::string &_threads = input.getCmdOption("-t");
    const std::string &_user = input.getCmdOption("-usr");
    const std::string &_pwd = input.getCmdOption("-pwd");
    const std::string &_ssl_dir = input.getCmdOption("-ssl_dir");
    const std::string &_ssl = input.getCmdOption("-ssl");
    const std::string &_csr_file = input.getCmdOption("-csr_file");
    const std::string &_key_file = input.getCmdOption("-key_file");

    auto threads = 4;
    auto address = net::ip::make_address(arcirk::bIp::get_default_host("0.0.0.0", "192.168"));

    unsigned short port = 8080;

    verify_directories();

    verify_database();

    path html = m_root_conf;
    html /= "html";
    std::string doc_root = html.string();

    auto conf = arcirk::bConf(m_root_conf, "arcirk_server", conf_aliases);

    if(!_host.empty())
        address = net::ip::make_address(_host);
    else{
        std::string v = conf[ServerHost].to_string();
        if(!v.empty())
            address = net::ip::make_address(v);
    }
    if(!_port.empty())
        port = static_cast<unsigned short>(std::atoi(_port.c_str()));
    else{
        int v = conf[ServerPort].get_int();
        if(v > 0)
            port = static_cast<unsigned short>(v);
    }
    if(!_threads.empty())
        threads = std::max<int>(1, std::atoi(_threads.c_str()));

    if(!_doc_root.empty())
        doc_root = _doc_root;
    else{
        std::string v = conf[ServerHttpRoot].to_string();
        if(!v.empty())
            doc_root = v;
    }

    if(!_ssl_dir.empty()){
        install_ssl_certificate(_ssl_dir);
    }

    bool is_ssl = false;
    std::string csr_file = _csr_file;
    std::string key_file = _key_file;

    if(!_ssl.empty()){
        is_ssl = _ssl == "true";
    }
    if(!is_ssl){
        is_ssl = conf[ServerSSL].get_bool();
    }
    if(csr_file.empty()){
        csr_file = conf[SSL_csr_file].get_string();
    }
    if(key_file.empty()){
        key_file = conf[SSL_key_file].get_string();
    }
    conf[ServerHost] = address.to_string();
    conf[ServerPort] = (int)port;
    conf[ServerHttpRoot] = doc_root;
    conf[ServerSSL] = is_ssl;
    conf[SSL_csr_file] = csr_file;
    conf[SSL_key_file] = key_file;
    conf.save();

    ssl::context ctx{ssl::context::tlsv12};
    if (is_ssl){
        load_certs(ctx, csr_file, key_file);
    }

    const std::string Protocol = is_ssl ? "wss://" : "ws://";
    std::cout << arcirk::local_8bit("arcirk.websocket.server v.") << version << std::endl;
    std::cout << arcirk::local_8bit("Попытка запуска сервера ") << Protocol << address << ":" << port << std::endl;
    //std::cout << arcirk::local_8bit("Каталог html ") << doc_root << std::endl;

    // The io_context is required for all I/O
    net::io_context ioc;

    // Create and launch a listening port
    if (!is_ssl){
        boost::make_shared<listener>(
                ioc,
                tcp::endpoint{address, port},
                boost::make_shared<shared_state>(doc_root.c_str()))->run();
    }else
        boost::make_shared<listener_ssl>(
                ioc,
                ctx,
                tcp::endpoint{address, port},
                boost::make_shared<shared_state>(doc_root.c_str()))->run();


    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&ioc](boost::system::error_code const&, int)
            {
                // Stop the io_context. This will cause run()
                // to return immediately, eventually destroying the
                // io_context and any remaining handlers in it.
                ioc.stop();
            });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for(auto& t : v)
        t.join();

    return EXIT_SUCCESS;
}