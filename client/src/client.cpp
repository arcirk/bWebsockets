#include "../include/client.hpp"
#include <common/root_certificates.hpp>
#include "../include/session.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

websocket_client::websocket_client(ssl::context& ctx, client::ClientParam &client_param)
: ctx_(ctx)
{
    state_ = nullptr;
}

void websocket_client::connect(const client::bClientEvent &event, const client::callbacks &f) {
    if(event == client::bClientEvent::wsClose){
        m_data.on_close= boost::get<callback_close>(f);
    }else if(event == client::bClientEvent::wsConnect){
        m_data.on_connect = boost::get<callback_connect>(f);
    }else if(event == client::bClientEvent::wsError){
        m_data.on_error = boost::get<callback_error>(f);
    }else if(event == client::bClientEvent::wsMessage){
        m_data.on_message = boost::get<callback_message>(f);
    }else if(event == client::bClientEvent::wsStatusChanged){
        m_data.on_status_changed = boost::get<callback_status>(f);
    }
}

void websocket_client::open(const Uri &url) {

    ssl::context ctx{ssl::context::tlsv12_client};
   // bool _ssl = url.Protocol == "wss";
    set_certificates(ctx);
    boost::thread(boost::bind(&websocket_client::start, this, url)).detach();

}

void websocket_client::start(arcirk::Uri &url) {

    boost::asio::io_context ioc;
    bool _ssl = url.Protocol == "wss";
    set_certificates(ctx_);
    state_ = boost::make_shared<shared_state>();
    state_->set_basic_auth_string(url.BasicAuth);
    state_->connect(client::bClientEvent::wsMessage, (callback_message)std::bind(&websocket_client::on_message, this, std::placeholders::_1));
    state_->connect(client::bClientEvent::wsStatusChanged, (callback_status)std::bind(&websocket_client::on_status_changed, this, std::placeholders::_1));
    state_->connect(client::bClientEvent::wsError, (callback_error)std::bind(&websocket_client::on_error, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    state_->connect(client::bClientEvent::wsConnect,  (callback_connect)std::bind(&websocket_client::on_connect, this));
    state_->connect(client::bClientEvent::wsClose, (callback_close)std::bind(&websocket_client::on_close, this));
    std::make_shared<resolver>(ioc, ctx_, state_)->run(url.Host.c_str(), url.Port.c_str(), _ssl);

    ioc.run();

    state_ = nullptr;

    std::cout << "exit thread" << std::endl;
}

void websocket_client::set_certificates(ssl::context& ctx) {

    using namespace boost::filesystem;

    if(!exists(certificate_file_))
        return;

    std::string _cert;
    std::ifstream c_in(certificate_file_.string());
    std::ostringstream c_oss;
    c_oss << c_in.rdbuf();
    _cert = c_oss.str();

    if(_cert.empty()){
        std::cerr << "error read certificate files" << std::endl;
        return;
    }

    load_root_default_certificates(ctx);
}

void websocket_client::set_certificate_file(const std::string &file) {
    certificate_file_ = file;
}

void websocket_client::on_close() {
    if(m_data.on_close)
        m_data.on_close();
}

void websocket_client::on_connect() {
    if(m_data.on_connect)
        m_data.on_connect();
}

void websocket_client::on_error(const std::string &what, const std::string &err, int code) {
    if(m_data.on_error)
        m_data.on_error(what, err, code);
}

void websocket_client::on_message(const std::string &message) {
    if(m_data.on_message)
        m_data.on_message(message);
}

void websocket_client::on_status_changed(bool status) {
    if(m_data.on_status_changed)
        m_data.on_status_changed(status);
}

void websocket_client::close(bool disable_notify) {

    if(!state_)
        return;

    state_->close(disable_notify);
}

void websocket_client::send_message(const std::string &message) {
    if(!state_)
        return;
    auto const ss = boost::make_shared<std::string const>(std::move(message));
    state_->send(ss);
}