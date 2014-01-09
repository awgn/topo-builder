/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef MORE_SHOW
#define MORE_SHOW

#include <type_traits.hpp>  // more!
#include <macro.hpp>        // more!
#include <cxxabi.hpp>       // more!

#include <ios>
#include <iomanip>
#include <array>
#include <tuple>
#include <chrono>
#include <memory>
#include <cstdint>
#include <type_traits>
#include <initializer_list>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>



#define MAKE_SHOW_PAIR(a, b) std::make_pair(std::string(#b), &UNPACK(a)::b)            

#define MAKE_GENERIC_SHOW(type, ...) make_generic_show<UNPACK(type)>(FOR2_EACH_COMMA(MAKE_SHOW_PAIR, type, __VA_ARGS__))

#define MAKE_SHOW(type, ...) \
inline std::string \
show(UNPACK(type) const &t) \
{ \
    static auto _show = MAKE_GENERIC_SHOW(type, __VA_ARGS__); \
    return _show(t); \
}

inline namespace more_show {

    // manipulators
    //
    
    template <typename T>
    struct _hex 
    {
        T value;
    };

    template <typename T>
    _hex<T> hex(T const &value)
    {
        return _hex<T>{value};
    }

    template <typename T>
    struct _oct 
    {
        T value;
    };

    template <typename T>
    _oct<T> oct(T const &value)
    {
        return _oct<T>{value};
    }

    template <typename T>
    struct _bin 
    {
        T value;
    };

    template <typename T>
    _bin<T> bin(T const &value)
    {
        return _bin<T>{value};
    }

    // forward declarations:
    //

    inline std::string 
    show(char c);
    
    inline std::string 
    show(bool);

    inline std::string 
    show(const char *v);

    inline std::string 
    show(std::string const &s);

    // numeric like...
    
    template <typename T> 
    inline typename std::enable_if<std::is_arithmetic<T>::value || (std::is_enum<T>::value && std::is_convertible<T,int>::value), std::string>::type 
    show(T const &value);

    // manipulators...
   
    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
    show(_hex<T> const &value);
    
    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
    show(_oct<T> const &value);

    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
    show(_bin<T> const &value);

    // pointers...
   
    template <typename T>
    inline std::string 
    show(T const *p);

    template <typename T>
    inline std::string 
    show(std::unique_ptr<T> const &);
    
    template <typename T>
    inline std::string 
    show(std::shared_ptr<T> const &);

    // pair<>
    
    template <typename U, typename V>
    inline std::string
    show(std::pair<U,V> const &r);

    // array...
    
    template <typename T, std::size_t N>
    inline std::string
    show(std::array<T,N> const &a);

    // tuple<>
    
    template <typename ...Ts>
    inline std::string
    show(std::tuple<Ts...> const &t);

    // chrono types
   
    template <typename Rep, typename Period>
    inline std::string
    show(std::chrono::duration<Rep, Period> const &dur);

    template <typename Clock, typename Dur>
    inline std::string
    show(std::chrono::time_point<Clock, Dur> const &r);

    // containers
   
    template <typename T>
    inline typename std::enable_if<
    (!std::is_pointer<T>::value) && (
        (more::traits::is_container<T>::value && !std::is_same<typename std::string,T>::value) ||
        (std::rank<T>::value > 0 && !std::is_same<char, typename std::remove_cv<typename std::remove_all_extents<T>::type>::type>::value)),
    std::string>::type 
    show(const T &v);

    namespace details
    {
        // show_on policy 
        //

        template <typename T, int N>
        struct show_on
        {
            static inline
            void apply(std::string &out, const T &tupl)
            {
                out += show(std::get< std::tuple_size<T>::value - N>(tupl)) + ' ';
                show_on<T,N-1>::apply(out,tupl);
            }
        }; 
        template <typename T>
        struct show_on<T, 0>
        {
            static inline
            void apply(std::string&, const T &)
            {}
        };

        // generic_show_on...
        //
        
        template <typename T, typename Tp, int N>
        struct generic_show_on
        {
            static inline
            void apply(std::string &out, const T &tupl, const Tp &value)
            {
                auto p = std::get< std::tuple_size<T>::value - N>(tupl);

                out += p.first + " = " +  show (std::bind(p.second, value)());
                if (N > 1) 
                    out += ", ";
                generic_show_on<T, Tp, N-1>::apply(out,tupl, value);
            }
        }; 
        template <typename T, typename Tp>
        struct generic_show_on<T, Tp, 0>
        {
            static inline
            void apply(std::string&, const T &, const Tp &)
            {}
        };

        template <typename T>
        struct duration_traits;
        template <> struct duration_traits<std::chrono::nanoseconds>  { static constexpr const char *str = "_ns"; };
        template <> struct duration_traits<std::chrono::microseconds> { static constexpr const char *str = "_us"; };
        template <> struct duration_traits<std::chrono::milliseconds> { static constexpr const char *str = "_ms"; };
        template <> struct duration_traits<std::chrono::seconds>      { static constexpr const char *str = "_s"; };
        template <> struct duration_traits<std::chrono::minutes>      { static constexpr const char *str = "_m"; };
        template <> struct duration_traits<std::chrono::hours>        { static constexpr const char *str = "_h"; };

    } // namespace details

    
    ///////////////////////////////////////
    // show with additional header/type:
    //

    template <typename Tp>
    inline std::string
    show(Tp &&type, const char *n)
    {
        auto hdr = n == nullptr ? "" :
                   n[0] == '\0' ? demangle(typeid(Tp).name()) : n;
        
        return std::move(hdr) + ' ' + show(std::forward<Tp>(type));
    }

    ///////////////////////////////////////
    // show for char 

    inline std::string
    show(char c)
    {
        return std::string(1, c);
    }
    
    ///////////////////////////////////////
    // show for bool 

    inline std::string
    show(bool v)
    {
        return v ? "true" : "false";
    }
    
    ///////////////////////////////////////
    // show for const char *

    inline std::string
    show(const char *v)
    {
        if (v != nullptr)
            return '"' + std::string(v) + '"';
        else
            return "\"nullptr\"";
    }

    ///////////////////////////////////////
    // show for std::string

    inline std::string
    show(std::string const &s)
    {
        return '"' + s + '"';
    }

    ///////////////////////////////////////
    // show for arithmetic types..

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value || (std::is_enum<T>::value && std::is_convertible<T,int>::value), std::string>::type
    show(T const &value)
    {
        return std::to_string(value);
    }

    /////////////////////////////////////////////
    // show for arithmetic types as hex values...

    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
    show(_hex<T> const &value)
    {
        std::ostringstream out;
        out << std::hex;

        if (std::is_same<T, uint8_t>::value)
            out << static_cast<uint32_t>(value.value);
        else
            out << value.value;

        return out.str();
    }                                               

    /////////////////////////////////////////////
    // show for arithmetic types as oct values...

    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
    show(_oct<T> const &value)
    {
        std::ostringstream out;
        out << std::oct << value.value;

        if (std::is_same<T, uint8_t>::value)
            out << static_cast<uint32_t>(value.value);
        else
            out << value.value;
        
        return out.str();
    }

    /////////////////////////////////////////////
    // show for arithmetic types as bin values...

    template <typename T>
    inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
    show(_bin<T> const &value)
    {
        std::ostringstream out;

        std::function<void(T)> binary = [&] (T value) 
        {
            T rem;

            if(value <= 1) {
                out << value;
                return;
            }
            
            rem = value % 2; 
            binary(value >> 1);    

            out << rem;
        };

        binary(value.value);
        return out.str();
    }

    ///////////////////////////////////////
    // show for pointers *

    template <typename T> 
    inline std::string
    show(T const *p)
    {
        std::ostringstream out;
        out << static_cast<const void *>(p);
        return out.str();
    }
    
    ///////////////////////////////////////
    // show for unique_ptr

    template <typename T> 
    inline std::string
    show(std::unique_ptr<T> const &p)
    {
        std::ostringstream out;
        out << static_cast<const void *>(p.get()) << "_up";
        return out.str();
    }

    ///////////////////////////////////////
    // show for shared_ptr

    template <typename T> 
    inline std::string
    show(std::shared_ptr<T> const &p)
    {
        std::ostringstream out;
        out << static_cast<const void *>(p.get()) << "_sp" << p.use_count();
        return out.str();
    }

    //////////////////////////
    // show for pair...

    template <typename U, typename V>
    inline std::string
    show(const std::pair<U,V> &r)
    {
        return  '(' + show(r.first) + ' ' + show(r.second) + ')';
    }

    ///////////////////////////
    // show for array...

    template <typename T, std::size_t N>
    inline std::string
    show(std::array<T,N> const &a)
    {
        std::string out("[");
        details::show_on<std::array<T,N>, N>::apply(out,a);
        return std::move(out) + ']';
    }

    ////////////////////////////////////////////////////////
    // show for tuple... 

    template <typename ...Ts>
    inline std::string
    show(std::tuple<Ts...> const &t)
    {
        std::string out("( ");
        details::show_on<std::tuple<Ts...>, sizeof...(Ts)>::apply(out,t);
        return std::move(out) + ')';
    }                                              

    ////////////////////////////////////////////////////////
    // show for chrono types... 

    template <typename Rep, typename Period>
    inline std::string
    show(std::chrono::duration<Rep, Period> const &dur)
    {
        std::string out(std::to_string(dur.count()));
        return std::move(out) + details::duration_traits<std::chrono::duration<Rep,Period>>::str;
    }

    template <typename Clock, typename Dur>
    inline std::string
    show(std::chrono::time_point<Clock, Dur> const &r)
    {    
        return show(r.time_since_epoch());
    }

    template <typename T>
    inline std::string
    show(std::initializer_list<T> const &init)
    {
        std::string out("{ ");
        for(auto const & e : init)
        {
            out += show(e) + ' ';
        }
        return std::move(out) + '}';
    }

    ///////////////////////////////////////
    // show for generic containers...

    template <typename T>
    inline typename std::enable_if<
    (!std::is_pointer<T>::value) && (
        (more::traits::is_container<T>::value && !std::is_same<typename std::string,T>::value) ||
        (std::rank<T>::value > 0 && !std::is_same<char, typename std::remove_cv<typename std::remove_all_extents<T>::type>::type>::value)),
    std::string>::type 
    show(const T &v)
    {
        std::string out("[ ");
        for(auto const & e : v)
        {
            out += show(e) + ' ';
        }
        return std::move(out) + ']';
    }
    
    //////////////////////////////////////////
    // generic_show for user defined types...
    
    template <typename Tp, typename ...Ps>
    struct generic_show
    {
        template <typename ...Ts>
        generic_show(Ts && ...args)
        : data_(std::forward<Ts>(args)...)
        {}

        std::string
        operator()(Tp const &value)
        {
            auto out = demangle(typeid(Tp).name()) + "{";
        
            details::generic_show_on<std::tuple<Ps...>, Tp, sizeof...(Ps)>::apply(out, data_, value); 

            return out + "}"; 
        }

        std::tuple<Ps...> data_;
    };
    
    template <typename Tp, typename ...Ts>
    generic_show<Tp, Ts...>
    make_generic_show(Ts && ... args)
    {
        return generic_show<Tp, Ts...>(std::forward<Ts>(args)...);
    }


} // namespace more_show


#endif /* __MORE_SHOW__ */
