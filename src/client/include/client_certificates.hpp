#ifndef CLIENT_CERTIFICATES_HPP
#define CLIENT_CERTIFICATES_HPP

#include <boost/asio/ssl.hpp>
#include <string>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

namespace detail {
    inline
    void
    load_root_certificates(ssl::context& ctx, const std::string& cert , boost::system::error_code& ec)
    {
        ctx.add_certificate_authority(
                boost::asio::buffer(cert.data(), cert.size()), ec);
        if(ec)
            return;
    }

}

inline
void
load_root_certificates(ssl::context& ctx, const std::string& cert, boost::system::error_code& ec)
{
    detail::load_root_certificates(ctx, cert, ec);
}

inline
void
load_root_certificates(ssl::context& ctx, const std::string& cert)
{
    boost::system::error_code ec;
    detail::load_root_certificates(ctx, cert, ec);
    if(ec)
        throw boost::system::system_error{ec};
}

#endif