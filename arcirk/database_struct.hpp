#ifndef ARCIRK_DATABASE_STRUCT_HPP
#define ARCIRK_DATABASE_STRUCT_HPP

#include "includes.hpp"

//BOOST_FUSION_DEFINE_STRUCT(
//        (arcirk::database::query_builder), query_value,
//        (std::string, key)
//        (arcirk::database::sql_type_of_comparison, comparison)
//        (arcirk::bVariant value)
//)

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

}



#endif