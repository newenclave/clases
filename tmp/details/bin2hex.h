#ifndef RS_DETAILS_BIN2HEX_H
#define RS_DETAILS_BIN2HEX_H

#include <string>

namespace rs  { namespace details {

    struct bin2hex {

        static
        size_t size( size_t binary_size )
        {
            return binary_size * 2;
        }

        static
        bool get( const char *d, size_t len, char *o )
        {
            static const char b2h[ ] = { '0', '1', '2', '3',
                                         '4', '5', '6', '7',
                                         '8', '9', 'A', 'B',
                                         'C', 'D', 'E', 'F'  };
            while( len-- ) {
                *o++ = b2h[ (std::uint8_t(*d  ) >> 4) & 0xF ];
                *o++ = b2h[  std::uint8_t(*d++)       & 0xF ];
            }
            return true;
        }

        static
        std::string get( const std::string &data )
        {
            std::string res(size(data.size( )), '\0');

            if( res.size( ) ) {
                get( data.c_str( ), data.size( ), &res[0] );
            }

            return std::move( res );
        }
    };

}}

#endif // BIN2HEX_H
