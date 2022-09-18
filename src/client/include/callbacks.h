//
// Created by admin on 10.09.2022.
//

#ifndef ARCIRK_SOLUTION_CALLBACKS_H
#define ARCIRK_SOLUTION_CALLBACKS_H

#include <iostream>
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>

typedef std::function<void(const std::string&)> callback_message;
typedef std::function<void(bool)> callback_status;
typedef std::function<void()> callback_connect;
typedef std::function<void(const std::string&, const std::string&, int)> callback_error;
typedef std::function<void()> callback_close;

BOOST_FUSION_DEFINE_STRUCT(
(arcirk), ClientParam,
(std::string, app_name)
(std::string, user_uuid)
(std::string, user_name)
(std::string, hash)
(std::string, host_name)
)

BOOST_FUSION_DEFINE_STRUCT(
(arcirk), ServerResponse,
(std::string, command)
(std::string, message)

)
#endif //ARCIRK_SOLUTION_CALLBACKS_H
