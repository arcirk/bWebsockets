#ifndef ARCIRK_LIBRARY_H
#define ARCIRK_LIBRARY_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

namespace arcirk {

    typedef rapidjson::GenericDocument<rapidjson::UTF8<> > bDocument;
    typedef rapidjson::GenericValue<rapidjson::UTF8<> > bValue;
    typedef rapidjson::GenericStringBuffer<rapidjson::UTF8<> > bStringBuffer;
    typedef unsigned char BYTE;
    typedef std::vector<BYTE> ByteArray;
    typedef boost::variant<std::string, long int, bool, double, boost::uuids::uuid, std::vector<BYTE>> Variant;
    typedef std::string T_str;
    typedef std::vector<T_str> T_vec;

    std::string local_8bit(const std::string& source);
    std::string to_utf(const std::string& source);

    namespace uuids{
        bool is_valid_uuid(std::string const& maybe_uuid, boost::uuids::uuid& result);
        boost::uuids::uuid string_to_uuid(const std::string& sz_uuid, bool random_uuid = false);
        std::string nil_string_uuid();
        boost::uuids::uuid nil_uuid();
        std::string uuid_to_string(const boost::uuids::uuid& uuid);
        boost::uuids::uuid random_uuid();
    }

    namespace standard_paths{
        std::string home();
        std::string home_roaming_dir();
        std::string home_local_dir();
        std::string program_data_dir();
        bool verify_directory(const std::string& dir);
        bool verify_directory(const boost::filesystem::path& dir_path);
        std::string this_application_conf_dir(const std::string& app_name, bool mkdir_is_not_exists = true);
        std::string this_server_conf_dir(const std::string& app_name, bool mkdir_is_not_exists = true);
    }

    namespace base64{
        bool byte_is_base64(BYTE c);
        std::string byte_to_base64(BYTE const* buf, unsigned int bufLen);
        ByteArray base64_to_byte(std::string const& encoded_string);
        void writeFile(const std::string& filename, ByteArray& file_bytes);
        void readFile(const std::string &filename, ByteArray &result);
        std::string base64_encode(const std::string &s);
        std::string base64_decode(const std::string &s);
    };

    class bVariant {
    public:
        bVariant(const std::string &val) : value(val) {}
        bVariant(const char *val) : value(std::string(val)) {}
        bVariant(long int val) : value((long int) val) {}
        bVariant(double val) : value((double) val) {}
        bVariant(int val) : value((long int) val) {}
        bVariant(bool val) : value((bool) val) {}
        bVariant(boost::uuids::uuid val) : value(val) {}
        bVariant(std::vector<BYTE> val) : value(val) {}

        bVariant() = default;

        std::string get_string();
        long int get_int();
        double get_double();
        bool get_bool();
        bool is_bool();
        bool is_string();
        bool is_int();
        bool is_uuid();
        boost::uuids::uuid get_uuid();
        bool is_double();
        bool is_byte();
        std::vector<BYTE> get_byte();
        std::string to_string();

    private:
        Variant value;
    };

    typedef struct content_value {
        std::string key;
        bVariant value;

        content_value(std::string key_, const bVariant &val)
                : key(std::move(key_)) { value = val; }

        content_value() = default;

    } content_value;

    namespace json{

        class bJson : public bDocument {

        public:
            bool read(const std::string& fileName);
            bool write(const std::string& fileName);

            bool value(const std::string &member, bVariant &val, const bVariant& defValue = "");
            bool value(const std::string &member, bValue& val, const bValue& defValue = {});

            bVariant get_member(const std::string &member, const bVariant& defValue = "");

            void insert(const std::string& key, bValue& val);
            void insert(const std::string& key, bVariant& val);
            void insert(content_value val);
            void insert(bValue* object, content_value val);

            void push_back(bValue &val);
            void push_back(bValue &arr, bVariant& val);
            void push_back(bValue &arr, bValue& val);

            void copy_from(bValue& val);

            bool is_parse();

            void set_object();
            void set_array();

            bValue to_object();
            bValue to_array();

            [[nodiscard]] std::string to_string(bool base64 = false) const;

            bool parse(const std::string &jsonText);

            unsigned int count();

        private:
            bool is_parse_ = false;
        };

    }



    struct Uri
    {
    public:
        std::string QueryString, Path, Protocol, Host, Port;

        static Uri Parse(const std::string &uri)
        {
            Uri result;

            typedef std::string::const_iterator iterator_t;

            if (uri.length() == 0)
                return result;

            iterator_t uriEnd = uri.end();

            // get query start
            iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

            // protocol
            iterator_t protocolStart = uri.begin();
            iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');            //"://");

            if (protocolEnd != uriEnd)
            {
                std::string prot = &*(protocolEnd);
                if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
                {
                    result.Protocol = std::string(protocolStart, protocolEnd);
                    protocolEnd += 3;   //      ://
                }
                else
                    protocolEnd = uri.begin();  // no protocol
            }
            else
                protocolEnd = uri.begin();  // no protocol

            // host
            iterator_t hostStart = protocolEnd;
            iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

            iterator_t hostEnd = std::find(protocolEnd,
                                           (pathStart != uriEnd) ? pathStart : queryStart,
                                           ':');  // check for port

            result.Host = std::string(hostStart, hostEnd);

            // port
            if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
            {
                hostEnd++;
                iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
                result.Port = std::string(hostEnd, portEnd);
            }

            // path
            if (pathStart != uriEnd)
                result.Path = std::string(pathStart, queryStart);

            // query
            if (queryStart != uriEnd)
                result.QueryString = std::string(queryStart, uri.end());

            return result;

        }   // Parse
    };  // uri

    class bIp{
    public:
        static boost::asio::ip::address_v6 sinaddr_to_asio(sockaddr_in6 *addr);
        static std::vector<boost::asio::ip::address> get_local_interfaces();
        static std::string get_default_host(const std::string& def_host,  const std::string& seg = "192.168");
    };

    class bConf{

    public:
        explicit
        bConf(const boost::filesystem::path& root, const std::string& app_name, std::vector<std::string>& aliases);

        bVariant & operator[] (int index);
        bVariant const& operator[] (int index) const;

        void save();
        bool parse();

        [[nodiscard]] bVariant get(int index) const;
        void set(int index, const bVariant& value);

        [[nodiscard]] std::string to_string() const;

    private:
        std::vector<std::string> m_aliases{};
        std::vector<bVariant> m_vec{};
        boost::filesystem::path m_file_name;
        boost::filesystem::path m_root;
    };

}

#endif //ARCIRK_LIBRARY_H