//
// Created by admin on 10.09.2022.
//

#ifndef ARCIRK_SOLUTION_CALLBACKS_H
#define ARCIRK_SOLUTION_CALLBACKS_H

#include <iostream>
#include <functional>

typedef std::function<void(const std::string&)> callback_message;
typedef std::function<void(bool)> callback_status;
typedef std::function<void()> callback_connect;
typedef std::function<void(const std::string&, const std::string&, int)> callback_error;
typedef std::function<void()> callback_close;

#endif //ARCIRK_SOLUTION_CALLBACKS_H
