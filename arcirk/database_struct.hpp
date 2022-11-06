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
(std::string, cache));

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
    static inline std::string synonym(roles value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };

    enum text_type{
        dbString,
        dbHtmlString,
        dbFormattedString,
        text_type_INVALID=-1,
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(text_type, {
        {text_type_INVALID, nullptr}    ,
        {dbString, "Text"}  ,
        {dbHtmlString, "HtmlText"}  ,
        {dbFormattedString, "FormattedString"}  ,
    })
    static inline std::string synonym(text_type value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    }

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

    static inline std::string synonym(tables value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    }
}

#endif