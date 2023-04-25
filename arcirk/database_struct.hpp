#ifndef ARCIRK_DATABASE_STRUCT_HPP
#define ARCIRK_DATABASE_STRUCT_HPP

#include "includes.hpp"
#include <map>
#include "arcirk.hpp"

namespace arcirk{
        enum DatabaseType{
            dbTypeSQLite = 0,
            dbTypeODBC
        };
};

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), database_config,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (int, version)
);
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), database_config_test,
        (std::vector<unsigned char>, ref)
        (std::string, first)
        (std::string, second)
        (std::vector<unsigned char>, cache)
        (int, version)
);
BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), user_info,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, hash)
                (std::string, role)
                (std::string, performance)
                (std::string, parent)
                (std::string, cache)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), messages,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, message)
                (std::string, token)
                (int, date)
                (std::string, content_type)
                (int, unread_messages)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), organizations,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), subdivisions,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), warehouses,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), price_types,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), workplaces,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, server)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), devices,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, deviceType)
                (std::string, address)
                (std::string, workplace)
                (std::string, price_type)
                (std::string, warehouse)
                (std::string, subdivision)
                (std::string, organization)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), devices_view,
                (std::string, ref)
                (std::string, workplace)
                (std::string, price)
                (std::string, warehouse)
                (std::string, subdivision)
                (std::string, organization)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), documents,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (std::string, number)
                (int, date)
                (std::string, xml_type)
                (int, version)
                (std::string, device_id)
                (std::string, workplace)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), document_table,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, cache)
                (double, price)
                (double, quantity)
                (std::string, barcode)
                (std::string, vendor_code)
                (std::string, product)
                (std::string, parent)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), nomenclature,
                (int, _id)
                (std::string, first) // Наименование
                (std::string, second)
                (std::string, ref)
                (std::string, cache) // Все остальные реквизиты
                (std::string, parent)
                (std::string, vendor_code)
                (std::string, trademark)
                (std::string, unit)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), barcodes,
                (int, _id)
                (std::string, first)
                (std::string, second)
                (std::string, ref)
                (std::string, barcode)
                (std::string, parent)
                (std::string, vendor_code)
                (std::string, first_name)
                (int, is_group)
                (int, deletion_mark)
                (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
                (arcirk::database), table_info_sqlite,
                (int, cid)
                (std::string, name)
                (std::string, type)
                (int, notnull)
                (std::string, dflt_value)
                (int, bk)
);

namespace arcirk::database{

    enum roles{
        dbUser,
        dbAdministrator,
        roles_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(roles, {
        {roles_INVALID, nullptr}    ,
        {dbAdministrator, "admin"}  ,
        {dbUser, "user"}  ,
    })

    enum text_type{
        dbText,
        dbHtmlText,
        dbXmlText,
        dbJsonText,
        text_type_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(text_type, {
        {text_type_INVALID, nullptr}    ,
        {dbText, "Text"}  ,
        {dbHtmlText, "HtmlText"}  ,
        {dbXmlText, "XmlText"}  ,
        {dbJsonText, "JsonText"}  ,
    })

    enum devices_type{
        devDesktop,
        devServer,
        devPhone,
        devTablet,
        devExtendedLib,
        dev_INVALID=-1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(devices_type, {
        {dev_INVALID, nullptr},
        {devDesktop, "Desktop"},
        {devServer, "Server"},
        {devPhone, "Phone"},
        {devTablet, "Tablet"},
        {devExtendedLib, "ExtendedLib"},
    })

    enum tables{
        tbUsers,
        tbMessages,
        tbOrganizations,
        tbSubdivisions,
        tbWarehouses,
        tbPriceTypes,
        tbWorkplaces,
        tbDevices,
        tbDevicesType,
        tbDocuments,
        tbDocumentsTables,
        tbNomenclature,
        tbDatabaseConfig,
        tbBarcodes,
        tables_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(tables, {
        {tables_INVALID, nullptr}    ,
        {tbUsers, "Users"}  ,
        {tbMessages, "Messages"}  ,
        {tbOrganizations, "Organizations"}  ,
        {tbSubdivisions, "Subdivisions"}  ,
        {tbWarehouses, "Warehouses"}  ,
        {tbPriceTypes, "PriceTypes"}  ,
        {tbWorkplaces, "Workplaces"}  ,
        {tbDevices, "Devices"}  ,
        {tbDevicesType, "DevicesType"}  ,
        {tbDocuments, "Documents"}  ,
        {tbDocumentsTables, "DocumentsTables"}  ,
        {tbNomenclature, "Nomenclature"}  ,
        {tbDatabaseConfig, "DatabaseConfig"}  ,
        {tbBarcodes, "Barcodes"}  ,
    })

    enum views{
        dvDevicesView,
        dvDocumentsTableView,
        views_INVALID=-1,
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(views, {
        { views_INVALID, nullptr }    ,
        { dvDevicesView, "DevicesView" }  ,
        { dvDocumentsTableView, "DocumentsTableView" }  ,
    });

    const std::string messages_table_ddl = "CREATE TABLE Messages (\n"
                                           "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                           "    [first]         TEXT,\n"
                                           "    second          TEXT,\n"
                                           "    ref             TEXT (36) UNIQUE\n"
                                           "                             NOT NULL,\n"
                                           "    message         TEXT,\n"
                                           "    token           TEXT      NOT NULL,\n"
                                           "    date            INTEGER,\n"
                                           "    content_type    TEXT      DEFAULT HTML,\n"
                                           "    unread_messages INTEGER   DEFAULT (0),\n"
                                           "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                           "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                           "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                           "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                           ");";
    const std::string messages_odbc_table_ddl = "CREATE TABLE [dbo].[Messages](\n"
                                            "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                            "[first] [varchar](max) NULL,\n"
                                            "[second] [varchar](max) NULL,\n"
                                            "[ref] [char](36) NOT NULL UNIQUE,\n"
                                            "[message] [text] NULL,\n"
                                            "[token] [varchar](max) NOT NULL,\n"
                                            "[date] [int] NOT NULL,\n"
                                            "[content_type] [char](10) NULL,\n"
                                            "[unread_messages] [int] NULL,\n"
                                            "[parent] [char](36) NULL,\n"
                                            "[is_group] [int] NOT NULL,\n"
                                            "[deletion_mark] [int] NOT NULL,\n"
                                            "[version] [int] NOT NULL\n"
                                            "CONSTRAINT [PK_Messages] PRIMARY KEY CLUSTERED\n"
                                            "(\n"
                                            "[_id] ASC\n"
                                            ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                            ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string users_table_ddl = "CREATE TABLE Users (\n"
                                        "    _id           INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                        "    [first]       TEXT      DEFAULT \"\"\n"
                                        "                            NOT NULL,\n"
                                        "    second        TEXT      DEFAULT \"\",\n"
                                        "    ref           TEXT (36) UNIQUE\n"
                                        "                            NOT NULL,\n"
                                        "    hash          TEXT      UNIQUE\n"
                                        "                            NOT NULL,\n"
                                        "    role          TEXT      DEFAULT user\n"
                                        "                            NOT NULL,\n"
                                        "    performance   TEXT      DEFAULT \"\",\n"
                                        "    parent        TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                        "    cache         TEXT      DEFAULT \"\",\n"
                                        "    is_group      INTEGER   NOT NULL\n"
                                        "                            DEFAULT (0),\n"
                                        "    deletion_mark INTEGER   NOT NULL\n"
                                        "                            DEFAULT (0),\n"
                                        "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                        ");";
    const std::string users_odbc_table_ddl = "CREATE TABLE [dbo].[Users](\n"
                                        "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                        "[first] [varchar](max) NULL,\n"
                                        "[second] [varchar](max) NULL,\n"
                                        "[ref] [char](36) NOT NULL UNIQUE,\n"
                                        "[hash] [varchar](max) NOT NULL,\n"
                                        "[role] [varchar](max) NULL,\n"
                                        "[performance] [varchar](max) NULL,\n"
                                        "[parent] [char](36) NULL,\n"
                                        "[cache] [text] NULL,\n"
                                        "[is_group] [int] NOT NULL,\n"
                                        "[deletion_mark] [int] NOT NULL,\n"
                                        "[version] [int] NOT NULL\n"
                                        "CONSTRAINT [PK_Users] PRIMARY KEY CLUSTERED\n"
                                        "(\n"
                                        "[_id] ASC\n"
                                        ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                        ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string organizations_table_ddl = "CREATE TABLE Organizations (\n"
                                                "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                                "    [first]         TEXT,\n"
                                                "    second          TEXT,\n"
                                                "    ref             TEXT (36) UNIQUE\n"
                                                "                             NOT NULL,\n"
                                                "    cache           TEXT      DEFAULT \"\",\n"
                                                "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                                "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                                "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                                "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                                ");";
    const std::string organizations_odbc_table_ddl = "CREATE TABLE [dbo].[Organizations](\n"
                                                "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                "[first] [varchar](max) NULL,\n"
                                                "[second] [varchar](max) NULL,\n"
                                                "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                "[cache] [text] NULL,\n"
                                                 "[parent] [char](36) NULL,\n"
                                                 "[is_group] [int] NOT NULL,\n"
                                                 "[deletion_mark] [int] NOT NULL,\n"
                                                 "[version] [int] NOT NULL\n"
                                                "CONSTRAINT [PK_Organizations] PRIMARY KEY CLUSTERED\n"
                                                "(\n"
                                                "[_id] ASC\n"
                                                ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string subdivisions_table_ddl = "CREATE TABLE Subdivisions (\n"
                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                               "    [first]         TEXT,\n"
                                               "    second          TEXT,\n"
                                               "    ref             TEXT (36) UNIQUE\n"
                                               "                             NOT NULL,\n"
                                               "    cache           TEXT      DEFAULT \"\",\n"
                                               "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                               "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                               "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                               "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                               ");";
    const std::string subdivisions_odbc_table_ddl = "CREATE TABLE [dbo].[Subdivisions](\n"
                                                "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                "[first] [varchar](max) NULL,\n"
                                                "[second] [varchar](max) NULL,\n"
                                                "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                "[cache] [text] NULL,\n"
                                                    "[parent] [char](36) NULL,\n"
                                                    "[is_group] [int] NOT NULL,\n"
                                                    "[deletion_mark] [int] NOT NULL,\n"
                                                    "[version] [int] NOT NULL\n"
                                                "CONSTRAINT [PK_Subdivisions] PRIMARY KEY CLUSTERED\n"
                                                "(\n"
                                                "[_id] ASC\n"
                                                ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string warehouses_table_ddl = "CREATE TABLE Warehouses (\n"
                                             "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                             "    [first]         TEXT,\n"
                                             "    second          TEXT,\n"
                                             "    ref             TEXT (36) UNIQUE\n"
                                             "                             NOT NULL,\n"
                                             "    cache           TEXT      DEFAULT \"\",\n"
                                             "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                             "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                             "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                             "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                             ");";
    const std::string warehouses_odbc_table_ddl = "CREATE TABLE [dbo].[Warehouses](\n"
                                                "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                "[first] [varchar](max) NULL,\n"
                                                "[second] [varchar](max) NULL,\n"
                                                "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                "[cache] [text] NULL,\n"
                                                  "[parent] [char](36) NULL,\n"
                                                  "[is_group] [int] NOT NULL,\n"
                                                  "[deletion_mark] [int] NOT NULL,\n"
                                                  "[version] [int] NOT NULL\n"
                                                "CONSTRAINT [PK_Warehouses] PRIMARY KEY CLUSTERED\n"
                                                "(\n"
                                                "[_id] ASC\n"
                                                ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string price_types_table_ddl = "CREATE TABLE PriceTypes (\n"
                                              "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                              "    [first]         TEXT,\n"
                                              "    second          TEXT,\n"
                                              "    ref             TEXT (36) UNIQUE\n"
                                              "                             NOT NULL,\n"
                                              "    cache           TEXT      DEFAULT \"\",\n"
                                              "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                              "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                              "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                              "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                              ");";
    const std::string price_types_odbc_table_ddl = "CREATE TABLE [dbo].[PriceTypes](\n"
                                                  "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                  "[first] [varchar](max) NULL,\n"
                                                  "[second] [varchar](max) NULL,\n"
                                                  "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                  "[cache] [text] NULL,\n"
                                                   "[parent] [char](36) NULL,\n"
                                                   "[is_group] [int] NOT NULL,\n"
                                                   "[deletion_mark] [int] NOT NULL,\n"
                                                   "[version] [int] NOT NULL\n"
                                                  "CONSTRAINT [PK_PriceTypes] PRIMARY KEY CLUSTERED\n"
                                                  "(\n"
                                                  "[_id] ASC\n"
                                                  ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                  ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string devices_type_table_ddl = "CREATE TABLE DevicesType (\n"
                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                               "    [first]         TEXT,\n"
                                               "    second          TEXT,\n"
                                               "    ref             TEXT (36) UNIQUE\n"
                                               "                             NOT NULL,\n"
                                               "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                               "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                               "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                               "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                               ");";

    const std::string devices_type_odbc_table_ddl = "CREATE TABLE [dbo].[DevicesType](\n"
                                                   "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                   "[first] [varchar](max) NULL,\n"
                                                   "[second] [varchar](max) NULL,\n"
                                                   "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                    "[parent] [char](36) NULL,\n"
                                                    "[is_group] [int] NOT NULL,\n"
                                                    "[deletion_mark] [int] NOT NULL,\n"
                                                    "[version] [int] NOT NULL\n"
                                                   "CONSTRAINT [PK_DevicesType] PRIMARY KEY CLUSTERED\n"
                                                   "(\n"
                                                   "[_id] ASC\n"
                                                   ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                   ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string workplaces_table_ddl = "CREATE TABLE Workplaces (\n"
                                             "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                             "    [first]         TEXT,\n"
                                             "    second          TEXT,\n"
                                             "    ref             TEXT (36) UNIQUE\n"
                                             "                             NOT NULL,\n"
                                             "    cache           TEXT      DEFAULT \"\",\n"
                                             "    server          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                             "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                             "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                             "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                             "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                             ");";

    const std::string workplaces_odbc_table_ddl = "CREATE TABLE [dbo].[Workplaces](\n"
                                                    "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                    "[first] [varchar](max) NULL,\n"
                                                    "[second] [varchar](max) NULL,\n"
                                                    "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                  "[cache] [text] NULL,\n"
                                                    "[server] [char](36) NULL,\n"
                                                  "[parent] [char](36) NULL,\n"
                                                  "[is_group] [int] NOT NULL,\n"
                                                  "[deletion_mark] [int] NOT NULL,\n"
                                                  "[version] [int] NOT NULL\n"
                                                    "CONSTRAINT [PK_Workplaces] PRIMARY KEY CLUSTERED\n"
                                                    "(\n"
                                                    "[_id] ASC\n"
                                                    ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                    ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string devices_table_ddl = "CREATE TABLE Devices (\n"
                                          "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                          "    [first]         TEXT,\n"
                                          "    second          TEXT,\n"
                                          "    ref             TEXT (36) UNIQUE\n"
                                          "                             NOT NULL,\n"
                                          "    cache           TEXT      DEFAULT \"\",\n"
                                          "    deviceType      TEXT      DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    address         TEXT      DEFAULT \"127.0.0.1\",\n"
                                          "    workplace       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    price_type      TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    warehouse       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    subdivision     TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    organization    TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                          "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                          "    version         INTEGER   DEFAULT (0)  NOT NULL\n"
                                          ");";
    const std::string devices_odbc_table_ddl = "CREATE TABLE [dbo].[Devices](\n"
                                                  "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                  "[first] [varchar](max) NULL,\n"
                                                  "[second] [varchar](max) NULL,\n"
                                                  "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                  "[cache] [text] NULL,\n"
                                                  "[deviceType] [varchar](max) NULL,\n"
                                                  "[address] [varchar](max) NULL,\n"
                                                  "[workplace] [char](36) NULL,\n"
                                                  "[price_type] [char](36) NULL,\n"
                                                  "[warehouse] [char](36) NULL,\n"
                                                  "[subdivision] [char](36) NULL,\n"
                                                  "[organization] [char](36) NULL,\n"
                                               "[parent] [char](36) NULL,\n"
                                               "[is_group] [int] NOT NULL,\n"
                                               "[deletion_mark] [int] NOT NULL,\n"
                                               "[version] [int] NOT NULL\n"
                                                  "CONSTRAINT [PK_Devices] PRIMARY KEY CLUSTERED\n"
                                                  "(\n"
                                                  "[_id] ASC\n"
                                                  ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                  ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string devises_view_ddl = "CREATE VIEW DevicesView AS\n"
                                         "    SELECT Devices.ref AS ref,\n"
                                         "           Devices.[first] AS [first],\n"
                                         "           Devices.second AS second,\n"
                                         "           Devices.deviceType AS device_type,\n"
                                         "           Organizations.[first] AS organization,\n"
                                         "           Subdivisions.[first] AS subdivision,\n"
                                         "           Warehouses.[first] AS warehouse,\n"
                                         "           PriceTypes.[first] AS price,\n"
                                         "           Workplaces.[first] AS workplace\n"
                                         "      FROM Devices\n"
                                         "           LEFT JOIN\n"
                                         "           Organizations ON Devices.organization = Organizations.ref\n"
                                         "           LEFT JOIN\n"
                                         "           Subdivisions ON Devices.subdivision = Subdivisions.ref\n"
                                         "           LEFT JOIN\n"
                                         "           Warehouses ON Devices.warehouse = Warehouses.ref\n"
                                         "           LEFT JOIN\n"
                                         "           PriceTypes ON Devices.price_type = PriceTypes.ref\n"
                                         "           LEFT JOIN\n"
                                         "           Workplaces ON Devices.workplace = Workplaces.ref;";

    const std::string documents_table_ddl = "CREATE TABLE Documents (\n"
                                            "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                            "    [first]         TEXT,\n"
                                            "    second          TEXT,\n"
                                            "    ref             TEXT (36) UNIQUE\n"
                                            "                             NOT NULL,\n"
                                            "    cache           TEXT      DEFAULT \"\",\n"
                                            "    number          TEXT      DEFAULT \"\",\n"
                                            "    date            INTEGER NOT NULL DEFAULT(0),\n"
                                            "    xml_type        TEXT      DEFAULT \"\",\n"
                                            "    version         INTEGER NOT NULL DEFAULT(0),\n"
                                            "    device_id       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                            "    workplace       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                            "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                            "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                            "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL"
                                            ");";
    const std::string documents_odbc_table_ddl = "CREATE TABLE [dbo].[Documents](\n"
                                               "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                               "[first] [varchar](max) NULL,\n"
                                               "[second] [varchar](max) NULL,\n"
                                               "[ref] [char](36) NOT NULL UNIQUE,\n"
                                               "[cache] [text] NULL,\n"
                                               "[number] [varchar](max)NULL,\n"
                                               "[date] [int] NOT NULL,\n"
                                               "[xml_type] [varchar](max) NULL,\n"
                                               "[device_id] [char](36) NULL,\n"
                                               "[workplace] [char](36) NULL,\n"
                                                 "[parent] [char](36) NULL,\n"
                                                 "[is_group] [int] NOT NULL,\n"
                                                 "[deletion_mark] [int] NOT NULL,\n"
                                                 "[version] [int] NOT NULL\n"
                                               "CONSTRAINT [PK_Documents] PRIMARY KEY CLUSTERED\n"
                                               "(\n"
                                               "[_id] ASC\n"
                                               ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                               ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string document_table_table_ddl = "CREATE TABLE DocumentsTables (\n"
                                                 "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                                 "    [first]         TEXT,\n"
                                                 "    second          TEXT,\n"
                                                 "    ref             TEXT (36) UNIQUE\n"
                                                 "                             NOT NULL,\n"
                                                 "    cache           TEXT      DEFAULT \"\",\n"
                                                 "    price           DOUBLE DEFAULT (0),\n"
                                                 "    quantity        DOUBLE DEFAULT (0),\n"
                                                 "    barcode         TEXT DEFAULT \"\",\n"
                                                 "    vendor_code     TEXT DEFAULT \"\",\n"
                                                 "    product         TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                                 "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                                 "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                                 "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                                 "    version         INTEGER NOT NULL DEFAULT(0)\n"
                                                 ");";
    const std::string document_table_odbc_table_ddl = "CREATE TABLE [dbo].[DocumentsTables](\n"
                                                 "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                 "[first] [varchar](max) NULL,\n"
                                                 "[second] [varchar](max) NULL,\n"
                                                 "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                 "[cache] [text] NULL,\n"
                                                 "[price] [float] NOT NULL,\n"
                                                 "[quantity] [float] NOT NULL,\n"
                                                 "[barcode] [varchar](max) NULL,\n"
                                                 "[vendor_code] [varchar](max) NULL,\n"
                                                 "[product] [char](36) NULL,\n"
                                                      "[parent] [char](36) NULL,\n"
                                                      "[is_group] [int] NOT NULL,\n"
                                                      "[deletion_mark] [int] NOT NULL,\n"
                                                      "[version] [int] NOT NULL\n"
                                                 "CONSTRAINT [PK_DocumentsTables] PRIMARY KEY CLUSTERED\n"
                                                 "(\n"
                                                 "[_id] ASC\n"
                                                 ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                 ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string nomenclature_table_ddl = "CREATE TABLE Nomenclature (\n"
                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                               "    [first]         TEXT,\n"
                                               "    second          TEXT,\n"
                                               "    ref             TEXT (36) UNIQUE\n"
                                               "                             NOT NULL,\n"
                                               "    cache           TEXT      DEFAULT \"\",\n"
                                               "    vendor_code     TEXT DEFAULT \"\",\n"
                                               "    trademark       TEXT DEFAULT \"\",\n"
                                               "    unit            TEXT DEFAULT \"шт.\",\n"
                                               "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                               "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                               "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                               "    version         INTEGER NOT NULL DEFAULT(0)\n"
                                               ");";

    const std::string nomenclature_odbc_table_ddl = "CREATE TABLE [dbo].[Nomenclature](\n"
                                                "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                "[first] [varchar](max) NULL,\n"
                                                "[second] [varchar](max) NULL,\n"
                                                "[ref] [char](36) NOT NULL UNIQUE,\n"
                                                "[cache] [text] NULL,\n"
                                                "[vendor_code] [varchar](max) NULL,\n"
                                                "[trademark] [varchar](max) NULL,\n"
                                                "[unit] [varchar](max) NULL,\n"
                                                "[parent] [char](36) NULL,\n"
                                                "[is_group] [int] NOT NULL,\n"
                                                "[deletion_mark] [int] NOT NULL,\n"
                                                "[version] [int] NOT NULL\n"
                                                "CONSTRAINT [PK_Nomenclature] PRIMARY KEY CLUSTERED\n"
                                                "(\n"
                                                "[_id] ASC\n"
                                                ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string barcodes_table_ddl = "CREATE TABLE Barcodes (\n"
                                           "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                           "    [first]         TEXT,\n"
                                           "    second          TEXT,\n"
                                           "    ref             TEXT (36) UNIQUE\n"
                                           "                             NOT NULL,\n"
                                           "    barcode         TEXT (128) DEFAULT \"\",\n"
                                           "    vendor_code     TEXT DEFAULT \"\",\n"
                                           "    first_name      TEXT DEFAULT \"\",\n"
                                           "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                           "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
                                           "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
                                           "    version         INTEGER NOT NULL DEFAULT(0)\n"
                                               ");";

    const std::string barcodes_odbc_table_ddl = "CREATE TABLE [dbo].[Barcodes](\n"
                                            "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                            "[first] [varchar](max) NULL,\n"
                                            "[second] [varchar](max) NULL,\n"
                                            "[ref] [char](36) NOT NULL UNIQUE,\n"
                                            "[barcode][varchar](128) NULL,\n"
                                            "[parent] [char](36) NULL,\n"
                                            "[vendor_code]     [varchar](max) NULL,\n"
                                            "[first_name]     [varchar](max) NULL,\n"
                                            "[is_group] [int] NOT NULL,\n"
                                            "[deletion_mark] [int] NOT NULL,\n"
                                            "[version] [int] NOT NULL\n"
                                            "CONSTRAINT [PK_Barcodes] PRIMARY KEY CLUSTERED\n"
                                            "(\n"
                                            "[_id] ASC\n"
                                            ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                            ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";

    const std::string database_config_table_ddl = "CREATE TABLE DatabaseConfig (\n"
                                                  "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                                  "    [first]         TEXT,\n"
                                                  "    second          TEXT,\n"
                                                  "    ref             TEXT (36) UNIQUE\n"
                                                  "                             NOT NULL,\n"
                                                  "    version         INTEGER  DEFAULT(0)  NOT NULL\n" //Это версия структуры таблицы, во всех остальных поле версия - это версия объекта
                                                  ");";

    const std::string database_config_odbc_table_ddl = "CREATE TABLE [dbo].[DatabaseConfig](\n"
                                                "[_id] [int] IDENTITY(1,1) NOT NULL,\n"
                                                "[first] [varchar](max) NULL,\n"
                                                "[second] [varchar](max) NULL,\n"
                                                "[ref] [char](36) NOT NULL,\n"
                                                "[version] [int] NOT NULL\n"
                                                "CONSTRAINT [PK_DatabaseConfig] PRIMARY KEY CLUSTERED\n"
                                                "(\n"
                                                "[_id] ASC\n"
                                                ")WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]\n"
                                                ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]";


    const std::string documents_table_view_ddl = "CREATE VIEW DocumentsTableView AS\n"
                                                "    SELECT DocumentsTables.barcode AS barcode,\n"
                                                "           Barcodes.parent AS nomenclature,\n"
                                                "           DocumentsTables.parent AS document,\n"
                                                "           DocumentsTables.quantity AS quantity\n"
                                                "      FROM DocumentsTables AS DocumentsTables\n"
                                                "           LEFT JOIN\n"
                                                "           Barcodes AS Barcodes ON DocumentsTables.barcode = Barcodes.barcode;";


    static inline void fail(const std::string& what, const std::string& error, bool conv = true){
        std::tm tm = arcirk::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        if(conv)
            std::cerr << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(error) << std::endl;
        else
            std::cerr << std::string(cur_date) << " " << what << ": " << error << std::endl;
    };

    static inline void log(const std::string& what, const std::string& message, bool conv = true){
        std::tm tm = arcirk::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        if(conv)
            std::cout << std::string(cur_date) << " " << what << ": " << arcirk::local_8bit(message) << std::endl;
        else
            std::cout << std::string(cur_date) << " " << what << ": " <<message << std::endl;
    };

    static inline nlohmann::json table_default_json(arcirk::database::tables table) {

        //using namespace arcirk::database;
        switch (table) {
            case tbUsers:{
                auto usr_info = user_info();
                usr_info.ref = arcirk::uuids::nil_string_uuid();
                usr_info.parent = arcirk::uuids::nil_string_uuid();
                usr_info.is_group = 0;
                usr_info.deletion_mark = 0;
                return pre::json::to_json(usr_info);
            }
            case tbMessages:{
                auto tbl = messages();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.content_type ="Text";
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
                //std::string tbl_json = to_string(pre::json::to_json(tbl));
                //return tbl_json;
            }
            case tbOrganizations:{
                auto tbl = organizations();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbSubdivisions:{
                auto tbl = subdivisions();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbWarehouses:{
                auto tbl = warehouses();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbPriceTypes:{
                auto tbl = price_types();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbWorkplaces:{
                auto tbl = workplaces();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.server = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbDevices:{
                auto tbl = devices();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.deviceType = "Desktop";
                tbl.address = "127.0.0.1";
                tbl.workplace = arcirk::uuids::nil_string_uuid();
                tbl.price_type = arcirk::uuids::nil_string_uuid();
                tbl.warehouse = arcirk::uuids::nil_string_uuid();
                tbl.subdivision = arcirk::uuids::nil_string_uuid();
                tbl.organization = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbDocumentsTables: {
                auto tbl = document_table();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.price = 0;
                tbl.quantity = 0;
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbDocuments: {
                auto tbl = documents();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.device_id = arcirk::uuids::nil_string_uuid();
                tbl.date = date_to_seconds();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
//                std::string tbl_json = to_string(pre::json::to_json(tbl));
//                return tbl_json;
            }
            case tbNomenclature: {
                auto tbl = nomenclature();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
            }
            case tbBarcodes: {
                auto tbl = barcodes();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.parent = arcirk::uuids::nil_string_uuid();
                tbl.is_group = 0;
                tbl.deletion_mark = 0;
                return pre::json::to_json(tbl);
            }
            case tbDatabaseConfig: {
                auto tbl = database_config();
                tbl.ref = arcirk::uuids::nil_string_uuid();
                tbl.version = 0;
                return pre::json::to_json(tbl);
            }
            case tables_INVALID:{
                break;
            }
            case tbDevicesType:
                return devices_type();
        }

        return {};
    }

    template<typename T>
    static inline T table_default_struct(arcirk::database::tables table){
        auto j = table_default_json(table);
        auto result = pre::json::from_json<T>(j);
        return result;
    }

    static inline std::string get_ddl(tables table, DatabaseType type){
        switch (table) {
            case tbUsers: return type == DatabaseType::dbTypeSQLite ? users_table_ddl : users_odbc_table_ddl;
            case tbMessages: return type == DatabaseType::dbTypeSQLite ? messages_table_ddl : messages_odbc_table_ddl;
            case tbOrganizations: return type == DatabaseType::dbTypeSQLite ? organizations_table_ddl : organizations_odbc_table_ddl;
            case tbSubdivisions: return type == DatabaseType::dbTypeSQLite ? subdivisions_table_ddl : subdivisions_odbc_table_ddl;
            case tbWarehouses: return type == DatabaseType::dbTypeSQLite ? warehouses_table_ddl : warehouses_odbc_table_ddl;
            case tbPriceTypes: return type == DatabaseType::dbTypeSQLite ? price_types_table_ddl : price_types_odbc_table_ddl;
            case tbWorkplaces: return type == DatabaseType::dbTypeSQLite ? workplaces_table_ddl : workplaces_odbc_table_ddl;
            case tbDevices: return type == DatabaseType::dbTypeSQLite ? devices_table_ddl : devices_odbc_table_ddl;
            case tbDocumentsTables: return type == DatabaseType::dbTypeSQLite ? document_table_table_ddl : document_table_odbc_table_ddl;
            case tbDocuments: return type == DatabaseType::dbTypeSQLite ? documents_table_ddl : documents_odbc_table_ddl;
            case tbNomenclature: return type == DatabaseType::dbTypeSQLite ? nomenclature_table_ddl : nomenclature_odbc_table_ddl;
            case tbDatabaseConfig: return type == DatabaseType::dbTypeSQLite ? database_config_table_ddl : database_config_odbc_table_ddl;
            case tbDevicesType:  return type == DatabaseType::dbTypeSQLite ? devices_type_table_ddl : devices_type_odbc_table_ddl;
            case tbBarcodes:  return type == DatabaseType::dbTypeSQLite ? barcodes_table_ddl : barcodes_odbc_table_ddl;
            case tables_INVALID:{
                break;
            }
        }

        return {};
    }

    static inline std::string get_ddl(views table){
        switch (table) {
            case dvDevicesView: return devises_view_ddl;
            case dvDocumentsTableView: return documents_table_view_ddl;
            case views_INVALID:{
                break;
            }
        }

        return {};
    }

    static inline std::map<std::string, table_info_sqlite>  table_info(soci::session& sql, tables table, DatabaseType type, const std::string& database_name = "") {
        using namespace soci;
        std::map<std::string, table_info_sqlite> result{};
        std::string  query = arcirk::str_sample("PRAGMA table_info(\"%1%\");", arcirk::enum_synonym(table));
        std::string c_name = "name";
        std::string c_type = "type";
        if(type == DatabaseType::dbTypeODBC){
            sql << arcirk::str_sample("USE %1%;", database_name);
            query = arcirk::str_sample("SELECT * FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = '%1%'", arcirk::enum_synonym(table));
            c_name = "COLUMN_NAME";
            c_type = "DATA_TYPE";
        }
        //soci::rowset<table_info_sqlite> rs = (sql.prepare << query);
        soci::rowset<row> rs = (sql.prepare << query);
        //for (rowset<table_info_sqlite>::const_iterator it = rs.begin(); it != rs.end(); ++it)

        for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
        {
            //table_info_sqlite info = *it;
            auto info = table_info_sqlite();
            row const& row_ = *it;
            //info.cid = row_.get<int>("cid");
            info.name = row_.get<std::string>(c_name);
            info.type = row_.get<std::string>(c_type);
            result.emplace(info.name, info);
        }
        return result;
    }

    template<typename T>
    static inline T get_value(soci::row const& row, const std::size_t& column_index){
        //не знаю как правильно проверить на null поэтому вот так ...
        try {
            return row.get<T>(column_index);
        }catch (...){
            return {};
        }
    }

    static inline nlohmann::json row_to_json(const soci::row &row){
        using json = nlohmann::json;
        using namespace soci;

        json values{};
        for(std::size_t i = 0; i != row.size(); ++i) {
            const column_properties & props = row.get_properties(i);
            std::string column_name = props.get_name();
            switch(props.get_data_type())
            {
                case dt_string:{
                    auto val = get_value<std::string>(row, i);
                    values[column_name] = val;
                }
                    break;
                case dt_double:{
                    auto val = get_value<double>(row, i);
                    values[column_name] = val;
                }
                    break;
                case dt_integer:{
                    auto val = get_value<int>(row, i);
                    values[column_name] = val;
                }
                    break;
                case dt_long_long:{
                    auto val = get_value<long long>(row, i);
                    values[column_name] = val;
                }
                    break;
                case dt_unsigned_long_long:{
                    auto val = get_value<unsigned long long>(row, i);
                    values[column_name] = val;
                }
                    break;
                case dt_date:
                    //std::tm when = r.get<std::tm>(i);
                    values[column_name] = {};
                    break;
                case dt_blob:
                    values[column_name] = {};
                    break;
                case dt_xml:
                    values[column_name] = {};
                    break;
            }
        }
        return values;
    }

    static inline std::string query_insert(const std::string& table_name, nlohmann::json values){
        std::string result = str_sample("insert into %1% (", table_name);
        std::string string_values;
        std::vector<std::pair<std::string, nlohmann::json>> m_list;
        auto items_ = values.items();
        for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
            if(itr.key() == "_id")
                continue;
            m_list.emplace_back(itr.key(), itr.value());
        }

        for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
            result.append("[" + itr->first + "]");
            std::string value;
            if(itr->second.is_string())
                value = itr->second.get<std::string>();
            else if(itr->second.is_number_float()){
                value = std::to_string(itr->second.get<double>());
                auto index = value.find(",");
                if(index > 0)
                    value.replace(index, 1, ".");
            }
            else if(itr->second.is_number_integer())
                value = std::to_string(itr->second.get<long long>());

            boost::algorithm::erase_all(value, "'");

            if(value.empty())
                string_values.append("''");
            else
                string_values.append(str_sample("'%1%'", value));
            if(itr != (--m_list.cend())){
                result.append(",\n");
                string_values.append(",\n");
            }
        }
        result.append(")\n");
        result.append("values(");
        result.append(string_values);
        result.append(")");

        return result;
    }

    static inline std::map<tables, int> get_release_tables_versions(){
        std::map<tables, int> result;
        result.emplace(tables::tbDatabaseConfig, 2);
        result.emplace(tables::tbNomenclature, 7);
        result.emplace(tables::tbDocuments, 4);
        result.emplace(tables::tbDevices, 3);
        result.emplace(tables::tbMessages, 3);
        result.emplace(tables::tbUsers, 3);
        result.emplace(tables::tbDevicesType, 3);
        result.emplace(tables::tbDocumentsTables, 5);
        result.emplace(tables::tbOrganizations, 3);
        result.emplace(tables::tbPriceTypes, 3);
        result.emplace(tables::tbSubdivisions, 3);
        result.emplace(tables::tbWarehouses, 3);
        result.emplace(tables::tbWorkplaces, 3);
        result.emplace(tables::tbBarcodes, 3);
        return result;
    }

    static inline std::vector<tables> tables_name_array(){
        std::vector<tables> result = {
                tbUsers,
                tbMessages,
                tbOrganizations,
                tbSubdivisions,
                tbWarehouses,
                tbPriceTypes,
                tbWorkplaces,
                tbDevices,
                tbDevicesType,
                tbDocuments,
                tbDocumentsTables,
                tbNomenclature,
                tbDatabaseConfig,
                tbBarcodes
        };
        return result;
    }

    static inline std::vector<views> views_name_array(){
        std::vector<views> result = {
                dvDevicesView,
                dvDocumentsTableView
        };
        return result;
    }

    static inline std::string default_value_ddl(const std::string& table_name, const std::string& name, boost::variant<std::string, int> value){
        std::string sample = "ALTER TABLE [dbo].[%1%] ADD  CONSTRAINT [DF_%1%_%2%]  DEFAULT (%3%) FOR [%2%]";
        std::string val;
        if(value.type() == typeid(std::string))
            val = arcirk::str_sample("'%1%'", boost::get<std::string>(value));
        else if(value.type() == typeid(int))
            val = arcirk::str_sample("(%1%)", std::to_string(boost::get<int>(value)));

        if(val.empty())
            return {};

        return arcirk::str_sample(sample, table_name, name, val);
    }

    static inline std::map<arcirk::database::tables, std::vector<std::string>> get_default_values_ddl(){
        auto tables_arr = tables_name_array(); //Массив имен таблиц
        std::map<arcirk::database::tables, std::vector<std::string>> result;
        for (auto table : tables_arr) {
            std::vector<std::string> ddls;
            ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "version", 0));
            ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "parent", "00000000-0000-0000-0000-000000000000"));
            ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "deletion_mark", 0));
            ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "is_group", 0));

            if(table == tables::tbMessages){
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "content_type", "HTML"));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "unread_messages", 0));
                result.emplace(table, ddls);
            }
            else if(table == tables::tbUsers){
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "hash", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "role", "user"));
                result.emplace(table, ddls);
            }
            else if(table == tables::tbOrganizations || table == tables::tbSubdivisions
            || table == tables::tbWorkplaces || table == tables::tbWarehouses || table == tables::tbPriceTypes
            || table == tables::tbDevices || table == tables::tbNomenclature){
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "cache", " "));
                result.emplace(table, ddls);
                if(table == tables::tbWorkplaces)
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "server", "00000000-0000-0000-0000-000000000000"));
                else if(table == tables::tbDevices) {
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "deviceType", "00000000-0000-0000-0000-000000000000"));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "workplace", "00000000-0000-0000-0000-000000000000"));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "price_type", "00000000-0000-0000-0000-000000000000"));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "warehouse", "00000000-0000-0000-0000-000000000000"));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "subdivision", "00000000-0000-0000-0000-000000000000"));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "organization", "00000000-0000-0000-0000-000000000000"));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "address", "127.0.0.1"));
                }
                else if(table == tables::tbNomenclature) {
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "vendor_code", " "));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "trademark", " "));
                    ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "unit", "шт."));
                }
            }
            else if(table == tables::tbDocuments){
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "cache", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "number", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "xml_type", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "date", 0));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "device_id", "00000000-0000-0000-0000-000000000000"));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "workplace", "00000000-0000-0000-0000-000000000000"));
                result.emplace(table, ddls);
            }
            else if(table == tables::tbDocumentsTables){
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "cache", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "barcode", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "price", 0));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "quantity", 0));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "product", "00000000-0000-0000-0000-000000000000"));
                result.emplace(table, ddls);
            }
            else if(table == tables::tbBarcodes){
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "barcode", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "vendor_code", " "));
                ddls.push_back(default_value_ddl(arcirk::enum_synonym(table), "first_name", " "));
                result.emplace(table, ddls);
            }else
                result.emplace(table, ddls);
        }
        return result;
    }

    static inline void rebase(soci::session& sql, tables table, DatabaseType type, const std::string& database_name = ""){

        using namespace soci;

        std::string table_name = arcirk::enum_synonym(table);
        log("rebase", "start rebase table " + table_name);

        if(type == DatabaseType::dbTypeODBC){
            sql << "use " + database_name;
        }
        std::string temp_query = arcirk::str_sample("create temp table %1%_temp as select * from %1%;", table_name);
        if(type == DatabaseType::dbTypeODBC){
            temp_query = arcirk::str_sample("select * INTO ##%1%_temp  from %1%;", table_name);
        }
        auto def_values = get_default_values_ddl(); // массивы значений по умолчанию для sql server

        auto tr = soci::transaction(sql);
        sql << temp_query;
        sql << arcirk::str_sample("drop table %1%;", table_name);
        sql << get_ddl(table, type);
        if(type == DatabaseType::dbTypeODBC){
            auto def = def_values[table];
            for (auto str : def) {
                sql << str;
            }
        }
        tr.commit();

        auto t_info = table_info(sql, table, type, database_name);
        std::vector<std::string> tmt;

        std::string tem_prefix = "";
        if(type == DatabaseType::dbTypeODBC){
            tem_prefix = "##";
        }

        soci::rowset<soci::row> rs = (sql.prepare << arcirk::str_sample("select * from %1%%2%_temp;", tem_prefix, table_name));

        int count = 0;
        for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
        {
            row const& row = *it;
            nlohmann::json values{};
            count++;
            for(std::size_t i = 0; i != row.size(); ++i)
            {
                const column_properties & props = row.get_properties(i);
                std::string column_name = props.get_name();

                if(t_info.find(column_name) == t_info.end())
                    continue;

                switch(props.get_data_type())
                {
                    case dt_string:{
                        auto val = get_value<std::string>(row, i);
                        values[column_name] = val;
                    }
                        break;
                    case dt_double:{
                        auto val = get_value<double>(row, i);
                        values[column_name] = val;
                    }
                        break;
                    case dt_integer:{
                        auto val = get_value<int>(row, i);
                        values[column_name] = val;
                    }
                        break;
                    case dt_long_long:{
                        auto val = get_value<long long>(row, i);
                        values[column_name] = val;
                    }
                        break;
                    case dt_unsigned_long_long:{
                        auto val = get_value<unsigned long long>(row, i);
                        values[column_name] = val;
                    }
                        break;
                    case dt_date:
                        //std::tm when = r.get<std::tm>(i);
                        break;
                    case dt_blob:
                        break;
                    case dt_xml:
                        break;
                }
            }

            //sql << query_insert(table_name, values);
            tmt.push_back(query_insert(table_name, values));
        }

        auto tr_ = soci::transaction(sql);
//        sql << "select * into " + table_name + " from " + arcirk::str_sample("%1%%2%_temp;", tem_prefix, table_name);
        for (auto const& q: tmt) {
            sql << q;
        }
        if(type == DatabaseType::dbTypeSQLite)
            sql << arcirk::str_sample("drop table if exists %1%_temp;", table_name);
        else
            sql << arcirk::str_sample("drop table ##%1%_temp;", table_name);

        tr_.commit();

        log("rebase", "end rebase table " + table_name);
    }

    static inline std::vector<std::string> get_database_tables(soci::session& sql, DatabaseType type, const nlohmann::json& version){

        std::string query = "SELECT name FROM sqlite_master WHERE type='table';";
        if(type == arcirk::dbTypeODBC){
            std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.value("major", 0)), std::to_string(version.value("minor", 0)), std::to_string(version.value("path", 0)));
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

    static inline std::vector<std::string> get_database_views(soci::session& sql, DatabaseType type, const nlohmann::json& version){

        std::string query = "SELECT name FROM sqlite_master WHERE type='view';";
        if(type == arcirk::dbTypeODBC){
            std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.value("major", 0)), std::to_string(version.value("minor", 0)), std::to_string(version.value("path", 0)));
            sql << "use " + db_name;
            query = "SELECT * FROM sys.objects WHERE type in (N'V')";
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

    static inline void set_admin_default(soci::session& sql){
        using namespace soci;
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
                //sql << "INSERT INTO Users(ref, first, hash, parent, role) VALUES(?, ?, ?, ?, ?)", soci::use(u.ref), soci::use(u.first), soci::use(u.hash), soci::use(u.parent), soci::use(u.role);
                sql << query_insert("Users", pre::json::to_json(u));
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    static inline void set_enums_devices_types(soci::session& sql){
        using namespace soci;
        //заполняем перечисления
        int count = -1;
        sql << "select count(*) from DevicesType", into(count);
        if(count <= 0){
            auto tr = soci::transaction(sql);
            for (int l = 0; l < 5; ++l) {
                auto val = (devices_type)l;
                std::string ref = to_string(arcirk::uuids::random_uuid());
                std::string enum_name = arcirk::enum_synonym(val);
                sql << query_insert("DevicesType", nlohmann::json {
                        {"first", enum_name},
                        {"ref", ref}
                });
            }
            tr.commit();
        }
    }

    static inline void verify_database_views(soci::session& sql, const std::vector<std::string>& db_views, const std::string& view_name,  const std::string& t_ddl, bool is_rebase = false){
        using namespace soci;

        if(std::find(db_views.begin(), db_views.end(), view_name) == db_views.end()) {
            sql << t_ddl;
        }else{
            if(is_rebase){
                auto tr = soci::transaction(sql);
                sql << arcirk::str_sample("drop view %1%", view_name);
                sql << t_ddl;
                tr.commit();
            }
        }
    }

    static inline void verify_database(soci::session& sql, DatabaseType type, const nlohmann::json& version){

        using namespace soci;

        auto release_table_versions = get_release_tables_versions(); //Текущие версии релиза
        auto database_tables = get_database_tables(sql, type, version); //Массив существующих таблиц
        auto database_views = get_database_views(sql, type, version); //Массив существующих представлений
        auto tables_arr = tables_name_array(); //Массив имен таблиц
        auto def_values = get_default_values_ddl(); // массивы значений по умолчанию для sql server
        bool is_rebase = false;

        std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.value("major", 0)), std::to_string(version.value("minor", 0)), std::to_string(version.value("path", 0)));

        if(type == DatabaseType::dbTypeODBC)
            sql << "use " + db_name;

        //Сначала проверим, существует ли таблица версий
        if(std::find(database_tables.begin(), database_tables.end(), arcirk::enum_synonym(tables::tbDatabaseConfig)) == database_tables.end()) {
            auto ddl = get_ddl(tables::tbDatabaseConfig, type);
            sql << ddl;
        }

        //Заполним массив версий для сравнения
        std::map<tables, int> current_table_versions;
        soci::rowset<soci::row> rs = (sql.prepare << arcirk::str_sample("select * from %1%;", arcirk::enum_synonym(tables::tbDatabaseConfig)));
        for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
        {
            row const& row_ = *it;
            nlohmann::json t_name = row_.get<std::string>("first");
            auto t_ver = row_.get<int>("version");
            current_table_versions.emplace(t_name.get<tables>(), t_ver);
        }

        //Выполним реструктуризацию
        for (auto t_name : tables_arr) {
            if(t_name == tables::tbDatabaseConfig)
                continue;
            if(std::find(database_tables.begin(), database_tables.end(), arcirk::enum_synonym(t_name)) == database_tables.end()){
                //Таблицы не существует, просто создаем новую
                auto tr = soci::transaction(sql);
                sql << get_ddl(t_name, type);
                sql << query_insert(arcirk::enum_synonym(tables::tbDatabaseConfig), nlohmann::json{
                        {"first", arcirk::enum_synonym(t_name)},
                        {"version", release_table_versions[t_name]},
                        {"ref", arcirk::uuids::uuid_to_string(arcirk::uuids::random_uuid())}
                });
                if(type == DatabaseType::dbTypeODBC){
                    auto def = def_values[t_name];
                    for (auto str : def) {
                        sql << str;
                    }
                }
                tr.commit();
            }else{
                //Если существует, проверяем версию таблицы, если не совпадает запускаем реструктуризацию
                int current_ver = 0;
                auto itr_ver = current_table_versions.find(t_name);
                if(itr_ver != current_table_versions.end())
                    current_ver = itr_ver->second;
                if(release_table_versions[t_name] != current_ver){
                    rebase(sql, t_name, type, db_name);
                    auto tr = soci::transaction(sql);

                    if(current_ver == 0)
                        sql << query_insert(arcirk::enum_synonym(tables::tbDatabaseConfig), nlohmann::json{
                                {"first", arcirk::enum_synonym(t_name)},
                                {"version", release_table_versions[t_name]},
                                {"ref", arcirk::uuids::uuid_to_string(arcirk::uuids::random_uuid())}
                        });
                    else
                        sql << arcirk::str_sample("update %1% set version='%2%' where [first]='%3%'", arcirk::enum_synonym(tables::tbDatabaseConfig), std::to_string(release_table_versions[t_name]), arcirk::enum_synonym(t_name));
                    tr.commit();
                }
                is_rebase = true; //пересоздадим представления
            }
        }
        //Прочие действия
        set_admin_default(sql);  //проверка на существование пользователя с правами администратора
        set_enums_devices_types(sql); //Заполняем перечисления
        //Проверка представлений
        for (auto view : views_name_array()) {
            verify_database_views(sql, database_views,  arcirk::enum_synonym(view), get_ddl(view), is_rebase);
        }

    }


}

class native_exception : public std::exception
{
public:
    explicit native_exception(const char *msg) : message(msg) {}
    virtual ~native_exception() throw() {}
    virtual const char *what() const throw() { return message.c_str(); }
protected:
    const std::string message;
};

#endif