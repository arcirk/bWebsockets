#ifndef WS_CLIENT_TRANSPORT_HPP
#define WS_CLIENT_TRANSPORT_HPP

#include <arcirk.hpp>
#include <net.hpp>
#include <beast.hpp>

#include <boost/asio/ip/resolver_base.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl.hpp>

namespace arcirk::ws{

}
    class client_transport
    {
    public:
        using response_type = http::response<boost::beast::http::dynamic_body>;

        [[nodiscard]] virtual beast::tcp_stream& stream() = 0;

        void write(http::request<http::string_body>& request)
        {
            handle_write(request);
        }

        void read(response_type& response,
                  boost::beast::error_code& ec)
        {
            handle_read(response, ec);
        }

        virtual boost::beast::error_code set_hostname(const std::string& hostname) { return { }; }
        virtual void handshake() { }

        void connect(const tcp::resolver::results_type results)
        {
            stream().connect(results);
        }

    private:
        virtual void handle_write(http::request<http::string_body>& request) = 0;
        virtual void handle_read(response_type& response,
                                 boost::beast::error_code& ec) = 0;

    protected:
        boost::beast::flat_buffer m_buffer;

    };

    class client_transport_plain :
            public client_transport
    {
    public:
        client_transport_plain(net::io_context& io_ctx) :
                m_stream(io_ctx)
        {
        }

        [[nodiscard]] boost::beast::tcp_stream& stream() override
        {
            return m_stream;
        }

    private:
        void handle_write(http::request<http::string_body>& request) override
        {
            http::
            write(m_stream, request);
        }

        void handle_read(response_type& response,
                         beast::error_code& ec) override
        {
            http::
            read(m_stream, m_buffer, response, ec);
        }

    private:
        boost::beast::tcp_stream m_stream;
    };

    class client_transport_tls :
            public client_transport
    {
    public:
        client_transport_tls(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx) :
                m_io_ctx(io_ctx),
                m_ssl_ctx(ssl_ctx),
                m_stream(io_ctx, ssl_ctx)
        {
        }

        [[nodiscard]] boost::beast::tcp_stream& stream() override
        {
            return m_stream.next_layer();
        }

        boost::beast::error_code set_hostname(const std::string& hostname) override
        {
            // Set SNI Hostname (many hosts need this to handshake successfully)
            if (not SSL_set_tlsext_host_name(m_stream.native_handle(), hostname.c_str())) {
                return beast::error_code{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            }

            return { };
        }

        void handshake() override
        {
            m_stream.handshake(ssl::stream_base::client);
        }

    private:
        void handle_write(http::request<boost::beast::http::string_body>& request) override
        {
            http::
            write(m_stream, request);
        }

        void handle_read(response_type& response,
                         beast::error_code& ec) override
        {
            http::
            read(m_stream, m_buffer, response, ec);
        }

    private:
        net::io_context& m_io_ctx;
        ssl::context& m_ssl_ctx;
        beast::ssl_stream<beast::tcp_stream> m_stream;
    };

#endif