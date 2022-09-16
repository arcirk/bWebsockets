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

    virtual const std::string user_name() {return _user_name;};
    virtual const boost::uuids::uuid user_uuid() {return _user_uuid;};
    virtual const std::string & get_role(){return _role;};

//    virtual void throw_authorized() = 0;
//    [[nodiscard]] bool is_authorized() const{return authorized;};

    //virtual ~subscriber() = default;
    //virtual void deliver(const boost::shared_ptr<const std::string> &msg) = 0;





//    [[nodiscard]] std::string get_name() const;
//    void set_name(std::string &name);



//    void set_disable_notify(bool value);
//
//
//    void set_notify_apps(const std::string& value);
//    [[nodiscard]] std::string notify_apps() const;

    //bool use_authorization();

protected:
    std::string	_user_name = "не определен";
    boost::uuids::uuid	_user_uuid{};
    std::string _role = "user";

   //bool authorized = false;
    //bool _use_authorization = false;

private:

};

typedef boost::shared_ptr<subscriber> subscriber_ptr;

#endif //WS_SOLUTION_SUBSCRIBER_H
