//
// Created by arcady on 05.07.2021.
//

#ifndef BEAST_HPP
#define BEAST_HPP

#ifdef _WINDOWS
    #pragma warning (disable: 4001)
    #pragma warning (disable: 4101)
    #pragma warning (disable: 4061)
#endif // _WINDOWS

#include <boost/beast.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>


#endif //BEAST_HPP
