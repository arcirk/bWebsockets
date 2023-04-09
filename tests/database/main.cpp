//
// Created by admin on 08.04.2023.
//
#include <arcirk.hpp>
#include <memory>
//#include <database_struct.hpp>
#include <query_builder.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include <vector>
#include <soci/soci.h>
#include <soci/odbc/soci-odbc.h>
#include <nlohmann/json.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>
#include <shared_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>


static inline std::vector<std::string> get_database_tables(soci::session& sql, arcirk::DatabaseType type = arcirk::dbTypeSQLite){

    std::string query = "SELECT name FROM sqlite_master WHERE type='table';";
    if(type == arcirk::dbTypeODBC){
        auto version = arcirk::server::get_version();
        std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.major), std::to_string(version.minor), std::to_string(version.path));
        sql << "use " + db_name;
        query = "SELECT * FROM sys.objects WHERE type in (N'U')";
    }

    soci::rowset<soci::row> rs = (sql.prepare << query);
    std::vector<std::string> result;
    for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
    {
        soci::row const& row = *it;
        result.push_back(row.get<std::string>(0));
    }

    return result;
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    using namespace arcirk;
    using namespace soci;
    using namespace arcirk::database;

    command_line_parser::cmd_parser input(argc, argv);

    auto host = input.option("-h");
    auto usr = input.option("-usr");
    auto pwd = input.option("-pwd");

    std::string connection_string = arcirk::str_sample("DRIVER={SQL Server};"
                                                       "SERVER=%1%;Persist Security Info=true;"
                                                       "uid=%2%;pwd=%3%", host, usr, pwd);

    try {
        auto sql = session{soci::odbc, connection_string};
        std::cout << "connection ok" << std::endl;

        auto version = arcirk::server::get_version();
        std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.major), std::to_string(version.minor), std::to_string(version.path));
        //Проверяем на существование базы данных
        auto builder = builder::query_builder(builder::sql_database_type::type_ODBC);
        builder.row_count().from("sys.databases").where(nlohmann::json{{"name", db_name}}, true);
        int count = -1;
        sql << builder.prepare(), into(count);
        if (count <= 0){
            std::cerr << arcirk::local_8bit("База данных не найдена!") << std::endl;
            sql << arcirk::str_sample("CREATE DATABASE %1%", db_name);
            sql << builder.prepare(), into(count);
            if (count > 0){
                std::cout << arcirk::local_8bit("База данных успешно создана!") << std::endl;
            }else{
                std::cerr << arcirk::local_8bit("Ошибка создания базы данных!!") << std::endl;
                return EXIT_FAILURE;
            }

        }else{
            std::cout << arcirk::local_8bit("База данных существует!") << std::endl;
        }

        std::vector<std::string> tables_ddl;
        std::vector<std::string> indexes_ddl;

        //Проверяем таблицу
        auto database_tables = get_database_tables(sql, dbTypeODBC);
        if(std::find(database_tables.begin(), database_tables.end(), arcirk::enum_synonym(tables::tbDatabaseConfig)) == database_tables.end()) {
            //auto ddl = get_ddl(tables::tbDatabaseConfig);
            //sql << ddl;
            std::cerr << arcirk::local_8bit("Таблица ") << arcirk::enum_synonym(tables::tbDatabaseConfig) << arcirk::local_8bit(" не существует!") << std::endl;

            tables_ddl.push_back("CREATE TABLE [dbo].[DatabaseConfig](\n"
                                 "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                 "[ref] [varchar](36) NOT NULL,\n"
                                 "[first] [varchar](max) NULL,\n"
                                 "[second] [varchar](max) NULL,\n"
                                 "[version] [int] NOT NULL,\n"
                                 " CONSTRAINT [PK_DatabaseConfig_1] PRIMARY KEY CLUSTERED \n"
                                 "(\n"
                                 "[_id] ASC\n"
                                 ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                 ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]");
            indexes_ddl.push_back("ALTER TABLE [dbo].[DatabaseConfig] ADD  CONSTRAINT [DF_DatabaseConfig_version]  DEFAULT ((0)) FOR [version]");
        }

        sql << "USE [arcirk_v110]";

        auto tr = soci::transaction(sql);

        for (auto ddl : tables_ddl) {
            sql << ddl;
        }
        for (auto ddl : indexes_ddl) {
            sql << ddl;
        }

        tr.commit();

        //Тест записи
        builder.clear();

        auto v = database_config();

        auto uuid = arcirk::uuids::random_uuid();
        v.ref = arcirk::uuids::uuid_to_string(uuid);
        v.first = "test first";
        v.second = "test second";
        v.version = 0;
        auto js = pre::json::to_json(v);

//        ByteArray br = arcirk::uuids::to_byte_array(uuid);
//        ByteArray cache = arcirk::string_to_byte_array(nlohmann::json{
//                {"comment", "test comment"}
//        }.dump());
//
//        nlohmann::json js{
//                {"ref", br},
//                {"first", "test first"},
//                {"second", "test second"},
//                {"cache", cache},
//                {"version", 0}
//        };
//        soci::blob b_ref( sql );
//        b_ref.write( 0, reinterpret_cast < const char *>( br.data() ), br.size() );
//        soci::blob b_cache( sql );
//        b_cache.write( 0, reinterpret_cast < const char *>( cache.data() ), cache.size() );

        js["version"] = 1;
        builder.use(js);
        std::string str = builder.insert(arcirk::enum_synonym(tables::tbDatabaseConfig), true).prepare();
        //auto st = pre::json::from_json<arcirk::database::database_config_test>(js);
        sql << str;
        //sql << str.c_str() , soci::use(st);

        builder.clear();
        builder.use(js);
        str = builder.update(arcirk::enum_synonym(tables::tbDatabaseConfig), true).where(nlohmann::json{
                {"ref", v.ref}
        }, true).prepare();

        std::cout << str << std::endl;
        sql << str;

        std::cout << arcirk::local_8bit("Проверка базы завершена.") << std::endl;

    } catch (native_exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}