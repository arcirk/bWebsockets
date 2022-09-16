#ifndef SESSION_BASE_HPP
#define SESSION_BASE_HPP

#include <net.hpp>
#include <beast.hpp>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <boost/bind/bind.hpp>
#include "subscriber.hpp"

class shared_state;

class session_base : public subscriber{

public:
    virtual ~session_base() = default;

    virtual void fail(beast::error_code ec, char const* what) = 0;
//    virtual void on_accept(beast::error_code ec) = 0;
//    virtual void on_read(beast::error_code ec, std::size_t bytes_transferred) = 0;
    //virtual void on_write(beast::error_code ec, std::size_t bytes_transferred) = 0;
    virtual void deliver(const boost::shared_ptr<const std::string> &msg) = 0;
    virtual void
    send(boost::shared_ptr<std::string const> const& ss) = 0;
    virtual bool stopped() const = 0;
    virtual void close() = 0;
//    virtual void
//    on_send(boost::shared_ptr<std::string const> const& ss) = 0;
//    virtual void
//    on_close(beast::error_code ec) = 0;

    virtual const boost::uuids::uuid uuid(){return _uuid;};
    bool disable_notify() const {return _disable_notify;};
    bool authorized() const {return _authorized;};
    void throw_authorized(){
        std::string msg = arcirk::local_8bit("Отказано в доступе!");
        if (!_authorized)
            boost::throw_exception( std::out_of_range( msg ), BOOST_CURRENT_LOCATION );
    };
protected:
    boost::uuids::uuid	_uuid{};
    bool _disable_notify = false;
    std::string _notify_apps;
    bool _authorized;

};

#endif