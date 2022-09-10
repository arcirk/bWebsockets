//
// Created by admin on 10.09.2022.
//
#include <iostream>
#include <arcirk.hpp>
#include <bclient.h>
#include <functional>

#include "../include/callbacks.h"
bClient * client;

void on_connect(){
    std::cout << "websocket on_connect" << std::endl;
}

void on_message(const std::string& message){
    std::cout << "websocket on_message: " << message << std::endl;
}

void on_stop(){
    std::cout << "websocket on_stop" << std::endl;
}
void
on_error(const std::string &what, const std::string &err, int code){
    std::cerr << what << "(" << code << "): " << arcirk::local_8bit(err) << std::endl;
}

void on_status_changed(bool status){
    std::cout << "websocket on_status_changed: " << status << std::endl;
}

int
main(int argc, char* argv[]){

    setlocale(LC_ALL, "Russian");

    callback_connect _connect = std::bind(&on_connect);
    callback_error _err = std::bind(&on_error, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    callback_message _message = std::bind(&on_message, std::placeholders::_1);
    callback_status _status = std::bind(&on_status_changed, std::placeholders::_1);
    callback_close _on_close = std::bind(&on_stop);

    client = new bClient("192.168.43.4", 8080, _message, _status, _connect, _on_close, _err);
    std::string line;

    std::string auth_cred = "admin:admin";
    std::string encoded_auth_cred = "Basic " + arcirk::base64::base64_encode(auth_cred);

    while (getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        else if (line == "start")
        {
            client->open(encoded_auth_cred);
        }
        else if (line == "stop")
        {
            client->close(false);
        }
    }
    return EXIT_SUCCESS;
}