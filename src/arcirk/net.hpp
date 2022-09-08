//
// Created by arcady on 05.07.2021.
//

#ifndef NET_HPP
#define NET_HPP

#ifdef _WINDOWS
    #include <sdkddkver.h>
    #include <boost/asio.hpp>
    #include <windows.h>
    #include <boost/locale.hpp>
#else
    #include <boost/asio.hpp>
#endif //_WINDOWS



namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

#endif //NET_HPP
