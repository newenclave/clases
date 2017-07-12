#pragma once
#ifndef RC_SYSTEM_NET_INTERFAC_INFO_H
#define RC_SYSTEM_NET_INTERFAC_INFO_H

#include "boost/asio.hpp"

#include "rs/system/version.h"
#include "rs/ip/tools.h"

namespace rs { namespace system {

    class net_interface_info {

        using address_type = boost::asio::ip::address;
        using v4_type      = boost::asio::ip::address_v4;
        using v6_type      = boost::asio::ip::address_v6;
        address_type         sockaddr_;
        //address_type         dest_sockaddr_;
        address_type         mask_;
        std::string          name_;
        size_t               id_;

    public:

        net_interface_info( const sockaddr *sa, const sockaddr *mask,
                            const std::string &name, size_t id )
            :sockaddr_(ip::tools::from_sock_addr(sa))
            ,mask_(ip::tools::from_sock_addr( mask ))
            ,name_(name)
            ,id_(id)
        { }

        net_interface_info( const address_type &sa, const address_type &mask,
                            const std::string &name, size_t id )
            :sockaddr_(sa)
            ,mask_(mask)
            ,name_( name )
            ,id_( id )
        { }

        const std::string &name( ) const
        {
            return name_;
        }

        size_t id( ) const
        {
            return id_;
        }

        const address_type &addr( ) const
        {
            return sockaddr_;
        }

        const address_type &mask( ) const
        {
            return mask_;
        }

        bool is_v4( ) const
        {
            return sockaddr_.is_v4( );
        }

        bool is_v6( ) const
        {
            return sockaddr_.is_v6( );
        }

        v4_type v4( )
        {
            return std::move(sockaddr_.to_v4( ));
        }

        v6_type v6( )
        {
            return std::move(sockaddr_.to_v6( ));
        }

        bool check( const address_type &test ) const
        {
            bool res = false;
            if( sockaddr_.is_v4( ) && test.is_v4( ) ) {
                auto sa = sockaddr_.to_v4( ).to_ulong( );
                auto ma = mask_.to_v4( ).to_ulong( );
                auto ta = test.to_v4( ).to_ulong( );
                res = ((sa & ma) == (ta & ma));
            } else if( sockaddr_.is_v6( ) && test.is_v6( ) ) {

#ifdef _WIN32
                bool has_mask = version::vista_or_higher( );
#else 
                bool has_mask = true;
#endif
                if( has_mask ) { /// vista has ipv6 masks

                    auto sa = sockaddr_.to_v6( ).to_bytes( );
                    auto ma = mask_.to_v6( ).to_bytes( );
                    auto ta = test.to_v6( ).to_bytes( );

                    /// something wrong!
                    static_assert( sa.max_size( ) == 16,
                                   "bytes_type::max_size( ) != 16");
                    res = (sa[0x0] & ma[0x0]) == (ta[0x0] & ma[0x0])
                       && (sa[0x1] & ma[0x1]) == (ta[0x1] & ma[0x1])
                       && (sa[0x2] & ma[0x2]) == (ta[0x2] & ma[0x2])
                       && (sa[0x3] & ma[0x3]) == (ta[0x3] & ma[0x3])
                       && (sa[0x4] & ma[0x4]) == (ta[0x4] & ma[0x4])
                       && (sa[0x5] & ma[0x5]) == (ta[0x5] & ma[0x5])
                       && (sa[0x6] & ma[0x6]) == (ta[0x6] & ma[0x6])
                       && (sa[0x7] & ma[0x7]) == (ta[0x7] & ma[0x7])
                       && (sa[0x8] & ma[0x8]) == (ta[0x8] & ma[0x8])
                       && (sa[0x9] & ma[0x9]) == (ta[0x9] & ma[0x9])
                       && (sa[0xA] & ma[0xA]) == (ta[0xA] & ma[0xA])
                       && (sa[0xB] & ma[0xB]) == (ta[0xB] & ma[0xB])
                       && (sa[0xC] & ma[0xC]) == (ta[0xC] & ma[0xC])
                       && (sa[0xD] & ma[0xD]) == (ta[0xD] & ma[0xD])
                       && (sa[0xE] & ma[0xE]) == (ta[0xE] & ma[0xE])
                       && (sa[0xF] & ma[0xF]) == (ta[0xF] & ma[0xF])
                       ;
                } else { // XP, Seven
                    return true; // always valid.
                }
            }
            return res;            
        }
    };
    using net_interface_list = std::vector<net_interface_info>;

}}

#endif
