#ifndef ARCIRK_DATABASE_STRUCT_HPP
#define ARCIRK_DATABASE_STRUCT_HPP

#include "includes.hpp"

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
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), organizations,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), subdivisions,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), warehouses,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), price_types,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), workplaces,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, server)

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
        (std::string, price)
        (std::string, warehouse)
        (std::string, subdivision)
        (std::string, organization)
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
        (int, date)
        (std::string, device_id)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), document_table,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, parent)
        (std::string, barcode)
        (double, quantity)
        (double, price)
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
        tbDocumentsTable,
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
        {tbDocumentsTable, "DocumentsTable"}  ,
    })

    enum views{
        dvDevicesView,
        views_INVALID=-1,
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(views, {
        { views_INVALID, nullptr }    ,
        { dvDevicesView, "DevicesView" }  ,
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
                                         "    unread_messages INTEGER   DEFAULT (0) \n"
                                         ");";

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
                                      "                            DEFAULT (0) \n"
                                      ");";

    const std::string organizations_table_ddl = "CREATE TABLE Organizations (\n"
                                           "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                           "    [first]         TEXT,\n"
                                           "    second          TEXT,\n"
                                           "    ref             TEXT (36) UNIQUE\n"
                                           "                             NOT NULL,\n"
                                           "    cache           TEXT      DEFAULT \"\"\n"
                                           ");";
    const std::string subdivisions_table_ddl = "CREATE TABLE Subdivisions (\n"
                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                               "    [first]         TEXT,\n"
                                               "    second          TEXT,\n"
                                               "    ref             TEXT (36) UNIQUE\n"
                                               "                             NOT NULL,\n"
                                               "    cache           TEXT      DEFAULT \"\"\n"
                                               ");";
    const std::string warehouses_table_ddl = "CREATE TABLE Warehouses (\n"
                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                               "    [first]         TEXT,\n"
                                               "    second          TEXT,\n"
                                               "    ref             TEXT (36) UNIQUE\n"
                                               "                             NOT NULL,\n"
                                               "    cache           TEXT      DEFAULT \"\"\n"
                                               ");";
    const std::string price_types_table_ddl = "CREATE TABLE PriceTypes (\n"
                                             "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                             "    [first]         TEXT,\n"
                                             "    second          TEXT,\n"
                                             "    ref             TEXT (36) UNIQUE\n"
                                             "                             NOT NULL,\n"
                                             "    cache           TEXT      DEFAULT \"\"\n"
                                             ");";
    const std::string devices_type_table_ddl = "CREATE TABLE DevicesType (\n"
                                              "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                              "    [first]         TEXT,\n"
                                              "    second          TEXT,\n"
                                              "    ref             TEXT (36) UNIQUE\n"
                                              "                             NOT NULL\n"
                                              ");";

    const std::string workplaces_table_ddl = "CREATE TABLE Workplaces (\n"
                                              "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                              "    [first]         TEXT,\n"
                                              "    second          TEXT,\n"
                                              "    ref             TEXT (36) UNIQUE\n"
                                              "                             NOT NULL,\n"
                                              "    cache           TEXT      DEFAULT \"\",\n"
                                              "    server          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000]\n"
                                              ");";
    const std::string devices_table_ddl = "CREATE TABLE Devices (\n"
                                          "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                          "    [first]         TEXT,\n"
                                          "    second          TEXT,\n"
                                          "    ref             TEXT (36) UNIQUE\n"
                                          "                             NOT NULL,\n"
                                          "    cache           TEXT      DEFAULT \"\",\n"
                                          "    deviceType      TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    address         TEXT      DEFAULT \"127.0.0.1\",\n"
                                          "    workplace       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    price           TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    warehouse       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    subdivision     TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
                                          "    organization    TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000]\n"
                                          ");";

    const std::string devises_view_ddl = "CREATE VIEW DevicesView AS\n"
                                         "    SELECT Devices.ref AS ref,\n"
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
                                         "           PriceTypes ON Devices.price = PriceTypes.ref\n"
                                         "           LEFT JOIN\n"
                                         "           Workplaces ON Devices.workplace = Workplaces.ref;";

    const std::string documents_table_ddl = "CREATE TABLE Documents (\n"
                                          "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                          "    [first]         TEXT,\n"
                                          "    second          TEXT,\n"
                                          "    ref             TEXT (36) UNIQUE\n"
                                          "                             NOT NULL,\n"
                                          "    cache           TEXT      DEFAULT \"\",\n"
                                          "    date            INTEGER,\n"
                                          "    device_id       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000]\n"
                                          ");";

    const std::string document_table_table_ddl = "CREATE TABLE DocumentTable (\n"
                                          "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
                                          "    [first]         TEXT,\n"
                                          "    second          TEXT,\n"
                                          "    ref             TEXT (36) UNIQUE\n"
                                          "                             NOT NULL,\n"
                                          "    cache           TEXT      DEFAULT \"\",\n"
                                          "    price           DOUBLE DEFAULT (0),\n"
                                          "    quantity        DOUBLE DEFAULT (0),\n"
                                          "    barcode         TEXT      DEFAULT \"\",\n"
                                          "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000]\n"
                                          ");";
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