#include "../arcirk.hpp"

#ifdef _WINDOWS
#pragma warning(disable:4100)
#pragma warning(disable:4267)
#pragma comment(lib, "advapi32")
#endif

namespace arcirk{
    namespace base64{

        using std::wcout;
        using std::wstring;
        using std::vector;

        static const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

        const std::string base64_padding[] = {"", "==", "="};

        bool byte_is_base64(BYTE c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }

        std::string byte_to_base64(BYTE const* buf, unsigned int bufLen) {
            std::string ret;
            int i = 0;
            int j = 0;
            BYTE char_array_3[3];
            BYTE char_array_4[4];

            while (bufLen--) {
                char_array_3[i++] = *(buf++);
                if (i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for(i = 0; (i <4) ; i++)
                        ret += base64_chars[char_array_4[i]];
                    i = 0;
                }
            }

            if (i)
            {
                for(j = i; j < 3; j++)
                    char_array_3[j] = 0;

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; (j < i + 1); j++)
                    ret += base64_chars[char_array_4[j]];

                while((i++ < 3))
                    ret += '=';
            }

            return ret;
        }


        ByteArray base64_to_byte(std::string const& encoded_string) {
            int in_len = encoded_string.size();
            int i = 0;
            int j = 0;
            int in_ = 0;
            BYTE char_array_4[4], char_array_3[3];
            std::vector<BYTE> ret;

            while (in_len-- && ( encoded_string[in_] != '=') && byte_is_base64(encoded_string[in_])) {
                char_array_4[i++] = encoded_string[in_]; in_++;
                if (i ==4) {
                    for (i = 0; i <4; i++)
                        char_array_4[i] = base64_chars.find(char_array_4[i]);

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        ret.push_back(char_array_3[i]);
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j <4; j++)
                    char_array_4[j] = 0;

                for (j = 0; j <4; j++)
                    char_array_4[j] = base64_chars.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
            }

            return ret;
        }

        void writeFile(const std::string& filename, ByteArray& file_bytes){
            std::ofstream file(filename, std::ios::out|std::ios::binary);
            std::copy(file_bytes.cbegin(), file_bytes.cend(),
                      std::ostream_iterator<unsigned char>(file));
        }

        void readFile(const std::string &filename, ByteArray &result)
        {

            FILE * fp = fopen(filename.c_str(), "rb");

            fseek(fp, 0, SEEK_END);
            size_t flen= ftell(fp);
            fseek(fp, 0, SEEK_SET);

            std::vector<unsigned char> v (flen);

            fread(&v[0], 1, flen, fp);

            fclose(fp);

            result = v;
        }

        std::string base64_encode(const std::string &s) {
            namespace bai = boost::archive::iterators;

            try {
                std::stringstream os;

                // convert binary values to base64 characters
                typedef bai::base64_from_binary
                // retrieve 6 bit integers from a sequence of 8 bit bytes
                <bai::transform_width<const char *, 6, 8>> base64_enc; // compose all the above operations in to a new iterator

                std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
                          std::ostream_iterator<char>(os));

                os << base64_padding[s.size() % 3];
                return os.str();
            }catch (std::exception& e){
                std::cerr << "error: " << e.what() << std::endl;
                return "";
            }

        }

        std::string base64_decode(const std::string &s) {
            namespace bai = boost::archive::iterators;

            try {
                std::stringstream os;

                typedef bai::transform_width<bai::binary_from_base64<const char *>, 8, 6> base64_dec;

                auto size = (unsigned int)s.size();

                // Remove the padding characters, cf. https://svn.boost.org/trac/boost/ticket/5629
                if (size && s[size - 1] == '=') {
                    --size;
                    if (size && s[size - 1] == '=') --size;
                }
                if (size == 0) return {};

                std::copy(base64_dec(s.data()), base64_dec(s.data() + size),
                          std::ostream_iterator<char>(os));

                return os.str();

            }catch (std::exception& e){
                std::cerr << "error: " << e.what() << std::endl;
                return "";
            }
        }

    }
}