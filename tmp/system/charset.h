#pragma once
#ifndef RC_SYSTEM_CHARSET_H
#define RC_SYSTEM_CHARSET_H

namespace rs { namespace system {

    struct charset {
#ifdef _WIN32
        static
        std::string make_mb_string( LPCWSTR src, UINT CodePage = CP_UTF8 )
        {
            int cch = WideCharToMultiByte( CodePage, 0, src, -1, 0, 0, 0, 0 );
            if( 0 == cch ) {
                return std::string( );
            } else {
                std::vector<char> data;
                data.resize( cch + 1 );
                cch = WideCharToMultiByte( CodePage, 0,
                                           src, -1, &data[0],
                                           (DWORD)data.size( ), 0, 0 );
                return cch ? &data.front( ) : "";
            }
        }
#endif
    };

}}

#endif
