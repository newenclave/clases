#pragma once
#ifndef RC_SYSTEM_VERSION_H
#define RC_SYSTEM_VERSION_H

#ifdef _WIN32
#include <winnt.h>
#include <tchar.h>
#endif

namespace rs { namespace system {

    struct version {
#ifdef _WIN32
        static
        bool fill_native_version( OSVERSIONINFO *info )
        {
            LONG( __stdcall *NtRtlGetVer )(PRTL_OSVERSIONINFOW);

            RTL_OSVERSIONINFOW oi = { 0 };

            oi.dwOSVersionInfoSize = sizeof( oi );

            auto ntdll   = GetModuleHandle( _T("ntdll.dll") );
            auto farproc = GetProcAddress( ntdll, "RtlGetVersion" );

            if( !farproc ) {
                return false;
            }

            NtRtlGetVer = reinterpret_cast<decltype(NtRtlGetVer)>(farproc);

            if( NtRtlGetVer( &oi ) ) {
                return false;
            }

            if( info ) {
                info->dwBuildNumber  = oi.dwBuildNumber;
                info->dwMajorVersion = oi.dwMajorVersion;
                info->dwMinorVersion = oi.dwMinorVersion;
                info->dwPlatformId   = oi.dwPlatformId;
            }
            return true;
        };

        static
        bool vista_or_higher( )
        {
            OSVERSIONINFO ov;
            return fill_native_version( &ov ) && (ov.dwMajorVersion >= 6);
        }
#else
        static 
        constexpr bool vista_or_higher( )
        {
            return false;
        }
#endif
    };

} }

#endif
