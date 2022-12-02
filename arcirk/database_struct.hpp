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

namespace arcirk::database{

    enum roles{
        dbAdministrator,
        dbUser,
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

    enum tables{
        tbUsers,
        tbMessages,
        tables_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(tables, {
        {tables_INVALID, nullptr}    ,
        {tbUsers, "Users"}  ,
        {tbMessages, "Messages"}  ,
    })

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
}

#endif