#ifndef WEBSOCKET_SERVER_COMMANDS
#define WEBSOCKET_SERVER_COMMANDS

#include <iostream>
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>

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

    enum ServerCommands{
        ServerVersion = 0,
        ServerGetClientsList

    };

}



#endif