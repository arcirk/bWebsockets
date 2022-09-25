#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "shared_state.hpp"

class subscriber{
public:
    template<class Derived>
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }
};

template<class Derived>
class websocket_session {
public:
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }
};
class plain_websocket_session
        : public websocket_session<plain_websocket_session>
                , public std::enable_shared_from_this<plain_websocket_session>, public subscriber
{
public:
    void send(boost::shared_ptr<std::string const> const& ss)
    {

    }
};
class ssl_websocket_session
        : public websocket_session<ssl_websocket_session>
                , public std::enable_shared_from_this<ssl_websocket_session>, public subscriber
{
public:
    void send(boost::shared_ptr<std::string const> const& ss)
    {

    }
};

template<class Derived>
class http_session{};

class plain_http_session
        : public http_session<plain_http_session>
                , public std::enable_shared_from_this<plain_http_session>
{};

class ssl_http_session
        : public http_session<ssl_http_session>
                , public std::enable_shared_from_this<ssl_http_session>
{};

class detect_session : public std::enable_shared_from_this<detect_session>
{};

class listener : public std::enable_shared_from_this<listener>
{
    std::map<boost::uuids::uuid, subscriber*> plain_sessions_;
    std::mutex mutex_;

    template<typename T>
    void send(const std::string& message) {

        std::vector<std::weak_ptr<T>> v;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            v.reserve(plain_sessions_.size());
            for(auto p : plain_sessions_)
                v.emplace_back(p.second->template derived<plain_websocket_session>().weak_from_this());
        }
        for(auto const& wp : v)
            if(auto sp = wp.lock()){
                auto const ss = boost::make_shared<std::string const>(std::move(message));
                sp->send(ss);
            }
    }
};

