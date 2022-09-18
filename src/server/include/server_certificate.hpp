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
    std::string const dh =
            "-----BEGIN DH PARAMETERS-----\n"
            "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
            "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
            "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
            "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
            "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
            "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
            "-----END DH PARAMETERS-----\n";

    ctx.set_password_callback(
            [](std::size_t,
               boost::asio::ssl::context_base::password_purpose)
            {
                return "test";
            });

    ctx.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::single_dh_use);

    ctx.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2);

    ctx.use_certificate_chain(
            boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
            boost::asio::buffer(key.data(), key.size()),
            boost::asio::ssl::context::file_format::pem);

    ctx.use_tmp_dh(
            boost::asio::buffer(dh.data(), dh.size()));
}


#endif