#ifndef ARCIRK_DATABASE_STRUCT_HPP
#define ARCIRK_DATABASE_STRUCT_HPP

#include "includes.hpp"

BOOST_FUSION_DEFINE_STRUCT(
(public_struct), user_info,
(int, _id)
(std::string, first)
(std::string, second)
(std::string, ref)
(std::string, hash)
(std::string, role)
(std::string, performance)
(std::string, parent)
(std::string, cache));

#endif