#ifndef SESSION_BASE_HPP
#define SESSION_BASE_HPP

class ws_client;

class session_base{

public:
    virtual void stop() = 0;
    virtual bool is_open() const = 0;

    virtual bool is_ssl() const { return _is_ssl; };

    void use_ssl(bool value) {_is_ssl = value;}

protected:
    bool _is_ssl = false;
    bool started_ = false;
    std::string _auth = "";
    ws_client* client_ = nullptr;
    std::string host_ = "";

};

#endif