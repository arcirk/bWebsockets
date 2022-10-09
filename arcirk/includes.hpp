#ifndef ARCIRK_INCLUDES_HPP
#define ARCIRK_INCLUDES_HPP

#include <iostream>
#include <functional>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>
#include <nlohmann/json.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#define UNDEFINED std::monostate()

#endif