#ifndef RC_DETAILS_SLICE_H
#define RC_DETAILS_SLICE_H

#include <cstdint>

namespace rs  { namespace details {

    template <typename ValueType, typename SizeType = std::size_t>
    class slice {
    public:

        using value_type     = ValueType;
        using size_type      = SizeType;
        using pointer_type   = value_type *;
        using reference_type = value_type &;

        slice( value_type *value, size_t len )
            :val_(value)
            ,len_(len)
        { }

        slice             (               )  = default;
        slice             ( const slice & )  = default;
        slice             ( slice &&      )  = default;
        slice &operator = ( slice &&      )  = default;
        slice &operator = ( const slice & )  = default;

        template <typename U, typename S = size_type>
        slice<U, S> rebind( )
        {
            S new_size = (sizeof(value_type) * len_) / sizeof(U);
            slice<U, S> tmp( reinterpret_cast<U *>(val_), new_size );
            return tmp;
        }

        value_type *begin( )
        {
            return val_;
        }

        value_type *end( )
        {
            return val_ + size( );
        }

        size_type size( ) const
        {
            return len_;
        }

        pointer_type get( )
        {
            return val_;
        }

        reference_type operator *( )
        {
            return *val_;
        }

        slice &operator += ( size_type len )
        {
            val_ += len;
            len_ -= len;
            return *this;
        }

        slice &operator -= ( size_type len )
        {
            val_ -= len;
            len_ += len;
            return *this;
        }

        slice &operator ++ ( )
        {
            val_ ++;
            len_ --;
            return *this;
        }

        slice operator ++ (int)
        {
            slice tmp( val_, len_ );
            ++(*this);
            return tmp;
        }

    private:

        value_type  *val_ = nullptr;
        size_type    len_ = 0;
    };

}}

#endif // SLICE_H
