#include <arcirk.hpp>
#include <memory>
//#include <database_struct.hpp>
#include <query_builder.hpp>
#include <shared_struct.hpp>

#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include <vector>

#include "../arcirk/common/server_certificate.hpp"
#include "include/shared_state.hpp"
#include "include/listener.hpp"


using namespace arcirk;

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

boost::filesystem::path m_root_conf;

void verify_directories(const std::string& working_directory_dir = ""){

    using namespace boost::filesystem;

    if(!working_directory_dir.empty()){
        //path app_conf(working_directory_dir);
        //app_conf /= ARCIRK_VERSION;
        //m_root_conf = app_conf;
        m_root_conf = path(working_directory_dir);
        m_root_conf /= ARCIRK_VERSION;
    }else{
//        path app_conf(program_data());
//        app_conf /= ARCIRK_VERSION;
//        m_root_conf = app_conf;
        m_root_conf = path(program_data());
        m_root_conf /= ARCIRK_VERSION;
    }

    bool is_conf = arcirk::standard_paths::verify_directory(m_root_conf);

    if(is_conf){
        path html = m_root_conf;
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

        path data = m_root_conf;
        data /= "data";
        arcirk::standard_paths::verify_directory(data);

        path ssl_dir = m_root_conf;
        ssl_dir /= "ssl";
        arcirk::standard_paths::verify_directory(ssl_dir);
    }
}

void copy_ssl_file(const std::string& file_patch, server::server_config& conf){

    using namespace boost::filesystem;
    if(!exists(m_root_conf))
        return;

    path ssl_dir = m_root_conf;
    ssl_dir /= "ssl";

    path current(file_patch);

    if(!exists(current)){
        copy_file(
                current,
                ssl_dir / current.filename()
        );
        if(current.extension() == "key")
            conf.SSL_key_file = current.filename().string();
        else
            conf.SSL_crt_file = current.filename().string();
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

std::vector<std::string> get_tables(soci::session& sql){

    soci::rowset<soci::row> rs = (sql.prepare << "SELECT name FROM sqlite_master WHERE type='table';");
    std::vector<std::string> result;
    for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
    {
        soci::row const& row = *it;
        result.push_back(row.get<std::string>(0));
    }

    return result;
}

std::vector<std::string> get_views(soci::session& sql){

    soci::rowset<soci::row> rs = (sql.prepare << "SELECT name FROM sqlite_master WHERE type='view';");
    std::vector<std::string> result;
    for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
    {
        soci::row const& row = *it;
        result.push_back(row.get<std::string>(0));
    }

    return result;
}

void verify_table_users(soci::session& sql, const std::vector<std::string>& tables_arr){

    using namespace soci;

    if(std::find(tables_arr.begin(), tables_arr.end(), "Users") == tables_arr.end()){
        try {
            sql << database::users_table_ddl;
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return;
        }

        database::user_info u;
        u.ref = to_string(uuids::random_uuid());
        u.first = "admin";
        u.hash = arcirk::get_hash("admin", "admin");
        u.parent = arcirk::uuids::nil_string_uuid();
        u.role = enum_synonym(database::dbAdministrator);

        try {
            //Хотя бы одна учетная запись с ролью 'admin' должна быть
            int count = -1;
            sql << "select count(*) from Users where role = " <<  "'" << u.role << "'" , into(count);
            if(count <= 0){
                //Добавить учетную запись по умолчанию
                sql << "INSERT INTO Users(ref, first, hash, parent, role) VALUES(?, ?, ?, ?, ?)", soci::use(u.ref), soci::use(u.first), soci::use(u.hash), soci::use(u.parent), soci::use(u.role);
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

}

void verify_table_messages(soci::session& sql, const std::vector<std::string>& tables_arr){
    using namespace soci;

    if(std::find(tables_arr.begin(), tables_arr.end(), "Messages") == tables_arr.end()) {
        try {
            sql << database::messages_table_ddl;
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return;
        }
    }
}

void verify_tables(soci::session& sql, const std::vector<std::string>& tables_arr, std::map<std::string, std::string>& t_ddl){
    using namespace soci;

    for (auto itr = t_ddl.begin(); itr != t_ddl.end() ; ++itr) {
        if(std::find(tables_arr.begin(), tables_arr.end(), itr->first) == tables_arr.end()) {
            try {
                sql << itr->second;
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                continue;
            }
        }
    }


}

void verify_database(){

    using namespace boost::filesystem;
    using namespace soci;
    using namespace arcirk::database;

    path data = m_root_conf /+ "data" /+ "arcirk.sqlite";

    std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", data.string());
    session sql(soci::sqlite3, connection_string);

    try {
        auto m_tables = get_tables(sql);
        auto m_views = get_views(sql);
        verify_table_users(sql, m_tables);
        verify_table_messages(sql, m_tables);

        std::map<std::string, std::string> t_ddl;
        t_ddl.emplace(arcirk::enum_synonym(tables::tbOrganizations), organizations_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbPriceTypes), price_types_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbSubdivisions), subdivisions_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbWarehouses), warehouses_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbWorkplaces), workplaces_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbDevices), devices_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbDevicesType), devices_type_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbDocuments), documents_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbDocumentsTables), document_table_table_ddl);
        t_ddl.emplace(arcirk::enum_synonym(tables::tbNomenclature), nomenclature_table_ddl);

        //проверка таблиц
        verify_tables(sql, m_tables, t_ddl);

        //заполняем перечисления
        int count = -1;
        sql << "select count(*) from DevicesType", into(count);
        auto query = std::make_shared<builder::query_builder>();
        if(count <= 0){
            for (int l = 0; l < 5; ++l) {
                auto val = (devices_type)l;
                std::string ref = to_string(arcirk::uuids::random_uuid());
                std::string enum_name = arcirk::enum_synonym(val);
                query->clear();
                query->use({
                       {"first", enum_name},
                       {"ref", ref}
                });
                query->insert("DevicesType", true).execute(sql, {}, true);
            }
        }

        t_ddl.clear();
        t_ddl.emplace(arcirk::enum_synonym(views::dvDevicesView), devises_view_ddl);
        //проверка представлений
        verify_tables(sql, m_views, t_ddl);

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

void read_command_line(const command_line_parser::cmd_parser& parser, server::server_config& conf){

    if(parser.option_exists("-h")){
        conf.ServerHost = parser.option("-h");
    }
    if(parser.option_exists("-p")){
        const std::string &_port = parser.option("-p");
        if(!_port.empty())
            conf.ServerPort = static_cast<unsigned short>(std::atoi(_port.c_str()));
    }
    if(parser.option_exists("-wd")){
        conf.ServerWorkingDirectory= parser.option("-wd");
    }
    if(parser.option_exists("-t")){
        const std::string &_threads = parser.option("-t");
        if(!_threads.empty())
            conf.ThreadsCount =  std::max<int>(1, static_cast<unsigned short>(std::atoi(_threads.c_str())));
    }
    if(parser.option_exists("-usr")){
        conf.ServerUser = parser.option("-usr");
    }
    if(parser.option_exists("-pwd")){
        conf.ServerUserHash = arcirk::get_hash(conf.ServerUser, parser.option("-pwd"));
    }
    if(parser.option_exists("-crt_file")){
        copy_ssl_file(parser.option("-crt_file"), conf);
    }
    if(parser.option_exists("-key_file")){
        copy_ssl_file(parser.option("-key_file"), conf);
    }
    conf.UseAuthorization = parser.option_exists("-use_auth"); //требуется авторизация на сервере
    conf.AllowDelayedAuthorization = parser.option_exists("-ada");// разрешить отложенную авторизацию
    conf.AllowHistoryMessages = parser.option_exists("-ahm"); //разрешить хранение истории сообщений
}

void save_conf(server::server_config& conf){
    write_conf(conf, m_root_conf, ARCIRK_SERVER_CONF);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    using namespace boost::filesystem;

    command_line_parser::cmd_parser input(argc, argv);

    if(input.option_exists("-v") || input.option_exists("-version")){
        std::cout << arcirk::local_8bit("arcirk.websocket.server v.") << ARCIRK_VERSION << std::endl;
        return EXIT_SUCCESS;
    }
    //Проверяем доступность структуры каталогов
    verify_directories();

    auto conf = server::server_config();
    //инициализируем настройки
    read_conf(conf, m_root_conf, ARCIRK_SERVER_CONF);
    //если рабочий каталог не задан используем каталог по умолчанию
    if(conf.ServerWorkingDirectory.empty())
        conf.ServerWorkingDirectory = program_data().string();
    //проверяем доступность базы данных
    verify_database();
    //читаем командную строку
    read_command_line(input, conf);

    if(conf.ServerHost.empty()){
        //выбираем первый локальный хост по шаблону "192.168.xxx.xxx"
        conf.ServerHost = arcirk::bIp::get_default_host("0.0.0.0", "192.168");
    }
    if(conf.ServerHttpRoot.empty()){
        path html = m_root_conf;
        html /= "html";
        conf.ServerHttpRoot = html.string();
    }

    conf.Version = ARCIRK_VERSION;

    save_conf(conf);

    auto const address = net::ip::make_address(conf.ServerHost);
    auto const port = static_cast<unsigned short>(conf.ServerPort);
    auto const doc_root = std::make_shared<std::string>(conf.ServerHttpRoot);
    auto const threads = std::max<int>(1, conf.ThreadsCount);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    if(!conf.SSL_crt_file.empty() && !conf.SSL_key_file.empty() && !input.option_exists("-ssl_def")){
        load_certs(ctx, conf.SSL_crt_file, conf.SSL_key_file);
    }else
        load_server_default_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener>(
            ioc,
            ctx,
            tcp::endpoint{address, port},
            doc_root,
            boost::make_shared<shared_state>())->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
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