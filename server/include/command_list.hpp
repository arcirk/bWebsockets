#ifndef WEBSOCKET_SERVER_COMMANDS
#define WEBSOCKET_SERVER_COMMANDS

#include <iostream>
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>
#include <nlohmann/json.hpp>

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::json), Message,
        (std::string, session_uuid)
        (std::string, user_uuid)
        (std::string, user_name)
        (std::string, recipient_uuid)
        (std::string, command)
        (std::string, message)
        (std::string, param)
        );

namespace arcirk::server{

    enum ServerPublicCommands{
        ServerVersion,
        ServerOnlineClientsList,
        TS_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ServerPublicCommands, {
        {TS_INVALID, nullptr}    ,
        {ServerVersion, "ServerVersion"}  ,
        {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,

    })

    static inline std::string synonym(ServerPublicCommands value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };

}



#endif