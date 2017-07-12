#pragma once
#ifndef RC_SYSTEM_NET_ARP_H
#define RC_SYSTEM_NET_ARP_H

#ifdef _WIN32

#else
#   include <sys/types.h>
#   include <ifaddrs.h>
#   include <net/if_arp.h>
#   include <arpa/inet.h>
#   include <sys/ioctl.h>
#endif

#include <string>
#include <cstdint>

namespace rs { namespace system {

    struct arp_adddress {
        std::uint8_t addr[6];

        arp_adddress( )
        {
            addr[0] = addr[1] = addr[2] =
            addr[3] = addr[4] = addr[5] = 0;
        }

        std::string to_string( )
        {
            char buff[24];
            sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
                (addr[0] & 0xff), (addr[1] & 0xff), (addr[2] & 0xff),
                (addr[3] & 0xff), (addr[4] & 0xff), (addr[5] & 0xff));
            return std::string( &buff[0] );
        }
    };

#ifdef _WIN32
    struct net_arp {
        static
        arp_adddress get( const std::string &iface, std::uint32_t ipv4 )
        {
            return arp_adddress( );
        }
#else
    struct net_arp {
        static
        arp_adddress get( const std::string &iface, std::uint32_t ipv4 )
        {
            arp_adddress res;
            arpreq       areq;
            int          s;

            auto sin = reinterpret_cast<sockaddr_in *>(&areq.arp_pa);

            memset( &areq, 0, sizeof(areq) );

            sin->sin_family         = AF_INET;
            sin->sin_addr.s_addr    = ipv4;

            sin = reinterpret_cast<sockaddr_in *>(&areq.arp_ha);
            sin->sin_family         = ARPHRD_ETHER;

            strncpy( areq.arp_dev, iface.c_str( ), 15 );

            if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                return res;
            }

            if( ioctl(s, SIOCGARP, reinterpret_cast<caddr_t>(&areq)) == -1 ) {
                close( s );
                return res;
            }

            close( s );

            res.addr[0] = areq.arp_ha.sa_data[0];
            res.addr[1] = areq.arp_ha.sa_data[1];
            res.addr[2] = areq.arp_ha.sa_data[2];
            res.addr[3] = areq.arp_ha.sa_data[3];
            res.addr[4] = areq.arp_ha.sa_data[4];
            res.addr[5] = areq.arp_ha.sa_data[5];

            return res;
        }
    };

#endif

}}

#endif // NETARP_H
