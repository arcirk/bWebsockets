//
// Created by Борисоглебский on 21.09.2022.
//

#ifndef ARCIRK_SOLUTION_SHARED_STATE_H
#define ARCIRK_SOLUTION_SHARED_STATE_H

#include <arcirk.hpp>
#include "callbacks.hpp"

class session;

using namespace arcirk;

class shared_state {
    client::bClientData m_data;
public:
    explicit
    shared_state(){};
    explicit
    shared_state(const client::bClientData& param);

    void on_connect(session* sess);
    void on_message(const std::string& message);
    void on_stop();
    void on_error(const std::string &what, const std::string &err, int code);
    void on_status_changed(bool status);

private:


};


#endif //ARCIRK_SOLUTION_CHARED_STATE_H
