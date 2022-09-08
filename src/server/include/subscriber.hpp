//
// Created by arcady on 14.07.2021.
//

#ifndef WS_SOLUTION_SUBSCRIBER_H
#define WS_SOLUTION_SUBSCRIBER_H

#include "beast.hpp"
#include "net.hpp"
#include <boost/smart_ptr.hpp>
#include <memory>
#include <iostream>
#include <memory>
#include <set>
#include <map>
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.



class subscriber{

public:

    virtual ~subscriber() = default;
    virtual void deliver(const boost::shared_ptr<const std::string> &msg) = 0;
    virtual boost::uuids::uuid & get_uuid() = 0;
    virtual boost::uuids::uuid & get_user_uuid() = 0;
    virtual const std::string & get_role() = 0;


    [[nodiscard]] std::string get_name() const;
    void set_name(std::string &name);

    [[nodiscard]] bool is_authorized() const{return authorized;};

    void set_disable_notify(bool value);
    bool disable_notify();

    void set_notify_apps(const std::string& value);
    [[nodiscard]] std::string notify_apps() const;

protected:
    boost::uuids::uuid	_uuid{};
    bool authorized = false;
    std::string	_user_name = "не определен";
    bool no_notify = false;
    std::string _notify_apps;
    boost::uuids::uuid	_user_uuid{};
    std::string _role = "user";

private:

};

typedef boost::shared_ptr<subscriber> subscriber_ptr;

#endif //WS_SOLUTION_SUBSCRIBER_H
