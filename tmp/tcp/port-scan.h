#pragma once

#ifndef RS_TCP_PORT_SCAN_H
#define RS_TCP_PORT_SCAN_H

#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>
#include "boost/asio/ip/tcp.hpp"

namespace rs { namespace tcp {

    template<typename MutexType = std::mutex>
    class port_scan {

        using socket_type = boost::asio::ip::tcp::socket;
        using socket_ptr  = std::shared_ptr<socket_type>;
        using locker_type = std::lock_guard<MutexType>;

    public:

        using io_service   = boost::asio::io_service;
        using error_code   = boost::system::error_code;
        using address_type = boost::asio::ip::address;
        using mutex_type   = MutexType;

        struct port {
            std::uint16_t value;
            explicit port( std::uint16_t v )
                :value(v)
            { }
        };

        port_scan( io_service &ios, address_type addr)
            :ios_(ios)
            ,addr_(addr)
        {  }

        virtual ~port_scan( )
        { }

        void start( port begin, port end, int pool_size = 10 )
        {
            current_ = begin.value;

            std::vector<socket_ptr> tmp;

            for( int i=0; i<pool_size; i++ ) {
                auto sock = std::make_shared<socket_type>( std::ref( ios_ ) );
                tmp.push_back( sock );
            }

            sockets_.swap( tmp );

            for( int i=0; i<pool_size; i++ ) {
                start( sockets_[i], nextport( ), end.value );
            }
        }

        size_t scanned( ) 
        {
            return scanned_.size( );
        }

        const address_type &address( ) const 
        {
            return addr_;
        }

    protected:

        std::uint16_t nextport( )
        {
            locker_type l( current_lock_ );
            return current_++;
        }

        void start( socket_ptr sock, std::uint16_t begin, std::uint16_t  end )
        {
            addr_.is_v6( ) ? sock->open( boost::asio::ip::tcp::v6( ) )
                           : sock->open( boost::asio::ip::tcp::v4( ) );

            using boost::asio::socket_base;
            sock->set_option( socket_base::reuse_address(true) );

            boost::asio::ip::tcp::endpoint ep( addr_, begin );

            while( begin <= end ) {
                if( check_it( port( begin ) ) ) {
                    break;
                }
                begin = nextport( );
            }

            if( begin <= end ) {
                //std::cout << "Start scan: " << begin << "\n";
                scanned_.insert( begin );
                sock->async_connect( ep, 
                    [this, begin, end, sock](const error_code &e) 
                    {
                        on_connect( e, sock, begin, end );
                    } );
            }
        }

        void on_connect( const error_code &err, 
                         socket_ptr sock, 
                         std::uint16_t current, std::uint16_t end )
        {
            if( !err ) {
                on_openport( port( current ) );
            } else {
                on_connect_error( err, port( current ) );
            }
            sock->close( );
            current = nextport( );
            if( current <= end ) {
                start( sock, current, end );
            }
        }

        virtual void on_openport( port ) { }
        virtual void on_connect_error( const error_code &, port ) { }
        virtual bool check_it( port ) 
        { 
            return true;
        }

    private:
        std::set<std::uint16_t>   scanned_;
        io_service               &ios_;
        std::vector<socket_ptr>   sockets_;
        address_type              addr_;
        std::uint16_t             current_;
        mutex_type                current_lock_;
    };

}}

#endif