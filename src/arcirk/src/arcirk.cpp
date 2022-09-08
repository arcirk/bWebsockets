
#include "../arcirk.hpp"

namespace arcirk{
    std::string bVariant::get_string(){
        if (is_string())
            return boost::get<std::string>(value);
        else
            return {};
    }

    long int bVariant::get_int(){
        if (is_int())
            return boost::get<long int>(value);
        else
            return 0;
    }

    double bVariant::get_double(){
        if (is_double())
            return boost::get<double>(value);
        else
            return 0;
    }
    bool bVariant::get_bool(){
        if (is_bool())
            return boost::get<bool>(value);
        else
            return false;
    }

    bool bVariant::is_bool(){
        return value.type() == typeid(bool);
    }

    bool bVariant::is_string(){
        return value.type() == typeid(std::string);
    }

    bool bVariant::is_int(){
        return value.type() == typeid(long int);
    }

    bool bVariant::is_uuid(){
        return value.type() == typeid(boost::uuids::uuid);
    }

    bool bVariant::is_byte(){
        return value.type() == typeid(std::vector<BYTE>);
    }

    std::vector<BYTE> bVariant::get_byte() {
        if (is_byte())
            return boost::get<std::vector<BYTE>>(value);
        else
            return {};
    }

    boost::uuids::uuid bVariant::get_uuid(){
        if (is_uuid())
            return boost::get<boost::uuids::uuid>(value);
        else
        {
            if(is_string()){
                boost::uuids::uuid result = arcirk::uuids::string_to_uuid(get_string());
                if (result != boost::uuids::nil_uuid())
                    return result;
            }
        }

        return boost::uuids::nil_uuid();
    }

    bool bVariant::is_double(){
        return value.type() == typeid(double);
    }

    std::string bVariant::to_string(){
        if (is_string())
            return get_string();
        else if (is_bool())
            return std::to_string(get_bool());
        else if (is_int())
            return std::to_string(get_int());
        else if (is_double())
            return std::to_string(get_double());
        else if (is_uuid()){
            return uuids::uuid_to_string(get_uuid());
        } else if (is_byte()){
            std::vector<BYTE> val = get_byte();
            return base64::byte_to_base64(&val[0], val.size());
        } else
            return {};
    }

}