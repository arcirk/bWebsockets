#include "../arcirk.hpp"

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace arcirk{

    namespace uuids{

        bool is_valid_uuid(std::string const& maybe_uuid, boost::uuids::uuid& result) {
            using namespace boost::uuids;

            try {
                result = string_generator()(maybe_uuid);
                return result.version() != uuid::version_unknown;
            } catch(...) {
                return false;
            }
        }
        std::string nil_string_uuid(){
            return "00000000-0000-0000-0000-000000000000";
        }

        boost::uuids::uuid string_to_uuid(const std::string& sz_uuid, bool random_uuid) {

            if (sz_uuid == nil_string_uuid() && random_uuid)
                return boost::uuids::random_generator()();

            boost::uuids::uuid uuid{};

            if (is_valid_uuid(sz_uuid, uuid)){
                return uuid;
            }

            if (!random_uuid)
                return boost::uuids::nil_uuid();
            else
                return boost::uuids::random_generator()();
        }

        boost::uuids::uuid nil_uuid(){
            return boost::uuids::nil_uuid();
        }

        std::string uuid_to_string(const boost::uuids::uuid& uuid){
            try {
                return boost::to_string(uuid);
            } catch(...) {
                return nil_string_uuid();
            }
        }

        boost::uuids::uuid random_uuid(){
            return boost::uuids::random_generator()();
        }

        ByteArray to_byte_array(const boost::uuids::uuid& uuid){
            ByteArray v(uuid.size());
            std::copy(uuid.begin(), uuid.end(), v.begin());
            return v;
        }
        boost::uuids::uuid from_byte_array(const ByteArray& byte){
            boost::uuids::uuid u;
            memcpy(&u, byte.data(), byte.size());
            return u;
        }
    }

    namespace standard_paths{
        std::string home(){
            using namespace boost::filesystem;
#ifdef _WINDOWS
            path p(arcirk::to_utf(getenv("USERPROFILE")));
            return p.string();
#else
            return getenv("HOME");
#endif
        }

        std::string home_roaming_dir(){
            using namespace boost::filesystem;
#ifdef _WINDOWS
            path p(arcirk::to_utf(getenv("APPDATA")));
            return p.string();
#else
            return home();
#endif
        }

        std::string home_local_dir(){
            using namespace boost::filesystem;
#ifdef _WINDOWS
            path p(arcirk::to_utf(getenv("LOCALAPPDATA")));
            return p.string();
#else
            return home();
#endif
        }

        std::string program_data_dir(){
            using namespace boost::filesystem;
#ifdef _WINDOWS
            path p(arcirk::to_utf(getenv("ProgramData")));
            return p.string();
#else
            return home();
#endif
        }
        bool verify_directory(const std::string& dir_path) {
            using namespace boost::filesystem;
            path p(arcirk::local_8bit(dir_path));

            if (!exists(p)) {
                try {
                    return boost::filesystem::create_directories(p);
                }catch (std::exception& e){
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            return true;

        }
        bool verify_directory(const boost::filesystem::path& dir_path) {
            using namespace boost::filesystem;
            path p(arcirk::local_8bit(dir_path.string()));
            if (!exists(p)) {
                try {
                    return boost::filesystem::create_directories(p);
                }catch (std::exception& e){
                    std::cerr << e.what() << std::endl;
                    return false;
                }

            }
            return true;
        }

        std::string this_application_conf_dir(const std::string& app_name, bool mkdir_is_not_exists){
            boost::filesystem::path app_conf(home_roaming_dir());
            app_conf /= app_name;
            if (mkdir_is_not_exists)
                verify_directory(app_conf);
            return app_conf.string();
        }

        std::string this_server_conf_dir(const std::string& app_name, bool mkdir_is_not_exists){
            boost::filesystem::path app_conf(program_data_dir());
            app_conf /= app_name;
            if (mkdir_is_not_exists)
                verify_directory(app_conf);
            return app_conf.string();
        }

    }

    std::string local_8bit(const std::string& source){
#ifdef _WINDOWS
        return boost::locale::conv::from_utf(source, "windows-1251");
#else
        return source;
#endif
    }

    std::string to_utf(const std::string& source){
#ifdef _WINDOWS
        return boost::locale::conv::to_utf<char>(source, "Windows-1251");
#else
        return source;
#endif
    }

    std::string get_sha1(const std::string& p_arg)
    {
        boost::uuids::detail::sha1 sha1;
        sha1.process_bytes(p_arg.data(), p_arg.size());
        unsigned hash[5] = {0};
        sha1.get_digest(hash);

        // Back to string
        char buf[41] = {0};

        for (int i = 0; i < 5; i++)
        {
            std::sprintf(buf + (i << 3), "%08x", hash[i]);
        }

        return std::string(buf);
    }

    std::string get_hash(const std::string& first, const std::string& second){
        std::string _usr(first);
        const std::string& _pwd(second);

        boost::trim(_usr);
        boost::to_upper(_usr);

        return get_sha1(_usr + _pwd);
    }

    int split_str_to_vec(const T_str& s, const T_str& DELIM, T_vec& v)
    {
        size_t l, r;

        for (l = s.find_first_not_of(DELIM), r = s.find_first_of(DELIM, l);
             l != std::string::npos; l = s.find_first_not_of(DELIM, r), r = s.find_first_of(DELIM, l))
            v.push_back(s.substr(l, r - l));
        return (int)v.size();
    }

    T_vec split(const T_str& line, const T_str& sep)
    {
        T_vec  v;

        split_str_to_vec(line, sep, v);

        return v;
    }

    tm current_date() {
        using namespace std;
        tm current{};
        time_t t = time(nullptr);
#ifdef _WINDOWS
        localtime_s(&current, &t);
#else
        localtime_r(&t, &current);
#endif
        return current;
    }

    long int date_to_seconds(const tm& dt, bool offset) {

        tm current = dt;
        time_t t = time(nullptr);

#ifdef _WIN32
        localtime_s(&current, &t);
#else
        localtime_r(&t, &current);
#endif

        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(mktime(&current));

        auto i_offset = offset ? tz_offset() : 0;

        return
                (long int)std::chrono::duration_cast<std::chrono::seconds>(
                        tp.time_since_epoch()).count() + i_offset;

    }

    long int tz_offset(time_t when)
    {
        if (when == NULL_TIME)
            when = std::time(nullptr);
        auto const tm = *std::localtime(&when);
        std::ostringstream os;
        os << std::put_time(&tm, "%z");
        std::string s = os.str();
        // s is in ISO 8601 format: "±HHMM"
        int h = std::stoi(s.substr(0,3), nullptr, 10);
        int m = std::stoi(s[0]+s.substr(3), nullptr, 10);

        return (h-1) * 3600 + m * 60;
    }

    long int add_day(const long int& dt, const int& quantity){
        return dt + (quantity * (60*60*24));
    }

    void trim(std::string& source){ boost::trim(source);};
    void to_upper(std::string& source){boost::to_upper(source);};
    void to_lower(std::string& source){boost::to_lower(source);};

    void* _crypt(void* data, unsigned data_size, void* key, unsigned key_size)
    {
        assert(data && data_size);
        if (!key || !key_size) return data;

        auto* kptr = (uint8_t*)key; // начало ключа
        uint8_t* eptr = kptr + key_size; // конец ключа

        for (auto* dptr = (uint8_t*)data; data_size--; dptr++)
        {
            *dptr ^= *kptr++;
            if (kptr == eptr) kptr = (uint8_t*)key; // переход на начало ключа
        }
        return data;
    }

    std::string crypt(const std::string &source, const std::string& key) {

        void * text = (void *) source.c_str();
        void * pass = (void *) key.c_str();
        _crypt(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));

        std::string result((char*)text);

        return result;
    }

    std::string byte_array_to_string(const ByteArray& data){
        return std::string(data.begin(), data.end());
    }

    ByteArray string_to_byte_array(const std::string& str){
        return ByteArray(str.begin(), str.end());
    }

    std::string left(const std::string &source, const std::string::size_type& count){
        return  source.substr(0, count);
    }

    std::string right(const std::string &source, const std::string::size_type& start){
        return  source.substr(start, source.length());
    }

    T_list parse_section_ini(const std::string& source, const std::string& sep){

        T_vec vec = split(source, sep);

        T_list result;

        for (auto itr = vec.begin(); itr != vec.end() ; ++itr) {
            auto index = itr->find_first_of('=');
            if(index != std::string::npos){
                auto first = left(*itr, index);
                auto second = right(*itr, index + 1);
                result.push_back(std::pair<T_str,T_str>(first, second));
            }
        }
        return result;
    }
}