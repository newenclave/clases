#pragma once
#ifndef RC_DETAILS_DUMMY_MUTEX_H
#define RC_DETAILS_DUMMY_MUTEX_H

namespace rs { namespace details {
    struct dummy_mutex {
        void lock    ( ) {              }
        void unlock  ( ) {              }
        bool try_lock( ) { return true; }
    };
}}

#endif