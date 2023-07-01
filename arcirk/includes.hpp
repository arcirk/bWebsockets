#ifndef ARCIRK_INCLUDES_HPP
#define ARCIRK_INCLUDES_HPP

#include <iostream>

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/boost-fusion.h>
#include <soci/odbc/soci-odbc.h>

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

#include <string_view>
#include <execution>

#define UNDEFINED std::monostate()

namespace arcirk{
    template<typename T>
    static inline std::string enum_synonym(T value){
        using n_json = nlohmann::json;
        return n_json(value).get<std::string>();
    };

    template<typename T>
    nlohmann::json values_from_param(const nlohmann::json& param){
        if(param.empty())
            return {};
        T e = T();
        auto source = pre::json::to_json(e);
        nlohmann::json result = {};

        if(source.is_object()){
            for (auto itr = source.begin(); itr != source.end() ; ++itr) {
                auto i = param.find(itr.key());
                if( i != param.end()){
                    result[itr.key()] = i.value();
                }
            }
            return result;
        }else
            return {};
    }

    static inline std::string type_string(nlohmann::json::value_t t){
        using json = nlohmann::json;
        if(t == json::value_t::null) return "null";
        else if(t == json::value_t::boolean) return "boolean";
        else if(t == json::value_t::number_integer) return "number_integer";
        else if(t == json::value_t::number_unsigned) return "number_unsigned";
        else if(t == json::value_t::number_float) return "number_float";
        else if(t == json::value_t::object) return "object";
        else if(t == json::value_t::array) return "array";
        else if(t == json::value_t::string) return "string";
        else return "undefined";
    }

    template<typename T>
    T secure_serialization(const nlohmann::json &source)
    {
        using json = nlohmann::json;

        if(!source.is_object())
            return T();

        try {
            auto result = pre::json::from_json<T>(source);
            return result;
        }catch (const std::exception& e){
            fail(__FUNCTION__, e.what());
        }

        nlohmann::json object = pre::json::to_json(T());

        for (auto it = source.items().begin(); it != source.items().end(); ++it) {
            if(object.find(it.key()) != object.end()){
                if(it.value().type() == object[it.key()].type()){
                    object[it.key()] = it.value();
                }else{
                    if(it.value().type() == json::value_t::number_unsigned &&
                            (object[it.key()].type() == json::value_t::number_integer ||
                                    object[it.key()].type() == json::value_t::number_float)){
                        object[it.key()] = it.value();
                    }else{
                        fail(__FUNCTION__, "Ошибка проверки по типу ключа: " + it.key());
                        std::cerr << it.value() << " " << type_string(it.value().type()) << " " << type_string(object[it.key()].type()) <<  std::endl;
                    }
                }
            }
        }

        return pre::json::from_json<T>(object);
    }

    template<typename T>
    T secure_serialization(const std::string &source)
    {
        using json = nlohmann::json;
        try {
            return secure_serialization<T>(json::parse(source));
        } catch (std::exception& e) {
            std::cerr << __FUNCTION__ << e.what() << std::endl;
        }
        return T();
    }

}

#endif