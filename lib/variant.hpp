/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef MORE_VARIANT_HPP
#define MORE_VARIANT_HPP

#include <typeinfo>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <typeinfo>
#include <functional>
#include <sstream>

// Yet another boost tribute: the variant class.
//

namespace more { 

    namespace variant_details {

        constexpr unsigned int 
        next_pow2(unsigned int v, unsigned int power = 1)
        {
            return (power >= v) ? power : 
                                  next_pow2(v, power * 2);
        }
       
        template <typename T>
        struct assert_is_nothrow_move_constructible
        {
#if (__clang__) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 6))
            static_assert(std::is_nothrow_move_constructible<T>::value,
                          "all types must be nothrow move constructible");
#else
#pragma message "Types in variant must be nothrow move constructible"
#endif
            typedef int type;
        };

        template <template <typename> class F, typename ... Ts> struct for_each;
        template <template <typename> class F, typename T>
        struct for_each<F, T>
        {
            typedef typename F<T>::type type;
        };
        template <template <typename> class F, typename T, typename ...Ts>
        struct for_each<F, T, Ts...> : for_each <F, Ts...>
        {
            typedef typename F<T>::type type;
        };
        template <template <typename> class F>
        struct for_each<F>
        {
            typedef void type;
        };

        ///////////////////////////////////////////// max - sizeof
         
        template <typename T> 
        struct size_of
        {
            static constexpr size_t value = sizeof(T); 
        };

        template <template <typename> class F, typename ...Ts> struct max;
        template <template <typename> class F>
        struct max<F>
        {
            static constexpr size_t value = 0;
        };
        template <template <typename> class F, typename T, typename ...Ts>
        struct max<F, T, Ts...>
        {
            static constexpr size_t _head  = F<T>::value;
            static constexpr size_t _tail  = max<F, Ts...>::value;
            static constexpr size_t value = _head > _tail ? _head : _tail;
        };

        ///////////////////////////////////////////// type index 

        template <int N, typename T, typename ...Ti> struct index_of_;

        template <int N, typename T, typename T0, typename ...Ti>
        struct index_of_<N,T,T0,Ti...>
        {
            enum { value = std::is_same<T,T0>::value ? N : index_of_<N+1, T, Ti...>::value };
        };
        template <int N, typename T, typename T0>
        struct index_of_<N,T,T0>
        {
            enum { value = std::is_same<T,T0>::value ? N : -1 };
        };

        template <typename T, typename ...Ti> struct index_of
        {
            enum { value = index_of_<0, T, Ti...>::value };    
        };

        ////////////////////////////////////////////// get n-th type

        template <int i, int N, typename ...Ti> struct get_type_;
        template <int i, int N, typename T0, typename ...Ti>
        struct get_type_<i,N, T0, Ti...>
        {
            typedef typename get_type_<i+1, N, Ti...>::type type;
        };
        template <int N, typename T0, typename ...Ti>
        struct get_type_<N,N, T0, Ti...>
        {
            typedef T0 type;
        };

        template <int N, typename ...Ti> struct get_type
        {
            typedef typename get_type_<0,N, Ti...>::type type;
        };  

        ////////////////////////////////////////////// stream_on:

        template <typename Out>
        struct stream_on
        {
            Out &out_;
            
            stream_on(Out &o)
            : out_(o)
            {}

            template <typename T>
            void operator()(const T & elem)
            {
                out_ << elem;
            }
        };

        ///////////////////////////////////////////////////////////// FIXME!
        // this workaround is required as long as compliers do not implement
        // ref-qualifier (see variant<>::get method).

        template <typename Tp>
        inline Tp move_if(Tp &value, std::true_type)
        { return value; }
        
        template <typename Tp>
        inline Tp & move_if(Tp &value, std::false_type)
        { return value; }

        template <bool V, typename Tp>
        inline auto 
        move_if(Tp & value)
        -> decltype(move_if(value, std::integral_constant<bool, V>()))
        {
            return move_if(value, std::integral_constant<bool, V>());
        }

        ////////////////////////////////////////////// visitor:

        template <typename ...Tp> struct visitor;
        template <typename T, typename ...Tp>
        struct visitor<T, Tp...> 
        {
            template <typename F, typename V>
            static auto 
            apply(F fun, V &&var, int n = 0)
            -> decltype(fun(std::declval<T &>())) 
            {
                if (n == var.which()) 
                {
                    return fun(move_if<std::is_rvalue_reference<V &&>::value>(var.template get<T>()));
                }
                return visitor<Tp...>::apply(fun, std::forward<V>(var), n+1);    
            }
            
            template <typename F, typename V1, typename V2>
            static auto 
            apply2(F fun, V1 &&var1, V2 &&var2, int n = 0)
            -> decltype(fun(std::declval<T &>(), std::declval<T &>())) 
            {
                if (n == var1.which() && n == var2.which()) 
                {
                    return fun(move_if<std::is_rvalue_reference<V1 &&>::value>(var1.template get<T>()), 
                               move_if<std::is_rvalue_reference<V2 &&>::value>(var2.template get<T>()));
                }
                return visitor<Tp...>::apply2(fun, std::forward<V1>(var1), std::forward<V2>(var2), n+1);    
            }
        };
        template <typename T>
        struct visitor<T>
        {
            template <typename F, typename V>
            static auto 
            apply(F fun, V &&var, int n = 0)
            -> decltype(fun(std::declval<T&>()))
            {
                if (n == var.which()) 
                {
                    return fun(move_if<std::is_rvalue_reference<V &&>::value>(var.template get<T>()));
                }
                throw std::runtime_error("variant: internal error");
            }
            template <typename F, typename V1, typename V2>
            static auto 
            apply2(F fun, V1 &&var1, V2 &&var2, int n = 0)
            -> decltype(fun(std::declval<T &>(), std::declval<T &>())) 
            {
                if (n == var1.which() && n == var2.which()) 
                {           
                    return fun(move_if<std::is_rvalue_reference<V1 &&>::value>(var1.template get<T>()), 
                               move_if<std::is_rvalue_reference<V2 &&>::value>(var2.template get<T>()));
                }
                throw std::runtime_error("variant: internal error");
            }
        };

    }  // namespace details

    
    ////////////////////////////////////////////// variant class...
    ////////////////////////////////////////////// 


    template <typename ...Ts>  
    class variant 
    {
        // ensure the types have nothrow move or the copy constructors:
        //
        
        typedef typename variant_details::for_each<variant_details::assert_is_nothrow_move_constructible, Ts...>::type check;

    public: 

        // determine the storage_type:
        //

        static constexpr size_t storage_len   = variant_details::max<variant_details::size_of, Ts...>::value;
        static constexpr size_t storage_align = variant_details::max<std::alignment_of, Ts...>::value;

        typedef typename std::aligned_storage<storage_len, variant_details::next_pow2(storage_align)>::type storage_type;

    public:
        
        variant()
        : type_(-1)
        {}

        ~variant()
        {
            if (type_ != -1)
            {
                variant_details::visitor<Ts...>::apply(dtor(), *this);
            }
        }

        template <typename T, typename V = typename std::enable_if<!std::is_same<typename std::decay<T>::type,variant>::value, void>::type>
        variant(T && arg)
        : type_(-1) 
        {
            set(std::forward<T>(arg));
        }
        
        variant(const variant &rhs)   // copy constructor
        : type_(rhs.type_)
        {
            if (rhs.type_ != -1)
            {
                variant_details::visitor<Ts...>::apply2(copy_ctor(), *this, rhs);
            }
        }

        variant(variant && rhs)       // move constructor
        : type_(rhs.type_)
        {
            if (rhs.type_ != -1)
            {
                variant_details::visitor<Ts...>::apply2(move_ctor(), *this, std::move(rhs));
                variant_details::visitor<Ts...>::apply(dtor(), rhs);
                rhs.type_ = -1;
            }
        }

        variant& operator=(variant const &rhs)  // assignment operator
        {
            if (type_ == rhs.type_)
            {
                if (type_ != -1)  
                {
                    variant_details::visitor<Ts...>::apply2(assign_op(), *this, rhs);
                }
            }
            else
            {
                if (rhs.type_ != -1) 
                {
                    variant tmp(rhs);
                    
                    if (type_ != -1) 
                        variant_details::visitor<Ts...>::apply(dtor(), *this);
                
                    type_ = rhs.type_;

                    variant_details::visitor<Ts...>::apply2(move_ctor(), *this, std::move(tmp));
                }
                else
                {
                    variant_details::visitor<Ts...>::apply(dtor(), *this);
                    type_ = -1;
                }
            }
            return *this;
        }


        variant& operator=(variant &&rhs)   // move assignment operator
        {
            if (type_ == rhs.type_)
            {
                if (type_ != -1)  
                {
                    variant_details::visitor<Ts...>::apply2(move_assign_op(), *this, std::move(rhs));
                    variant_details::visitor<Ts...>::apply(dtor(), rhs);
                    rhs.type_ = -1;
                }
            }
            else
            {
                if (type_ != -1) 
                    variant_details::visitor<Ts...>::apply(dtor(), *this);
             
                type_ = rhs.type_;
                
                if (type_ != -1)
                {
                    variant_details::visitor<Ts...>::apply2(move_ctor(), *this, std::move(rhs));
                    variant_details::visitor<Ts...>::apply(dtor(), rhs);
                    rhs.type_ = -1;
                }
            }
            return *this;
        }
        
        // universal assignment operator
        //
        
        template <typename T, typename V = typename std::enable_if<!std::is_same<typename std::decay<T>::value,variant>::value, variant>::type>
        variant &
        operator=(T && arg)
        {
            set(std::forward<T>(arg));
            return *this;
        }
        
        bool
        empty() const
        {
            return type_ == -1;
        }

        int 
        which() const
        {
            return type_;
        }

        // get methods: these methods require ref-qualifier which is not 
        //              yet implemented in common compilers: i.e. g++-4.8 
        
        template <typename T>
        T & get()
        {
            if (variant_details::index_of<T,Ts...>::value != type_)
                throw std::bad_cast();

            return *reinterpret_cast<T *>(&storage_);
        }

        template <typename T>
        const T & get() const
        {
            if (variant_details::index_of<T,Ts...>::value != type_)
                throw std::bad_cast();

            return *reinterpret_cast<const T *>(&storage_);
        }

        template <typename T>
        void set(T value)
        {
            constexpr auto t = variant_details::index_of<typename std::decay<T>::type, Ts...>::value;

            static_assert(t != -1, "T not found in variant");
            
            // destroy the object in the variant...
            
            this->~variant();            

            // move the object to the storage_
            
            new (&storage_) T(std::move(value));
            
            type_  = t;
        }

        bool operator==(variant const &rhs)
        {
            if (type_ == rhs.type_)
            {
                if (type_ == -1)
                    return true;

                return variant_details::visitor<Ts...>::apply2(op_equal(), *this, rhs);
            }
            else
                return false;
        }

        bool operator!=(variant const &rhs)
        {
            return !(*this == rhs);
        }

    private:
        
        struct ctor
        {
            template <typename T1, typename T2>
            void operator()(T1&& v1, T2 &&v2)
            {
                new (&v1) T2(std::forward<T2>(v2));
            }
        };

        struct dtor
        {
            template <typename T>
            void operator()(T &instance)
            {
                instance.~T();
            }
        };

        struct copy_ctor
        {
            template <typename T>
            void operator()(T &lhs, const T &rhs)
            {
                new (&lhs) T(rhs);
            }
        };

        struct move_ctor
        {
            template <typename T, typename Tr>
            void operator()(T &lhs, Tr &&rhs)
            {
                new (&lhs) T(std::move(rhs));
            }
        };
        
        struct assign_op
        {
            template <typename T>
            void operator()(T &lhs, const T &rhs)
            {
                lhs = rhs;
            }
        };
        
        struct move_assign_op
        {
            template <typename T, typename Tp>
            void operator()(T &lhs, Tp &&rhs)
            {
                lhs = std::forward<Tp>(rhs);
            }
        };

        struct op_equal
        {
            template <typename T>
            bool operator()(T &lhs, const T &rhs)
            {
                return lhs == rhs;
            }
        };

    private:
        
        storage_type storage_;
        int type_;
    };
         

    template <typename CharT, typename Traits, typename ...Ts>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, variant<Ts...> const&  that)
    {
        return variant_details::visitor<Ts...>::apply(
            variant_details::stream_on<decltype(out)>(out), that), out;
    }

    template <typename ...Ts>
    inline std::string
    show(variant<Ts...> const &var)
    {
        std::ostringstream out; out << var;
        return out.str();
    }

    template <typename Fun, typename ...Ts>
    auto visitor(Fun f, variant<Ts...> &v)
    -> decltype(variant_details::visitor<Ts...>::apply(f, v))
    {
        return variant_details::visitor<Ts...>::apply(f, v);    
    }

    template <typename Fun, typename ...Ts>
    auto visitor(Fun f, variant<Ts...> const &v)
    -> decltype(variant_details::visitor<Ts...>::apply(f, v))
    {
        return variant_details::visitor<Ts...>::apply(f, v);    
    }

    struct variant_hash
    {
        template <typename T>
        size_t operator()(const T& x) const
        {
            return std::hash<T>()(x);
        }
    };

} // namespace more


// Specialization for std::hash...
//
namespace std 
{
    template <typename ...Ts>
    struct hash<more::variant<Ts...>>
    {
        size_t operator()(const more::variant<Ts...>& x) const
        {
            return more::visitor(more::variant_hash(), x);
        }
    };
}

#endif /* MORE_VARIANT_HPP */

