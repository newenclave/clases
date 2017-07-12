#ifndef RESULT_H
#define RESULT_H

#include <sstream>
#include <memory>

namespace rs { namespace detail {

    namespace traits {

        template <typename T>
        struct value_trait {
            typedef T value_type;

            static
            void copy( value_type &v, const value_type &from )
            {
                v = from;
            }

            static
            value_type create(  )
            {
                return std::move(value_type( ));
            }

            template <typename ...Args>
            static
            value_type create( Args&&...args )
            {
                return std::move(value_type(std::forward<Args>( args )... ));
            }

            static
            void destroy( value_type & )
            { }

            static
            T &&move( value_type &v )
            {
                return std::move(v);
            }

            static
            T &value( value_type &v )
            {
                return v;
            }

            static
            const T &value( value_type const &v )
            {
                return v;
            }

        };

        template <typename T>
        struct shared_ptr_trait {

            typedef std::shared_ptr<T> value_type;
            static
            void copy( value_type &v, const value_type &from )
            {
                v = from;
            }

            static
            value_type create(  )
            {
                return std::make_shared<T>( );
            }

            template <typename ...Args>
            static
            value_type create( Args&&...args )
            {
                return std::make_shared<T>( std::forward<Args>( args )... );
            }

            static
            void destroy( value_type & )
            { }

            static
            T &&move( value_type &v )
            {
                return std::move(*v);
            }

            static
            T &value( value_type &v )
            {
                return *v;
            }

            static
            const T &value( value_type const &v )
            {
                return *v;
            }
        };
    }

    template <typename T, typename E,
              typename Trait = traits::value_trait<T> >
    class result {

        typename Trait::value_type value_;
        E                          error_;
        bool                       failed_;

    public:

        typedef T value_type;
        typedef E error_type;

        struct value_arg {

            template <typename ...Args>
            value_arg( Args&& ...args )
                :val_(Trait::create(std::forward<Args>(args)...))
            { }

            value_type &&move( )
            {
                return std::move(val_);
            }

        private:
            value_type val_;
        };

        struct error_arg {

            template <typename ...Args>
            error_arg( Args&& ...args )
                :val_(std::forward<Args>(args)...)
            { }

            error_type &&move( )
            {
                return std::move(val_);
            }

        private:
            error_type val_;
        };

        result( )
            :value_(Trait::create( ))
            ,failed_(false)
        { }

        result( const result &other )
            :value_(Trait::create(other))
            ,error_(other.error_)
            ,failed_(other.failed_)
        { }

        result( result &&other )
            :value_(std::move(other.value_))
            ,error_(std::move(other.error_))
            ,failed_(other.failed_)
        { }

        result( value_arg &&val )
            :value_(val.move( ))
            ,error_(error_type( ))
            ,failed_(false)
        { }

        result( error_arg &&val )
            :error_(val.move( ))
            ,failed_(true)
        { }

        result &operator = ( const result &other )
        {
            error_ = other.error_;
            Trait::copy( value_, other.value_ );
            return *this;
        }

        result &operator = ( result &&other )
        {
            error_ = std::move(other.error_);
            value_ = std::move(other.value_);
            return *this;
        }

        ~result( )
        {
            Trait::destroy( value_ );
        }

        template <typename ...Args>
        static
        result ok( Args&& ...args )
        {
            return result( value_arg(std::forward<Args>(args)...) );
        }

        template <typename ...Args>
        static
        result fail( Args&& ...args )
        {
            result res( error_arg(std::forward<Args>(args)...) );
            return std::move( res );
        }

        void swap( result &other )
        {
            std::swap( value_,  other.value_ );
            std::swap( error_,  other.error_ );
            std::swap( failed_, other.failed_ );
        }

        T &operator *( )
        {
            return Trait::value(value_);
        }

        const T &operator *( ) const
        {
            return Trait::value(value_);
        }

        T *operator -> ( )
        {
            return &Trait::value(value_);
        }

        const T *operator -> ( ) const
        {
            return &Trait::value(value_);
        }

        operator bool ( ) const
        {
            return !failed_;
        }

        const error_type &error( ) const
        {
            return error_;
        }

        T &&move( )
        {
            return Trait::move(value_);
        }

    };

    template <typename T, typename E>
    inline
    std::ostream & operator << ( std::ostream &o, const result<T, E> &res )
    {
        if( res ) {
            o << "Ok: "   << *res;
        } else {
            o << "Fail: " <<  res.error( );
        }
        return o;
    }

}}

#endif // RESULT_H
