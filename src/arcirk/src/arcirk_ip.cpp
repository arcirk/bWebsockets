//
// Created by arcady on 05.11.2021.
//
#ifdef _WINDOWS
    #pragma warning(disable:4101)
#endif

#include "../arcirk.hpp"

namespace arcirk{

    boost::asio::ip::address_v6 bIp::sinaddr_to_asio(sockaddr_in6 *addr) {
        boost::asio::ip::address_v6::bytes_type buf;
        memcpy(buf.data(), addr->sin6_addr.s6_addr, sizeof(addr->sin6_addr));
        return boost::asio::ip::make_address_v6(buf, addr->sin6_scope_id);
    }

#if defined(_WIN32)
    #undef UNICODE
#include <winsock2.h>
// Headers that need to be included after winsock2.h:
#include <iphlpapi.h>
#include <ws2ipdef.h>

#pragma comment(lib, "Iphlpapi.lib")

typedef IP_ADAPTER_UNICAST_ADDRESS_LH Addr;
typedef IP_ADAPTER_ADDRESSES *AddrList;

std::vector<boost::asio::ip::address> bIp::get_local_interfaces() {
    // It's a windows machine, we assume it has 512KB free memory
    DWORD outBufLen = 1 << 19;
    AddrList ifaddrs = (AddrList) new char[outBufLen];
    struct ifaddrs *ifaddr;

    std::vector<boost::asio::ip::address> res;

    ULONG err = GetAdaptersAddresses(AF_UNSPEC,
        GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, ifaddrs,
        &outBufLen);

    if (err == NO_ERROR) {
        for (AddrList addr = ifaddrs; addr != 0; addr = addr->Next)
            {
            if (addr->OperStatus != IfOperStatusUp) continue;
            // if (addr->NoMulticast) continue;

            // Find the first IPv4 address
            if (addr->Ipv4Enabled) {
                for (Addr *uaddr = addr->FirstUnicastAddress; uaddr != 0; uaddr = uaddr->Next) {
                    if (uaddr->Address.lpSockaddr->sa_family != AF_INET) continue;
                    res.push_back(boost::asio::ip::make_address_v4(ntohl(reinterpret_cast<sockaddr_in *>(uaddr->Address.lpSockaddr)->sin_addr.s_addr)));
                }
            }

            if (addr->Ipv6Enabled) {
                for (Addr *uaddr = addr->FirstUnicastAddress; uaddr != 0; uaddr = uaddr->Next) {
                    if (uaddr->Address.lpSockaddr->sa_family != AF_INET6) continue;
                    res.push_back(sinaddr_to_asio(reinterpret_cast<sockaddr_in6 *>(uaddr->Address.lpSockaddr)));
                }
            }
        }
    } else {

    }
    delete[]((char *)ifaddrs);
    return res;
}
#elif defined(__APPLE__) || defined(__linux__)
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/types.h>

    std::vector<boost::asio::ip::address> bIp::get_local_interfaces() {
        std::vector<boost::asio::ip::address> res;
        ifaddrs *ifs;
        if (getifaddrs(&ifs)) {
            return res;
        }
        for (auto addr = ifs; addr != nullptr; addr = addr->ifa_next) {
            // No address? Skip.
            if (addr->ifa_addr == nullptr) continue;

            // Interface isn't active? Skip.
            if (!(addr->ifa_flags & IFF_UP)) continue;

            if(addr->ifa_addr->sa_family == AF_INET) {
                res.push_back(boost::asio::ip::make_address_v4(ntohl(
                        reinterpret_cast<sockaddr_in *>(addr->ifa_addr)->sin_addr.s_addr)));
            } else if(addr->ifa_addr->sa_family == AF_INET6) {
                res.push_back(sinaddr_to_asio(reinterpret_cast<sockaddr_in6 *>(addr->ifa_addr)));
            } else continue;
        }
        freeifaddrs(ifs);
        return res;
    }
#else
#error "..."
#endif

    std::string bIp::get_default_host(const std::string& def_host, const std::string& seg) {

        if (def_host == "localhost" || def_host == "127.0.0.1"  || def_host == "0.0.0.0") {
            std::vector<boost::asio::ip::address> ip_vec = get_local_interfaces();

            for (auto iter: ip_vec) {
                if (iter.to_string().substr(0, seg.length()) == seg)
                    return iter.to_string();
            }
        }
        return def_host;
    }
}
