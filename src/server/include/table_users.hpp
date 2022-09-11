#ifndef TABLEUSERS_HPP
#define TABLEUSERS_HPP
#include "db_pool.hpp"
#include <ctime>
#include <vector>
#include <regex>
#include <numeric>
#include <algorithm>
#include <iomanip>

// некоторые вспомогательные ф-ии для преобразования массивов в векторы и обратно
template<typename T>
static void extract_integers(const std::string& str, std::vector<T>& result ) {
    result.clear();
    using re_iterator = std::regex_iterator<std::string::const_iterator>;
    using re_iterated = re_iterator::value_type;
    std::regex re("([\\+\\-]?\\d+)");
    re_iterator rit(str.begin(), str.end(), re), rend;
    std::transform(rit, rend, std::back_inserter(result), [](const re_iterated& it){return std::stoi(it[1]); });
}

template<typename T>
static void split_integers(std::string& str, const std::vector<T>& arr) {
    str = "{";
    if (arr.size()) {
        str += std::accumulate(arr.begin()+1, arr.end(), std::to_string(arr[0]),
                               [](const std::string& a, T b){return a + ',' + std::to_string(b);});
    } str += "}";
}

// структура таблицы `users'
class user_info {
public:

    int _id; // айди пользователя
    std::string first, second, ref, hash, role, performance, parent, cache; // имя и фамилия

    user_info():_id(0),first(),second(),ref(),hash(),role(),performance(),parent(),cache() {}

    void print() {
//        std::cout.imbue(std::locale("ru_RU.utf8"));
//        std::cout << "id: " << id << std::endl;
//        std::cout << "birthday: " << std::put_time(&birthday, "%c %Z") << std::endl;
//        std::cout << "firstname: " << firstname << std::endl;
//        std::cout << "lastname: " << lastname << std::endl;
//        std::string arr_str;
//        split_integers(arr_str, friends);
//        std::cout << "friends: " << arr_str << std::endl;
    }

    void clear() { _id = 0; first = second = ref  = hash  = role = performance  = parent = cache = "";}

    user_info& operator=(const user_info& rhs) {
        if (this != &rhs) {
            _id = rhs._id;
            first = rhs.first;
            second = rhs.second;
            ref = rhs.ref;
            hash = rhs.hash;
            role = rhs.role;
            performance = rhs.performance;
            parent = rhs.parent;
            cache = rhs.cache;
        }
        return *this;
    }

};

// для работы со своими типами, в SOCI имеются конвертеры
namespace soci {

    template<> struct type_conversion<user_info> {
        typedef values base_type;

        static void from_base(values const& v, indicator ind, user_info& p) {
            if (ind == i_null) return;
            try {
                p._id = v.get<int>("_id", 0);
                p.first = v.get<std::string>("first", {});
                p.second = v.get<std::string>("second", {});
                p.ref = v.get<std::string>("ref", {});
                p.hash = v.get<std::string>("hash", {});
                p.role = v.get<std::string>("role", {});
                p.performance = v.get<std::string>("performance", {});
                p.parent = v.get<std::string>("parent", {});
                p.cache = v.get<std::string>("cache", {});
            } catch (std::exception const & e) { std::cerr << e.what() << std::endl; }
        }

        static void to_base(const user_info& p, values& v, indicator& ind) {
            try {
                v.set("_id", p._id);
                v.set("first", p.first);
                v.set("second", p.second);
                v.set("ref", p.ref);
                v.set("hash", p._id);
                v.set("role", p.first);
                v.set("performance", p.second);
                v.set("parent", p.ref);
                v.set("cache", p.cache);

                ind = i_ok;
                return;
            } catch (std::exception const & e) { std::cerr << e.what() << std::endl; }
            ind = i_null;
        }

    };

}
#endif