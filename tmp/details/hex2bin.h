#ifndef RS_DETAILS_HEX2BIN_H
#define RS_DETAILS_HEX2BIN_H

#include <string>

namespace rs  { namespace details {

struct hex2bin {

    static
    size_t size( size_t binary_size )
    {
        return binary_size / 2;
    }

    static
    bool get( const char *data, size_t len, char *out )
    {
        struct h2b {
            static std::uint8_t get( char c )
            {
                switch (c) {
                case 'a': case 'b':
                case 'c': case 'd':
                case 'e': case 'f':
                    return c - 'a' + 10;
                case 'A': case 'B':
                case 'C': case 'D':
                case 'E': case 'F':
                    return c - 'A' + 10;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    return c - '0';
                default:
                    return 0xFF;
                }
            }
        };

        for( size_t i = 0; i < len; i += 2 ) {

            auto f = h2b::get( data[i] ) << 4;
            auto s = h2b::get( data[i + 1] );

            if( f == 0xFF || s == 0xFF ) {
                return false;
            }

            *out++ = (f | s);
        }

        return true;
    }

    static
    std::string get( const std::string &data )
    {
        std::string res(size(data.size( ) ), '\0');
        if( res.size( ) ) {
            get( data.c_str( ), data.size( ), &res[0] );
        }
        return std::move( res );
    }
};

}}

#endif // HEXT2BIN_H
