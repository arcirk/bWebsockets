
#include "../arcirk.hpp"

#include <cryptopp/osrng.h>

using CryptoPP::AutoSeededRandomPool;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cstdlib>
using std::exit;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;

#include <cryptopp/hex.h>
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;

#include <cryptopp/aes.h>
using CryptoPP::AES;

#include <cryptopp/modes.h>
using CryptoPP::ECB_Mode;

USING_NAMESPACE(CryptoPP)

const byte aes_key[CryptoPP::AES::MAX_KEYLENGTH] = {
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef
};
const byte aes_iv[CryptoPP::AES::BLOCKSIZE] = {
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef,
        0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
};

CryptoPP::CFB_FIPS_Mode<CryptoPP::AES>::Encryption aes_encryption;
CryptoPP::CFB_FIPS_Mode<CryptoPP::AES>::Decryption aes_decryption;

namespace arcirk::cryptography {

    crypt_utils::crypt_utils()
    {
        aes_encryption.SetKeyWithIV(aes_key, sizeof(aes_key), aes_iv);
        aes_decryption.SetKeyWithIV(aes_key, sizeof(aes_key), aes_iv);
    }

    [[maybe_unused]] std::string crypt_utils::encrypt_string(const std::string& plain_text) const
    {
        std::string plain = arcirk::from_utf(plain_text);
        string cipher;
        try
        {
            StringSource s(plain, true,
                           new StreamTransformationFilter(aes_encryption,
                                                          new StringSink(cipher)
                           ) // StreamTransformationFilter
            ); // StringSource

#if 0
        StreamTransformationFilter filter(e);
        filter.Put((const byte*)plain.data(), plain.size());
        filter.MessageEnd();

        const size_t ret = filter.MaxRetrievable();
        cipher.resize(ret);
        filter.Get((byte*)cipher.data(), cipher.size());
#endif
        }
        catch (const CryptoPP::Exception& e)
        {
            arcirk::fail(__FUNCTION__, e.what());
        }
        if(!cipher.empty())
            return arcirk::to_utf(arcirk::base64::base64_encode(cipher));
        else
            return "";
    }

    std::string crypt_utils::decrypt_string(const std::string& cipher_text) const
    {
        std::string cipher = arcirk::base64::base64_decode(cipher_text);
        string recovered;
        try
        {
            StringSource s(cipher, true,
                           new StreamTransformationFilter(aes_decryption,
                                                          new StringSink(recovered)
                           ) // StreamTransformationFilter
            ); // StringSource

#if 0
            StreamTransformationFilter filter(d);
            filter.Put((const byte*)cipher.data(), cipher.size());
            filter.MessageEnd();

            const size_t ret = filter.MaxRetrievable();
            recovered.resize(ret);
            filter.Get((byte*)recovered.data(), recovered.size());
#endif
        }
        catch (const CryptoPP::Exception& e)
        {
            arcirk::fail(__FUNCTION__, e.what());
        }

        if(!recovered.empty())
            return arcirk::to_utf(recovered);
        else
            return "";
    }
}