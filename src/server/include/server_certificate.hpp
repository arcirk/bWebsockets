#ifndef ARCIRK_SERVER_CERTIFICATE_HPP
#define ARCIRK_SERVER_CERTIFICATE_HPP

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>

inline
void
load_server_certificate(boost::asio::ssl::context& ctx, const std::string& cert, const std::string& key)
{
    ctx.set_password_callback(
            [](std::size_t,
               boost::asio::ssl::context_base::password_purpose)
            {
                return "test";
            });

//    ctx.set_options(
//            boost::asio::ssl::context::default_workarounds |
//            boost::asio::ssl::context::no_sslv2 |
//            boost::asio::ssl::context::single_dh_use);

    ctx.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2);

    ctx.use_certificate_chain(
            boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
            boost::asio::buffer(key.data(), key.size()),
            boost::asio::ssl::context::file_format::pem);

//    ctx.use_tmp_dh(
//            boost::asio::buffer(dh.data(), dh.size()));
}


#endif