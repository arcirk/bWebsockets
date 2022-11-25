#ifndef ARCIRK_INCLUDES_HPP
#define ARCIRK_INCLUDES_HPP

#include <iostream>
#include <exception>
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
#include <string_view>
#include <execution>

#define UNDEFINED std::monostate()

namespace arcirk{
    template<typename T>
    static inline std::string enum_synonym(T value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };
}

#endif