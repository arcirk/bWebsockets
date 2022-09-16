#ifndef DATABASE_STRUCT_HPP
#define DATABASE_STRUCT_HPP

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>

//BOOST_FUSION_DEFINE_STRUCT(
//        (public_struct), user_info,
//        (int, _id)
//                (std::string, first)
//                (std::string, second)
//                (std::string, ref)
//                (std::string, hash)
//                (std::string, role)
//                (std::string, performance)
//                (std::string, parent)
//                (std::string, cache));
//
//BOOST_FUSION_DEFINE_STRUCT(
//        (public_struct), server_settings,
//        (std::string, ServerHost)
//                (int, ServerPort)
//                (std::string, ServerUser)
//                (std::string, ServerUserHash)
//                (std::string, ServerName)
//                (std::string, ServerHttpRoot)
//                (std::string, AutoConnect)
//                (bool, UseLocalWebDavDirectory)
//                (std::string, LocalWebDavDirectory)
//                (std::string, WebDavHost)
//                (std::string, WebDavUser)
//                (std::string, WebDavPwd)
//                (bool, WebDavSSL)
//                (int, SQLFormat)
//                (std::string, SQLHost)
//                (std::string, SQLUser)
//                (std::string, SQLPassword)
//                (std::string, HSHost)
//                (std::string, HSUser)
//                (std::string, HSPassword)
//                (bool, ServerSSL)
//                (std::string, SSL_csr_file)
//                (std::string, SSL_key_file)
//                (bool, UseAuthorization)
//);



#endif //DATABASE_STRUCT_HPP