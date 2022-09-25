
#include <arcirk.hpp>
#include <map>

template<class Derived>
class websocket_session;
class plain_websocket_session;
class ssl_websocket_session;

class shared_state{
    std::map<boost::uuids::uuid, websocket_session<plain_websocket_session>*> plain_sessions_;
    std::mutex mutex_;
public:
    explicit
    shared_state(){};
    ~shared_state(){};

    //void send();
};