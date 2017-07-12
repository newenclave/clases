#pragma once
#ifndef RC_SYSTEM_NET_INTERFACES_H
#define RC_SYSTEM_NET_INTERFACES_H

#ifdef _WIN32
#   include <iphlpapi.h>
#   include <ws2tcpip.h>
#   include <iptypes.h>
#   pragma comment(lib, "iphlpapi.lib")
#   pragma comment(lib, "ws2_32.lib")
#else
#   include <sys/types.h>
#   include <ifaddrs.h>
#endif

#include "boost/asio.hpp"

#include "rs/system/net-interface-info.h"
#include "rs/system/charset.h"
#include "rs/ip/tools.h"

namespace rs { namespace system {

    struct net_interfaces {

    private:

#ifdef _WIN32
        using mib_table_type = std::vector<MIB_IPADDRROW>;
        using pip_table_type = std::vector<PIP_ADAPTER_ADDRESSES>;

        static
        bool get_addresses( net_interface_list &out, int family )
        {
            static const sockaddr_in6 mask0 = { 0 };
            const DWORD flags = 0
                | GAA_FLAG_SKIP_ANYCAST
                | GAA_FLAG_SKIP_MULTICAST
                | GAA_FLAG_SKIP_DNS_SERVER
                ;

            DWORD res = ERROR_BUFFER_OVERFLOW;
            ULONG size = 0;
            std::vector<char> tdata;

            net_interface_list tmp;

            while( res == ERROR_BUFFER_OVERFLOW ) {

                tdata.resize( size + 1 );

                auto p = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&tdata[0]);
                res = GetAdaptersAddresses( family, flags, NULL, p, &size );

                if( res == ERROR_SUCCESS ) {

                    /// PIP_ADAPTER_ADDRESSES_LH has mask
                    if( version::vista_or_higher( ) ) { 

                        using info_type = PIP_ADAPTER_ADDRESSES_LH;

                        auto p = reinterpret_cast<info_type>(&tdata[0]);
                        while( p ) {

                            auto addr = ip::tools::from_sock_addr( 
                                    p->FirstUnicastAddress
                                    ->Address.lpSockaddr );

                            auto family = 
                                p->FirstUnicastAddress
                                    ->Address.lpSockaddr->sa_family;

                            auto mask_bits = p->FirstUnicastAddress
                                ->OnLinkPrefixLength;

                            auto mask = 
                                ip::tools::create_mask( family, mask_bits );

                            tmp.emplace_back( addr, mask,
                                charset::make_mb_string( p->FriendlyName ),
                                p->IfIndex );
                            p = p->Next;
                        }
                    } else { /// PIP_ADAPTER_ADDRESSES_LH has mask
                        using info_type = PIP_ADAPTER_ADDRESSES;
                        auto p = reinterpret_cast<info_type>(&tdata[0]);
                        while( p ) {
                            tmp.emplace_back(
                                p->FirstUnicastAddress->Address.lpSockaddr,
                                reinterpret_cast<const sockaddr *>(&mask0),
                                charset::make_mb_string( p->FriendlyName ),
                                p->IfIndex );
                            p = p->Next;
                        }
                    }

                    out.swap( tmp );
                }
            }
            return res == NO_ERROR;
        }

        static
        std::map<DWORD, std::string> adapter_names( int family )
        {
            net_interface_list lst;
            std::map<DWORD, std::string> res;
            if( get_addresses( lst, family ) ) {
                for( auto &ad: lst ) {
                    res[ad.id( )] = ad.name( );
                }
            }
            return res;
        }

        static
        bool get_addr_table( net_interface_list &out )
        {
            PMIB_IPADDRTABLE ptable = nullptr;
            net_interface_list tmp;

            DWORD size = 0;
            std::vector<char> tmp_data;
            DWORD res = ERROR_INSUFFICIENT_BUFFER;
            while( res == ERROR_INSUFFICIENT_BUFFER ) {
                tmp_data.resize( size + 1 );
                auto buf = reinterpret_cast<PMIB_IPADDRTABLE>(&tmp_data[0]);
                res = GetIpAddrTable( buf, &size, 0 );
                if( res == NO_ERROR ) {
                    ptable = reinterpret_cast<PMIB_IPADDRTABLE>(&tmp_data[0]);
                    auto anames = adapter_names( AF_INET );
                    for( size_t b = 0; b < ptable->dwNumEntries; b++ ) {
                        sockaddr_in addr = { 0 };
                        sockaddr_in mask = { 0 };
                        addr.sin_family = AF_INET;
                        addr.sin_addr.s_addr = ptable->table[b].dwAddr;
                        mask.sin_addr.s_addr = ptable->table[b].dwMask;
                        tmp.emplace_back( 
                            reinterpret_cast<const sockaddr *>(&addr),
                            reinterpret_cast<const sockaddr *>(&mask),
                            anames[ptable->table[b].dwIndex],
                            ptable->table[b].dwIndex );
                    }
                    out.swap( tmp );
                }
            }
            return res == NO_ERROR;
        }

        static
        net_interface_list get_all( )
        {
            net_interface_list v4;
            net_interface_list v6;
            get_addr_table( v4 );
            if( get_addresses( v6, AF_INET6 ) ) {
                v4.insert( v4.end( ), v6.begin( ), v6.end( ) );
            }
            return std::move(v4);
        }

#else

        static
        bool enum_ifaces_( net_interface_list &out )
        {
            ifaddrs *addrs = nullptr;
            int res = ::getifaddrs( &addrs );
            if( 0 != res ) {
                //std::perror( "::getifaddrs" );
                return false;
            }

            net_interface_list tmp;
            ifaddrs *p = addrs;
            size_t id = 0;

            while( p  ) {
                if( p->ifa_addr ) {
                    switch (p->ifa_addr->sa_family) {
                    case AF_INET:
                    case AF_INET6:
                        tmp.emplace_back( p->ifa_addr, p->ifa_netmask,
                                          p->ifa_name, id++ );
                        break;
                    }
                }
                p = p->ifa_next;
            }
            ::freeifaddrs( addrs );
            out.swap( tmp );
            return true;
        }

        static
        net_interface_list get_all( )
        {
            net_interface_list res;
            enum_ifaces_(res);
            return std::move(res);
        }

#endif
    public:
        static 
        net_interface_list enumerate( )
        {
            return std::move( get_all( ) );
        }
    };

    inline std::ostream & operator << ( std::ostream &o, 
                                        const net_interface_info &inf )
    {
        o << inf.name( )  << " " << inf.addr( ) << "/" << inf.mask( );
        return o;
    }

}}

#endif
