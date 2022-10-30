#include "../arcirk.hpp"

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
}