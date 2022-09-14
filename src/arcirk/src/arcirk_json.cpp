
#include "../arcirk.hpp"
#ifdef USE_RAPIDJSON
#include "rapidjson/filewritestream.h"
#endif

#ifdef USE_RAPIDJSON
namespace arcirk::json{

    bool bJson::read(const std::string &fileName) {

        is_parse_ = false;

        std::ifstream ifs;

        ifs.open(fileName.c_str());
        if (!ifs)
        {
            std::cerr << "bJson::read Error open file\n";
            return false;
        }
        rapidjson::IStreamWrapper isw{ ifs };

        if (this->ParseStream(isw).HasParseError())
            std::cerr << "bJson::read ERROR: encountered a JSON parsing error" << std::endl;
        else
        {
            is_parse_ = true;
        }
        ifs.close();

        return is_parse_;
    }

    bool bJson::write(const std::string &fileName) {

        std::ofstream ofs(fileName);
        rapidjson::OStreamWrapper osw(ofs);

        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
        this->Accept(writer);

        return true;
    }

    bool bJson::value(const std::string &member, bVariant &value, const bVariant& defValue) {

        bValue::ConstMemberIterator itr = this->FindMember(member.c_str());

        if (itr == this->MemberEnd()) {
            value = defValue;
            return false;
        } else {
            if (itr->value.IsInt()) {
                int intVal = itr->value.GetInt();
                value = intVal;
            } else if (itr->value.IsString()) {
                std::string strVal = itr->value.GetString();
                if(member == "uuid_form"){
                    boost::uuids::uuid uuid_form{};
                    if (uuids::is_valid_uuid(strVal, uuid_form))
                        value = uuid_form;
                    else
                        value = strVal;
                }else
                    value = strVal;
            } else if (itr->value.IsBool()) {
                bool boolVal = itr->value.GetBool();
                value = boolVal;
            } else {
                value = "";
            }
        }

        return true;
    }

    bool bJson::value(const std::string &member, arcirk::bValue &value, const arcirk::bValue& defValue) {

        bValue::ConstMemberIterator itr = this->FindMember(member.c_str());

        if (itr == this->MemberEnd()) {
            value = value.CopyFrom(defValue, this->GetAllocator());
            return false;
        } else {
            value.CopyFrom(itr->value, this->GetAllocator());
        }

        return true;
    }

    bVariant bJson::get_member(const std::string &member, const bVariant& defValue) {
        bVariant result;
        value(member, result, defValue);
        return result;
    }

    void bJson::insert(arcirk::content_value val) {

//        bValue::MemberIterator itr = this->FindMember(val.key.c_str());
//
//        if (itr != this->MemberEnd()) {
//            if (val.value.is_string()) {
//                itr->value.SetString(val.value.get_string().c_str(), this->GetAllocator());
//            } else if (val.value.is_bool()) {
//                itr->value.SetBool(val.value.get_bool());
//            } else if (val.value.is_int()) {
//                itr->value.SetInt(val.value.get_int());
//            }else if (val.value.is_uuid()) {
//                itr->value.SetString(val.value.to_string().c_str(), this->GetAllocator());
//            }else{
//                std::cerr << "bJson::append::error: invalid  value" <<std::endl;
//            }
//        }else{
            bValue key(val.key.c_str(), this->GetAllocator());
            if (val.value.is_string()) {
                bValue _value(val.value.get_string().c_str(), this->GetAllocator());
                this->AddMember(key, _value, this->GetAllocator());
            } else if (val.value.is_bool()) {
                this->AddMember(key, val.value.get_bool(), this->GetAllocator());
            } else if (val.value.is_int()) {
                int v = val.value.get_int();
                this->AddMember(key, v, this->GetAllocator());
            }else if (val.value.is_uuid()) {
                bValue _value(val.value.to_string().c_str(), this->GetAllocator());
                this->AddMember(key, _value, this->GetAllocator());
            }else{
                this->AddMember(key, "", this->GetAllocator());
            }
        //}

    }

    void bJson::insert(const std::string &key, arcirk::bValue &val) {
        bValue _key(key.c_str(), this->GetAllocator());
        this->AddMember(_key, val, this->GetAllocator());
    }

    void bJson::insert(const std::string &key, arcirk::bVariant &val) {
        bValue _key(key.c_str(), this->GetAllocator());
        if (val.is_string()) {
            bValue _val(val.get_string().c_str(), this->GetAllocator());
            this->AddMember(_key, _val, this->GetAllocator());
        } else if (val.is_bool()) {
            this->AddMember(_key, val.get_bool(), this->GetAllocator());
        } else if (val.is_int()) {
            int v = val.get_int();
            this->AddMember(_key, v, this->GetAllocator());
        }else if (val.is_uuid()) {
            bValue _val(val.to_string().c_str(), this->GetAllocator());
            this->AddMember(_key, _val, this->GetAllocator());
        }
    }

    void bJson::insert(arcirk::bValue *object, arcirk::content_value val) {

        bValue key(val.key.c_str(), this->GetAllocator());

        if (val.value.is_string()) {
            bValue value(val.value.get_string().c_str(), this->GetAllocator());
            object->AddMember(key, value, this->GetAllocator());
        } else if (val.value.is_bool()) {
            object->AddMember(key, val.value.get_bool(), this->GetAllocator());
        } else if (val.value.is_int()) {
            int v = val.value.get_int();
            object->AddMember(key, v, this->GetAllocator());
        }else if (val.value.is_uuid()) {
            bValue value(val.value.to_string().c_str(), this->GetAllocator());
            object->AddMember(key, value, this->GetAllocator());
        }

    }

    void bJson::push_back(bValue &val) {
        this->PushBack(val, this->GetAllocator());
    }

    void bJson::push_back(bValue &arr, bVariant &val) {
        if (val.is_string()) {
            bValue value(val.get_string().c_str(), this->GetAllocator());
            arr.PushBack(value, this->GetAllocator());
        } else if (val.is_bool()) {
            bool v = val.get_bool();
            arr.PushBack( v, this->GetAllocator());
        } else if (val.is_int()) {
            int v = val.get_int();
            arr.PushBack( v, this->GetAllocator());
        }else if (val.is_uuid()) {
            bValue value(val.get_string().c_str(), this->GetAllocator());
            arr.PushBack( value, this->GetAllocator());
        }
    }

    void bJson::push_back(bValue &arr, bValue &val) {
        arr.PushBack(val, this->GetAllocator());
    }

    void bJson::copy_from(bValue &val) {
        this->CopyFrom(val, this->GetAllocator());
    }

    bool bJson::is_parse() {
        return is_parse_;
    }

    void bJson::set_object() {
        this->SetObject();
    }

    void bJson::set_array() {
        this->SetArray();
    }

    bValue bJson::to_object() {
        if (!this->IsObject())
            return {};
#ifdef _WINDOWS
        return this->GetObjectA();
#else
        bValue val(rapidjson::kObjectType);
        for(auto itr = this->MemberBegin(); itr != this->MemberEnd(); ++itr){
            val.AddMember(itr->name, itr->value, this->GetAllocator());
        }
        return val;
#endif
    }

    bValue bJson::to_array() {

        auto arr = this->GetArray();
        bValue v(rapidjson::kArrayType);
        for (auto item = arr.Begin(); item != arr.End(); item++) {
            if(item->IsArray()){
                v.PushBack(item->GetArray(), this->GetAllocator());
            }else if(item->IsObject()){
                v.PushBack(item->GetObject(), this->GetAllocator());
            }else if(item->IsBool()){
                v.PushBack(item->GetBool(), this->GetAllocator());
            }else if(item->IsInt()){
                v.PushBack(item->GetInt(), this->GetAllocator());
            }else if(item->IsInt64()){
                v.PushBack(item->GetInt64(), this->GetAllocator());
            }else if(item->IsDouble()){
                v.PushBack(item->GetDouble(), this->GetAllocator());
            }else if(item->IsString()){
                v.PushBack(bValue(item->GetString(), this->GetAllocator()), this->GetAllocator());
            }
        }

        return v;
    }

    std::string bJson::to_string(bool base64) const {

        bStringBuffer stringBuffer;
        rapidjson::Writer<bStringBuffer> writer(stringBuffer);
        this->Accept(writer);
        std::string result = stringBuffer.GetString();

        if(!base64)
            return result;
        else{
            return arcirk::base64::base64_encode(result);
        }
    }

    bool bJson::parse(const std::string &jsonText) {

        is_parse_ = !this->Parse(jsonText.c_str()).HasParseError();

        if (!is_parse_) {
            std::cerr << "parse ERROR: encountered a JSON parsing error" << std::endl;
            return false;
        }

        return true;
    }

    unsigned int bJson::count() {
        if(this->IsObject())
            return this->MemberCount();
        else if(this->IsArray())
            return this->GetArray().Size();
        else
            return 0;
    }


}
#endif