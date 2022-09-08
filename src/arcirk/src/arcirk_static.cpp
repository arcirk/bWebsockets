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
        return boost::locale::conv::to_utf<char>(source, "Windows-1251");
    }

}