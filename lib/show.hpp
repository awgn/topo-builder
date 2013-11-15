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

#include <ios>
#include <iomanip>
#include <array>
#include <tuple>
#include <chrono>
#include <memory>
#include <cstdint>
#include <type_traits>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>

#include <cxxabi.h>


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

    // forward declarations:
    //

    inline std::string 
    show(char c);
    
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
        // utilities 
        //

        inline std::string
        demangle(const char * name)
        {
            int status;
            std::unique_ptr<char, void(*)(void *)> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
            if (status < 0) {
                return std::string(1,'?');
            }
            return std::string(ret.get());
        }

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
                   n[0] == '\0' ? details::demangle(typeid(Tp).name()) : n;
        
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
    // show for const char *

    inline std::string
    show(const char *v)
    {
        return '"' + std::string(v) + '"';
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
        return  '(' + show(r.first) + ',' + show(r.second) + ')';
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
        std::string out("{ ");
        details::show_on<std::tuple<Ts...>, sizeof...(Ts)>::apply(out,t);
        return std::move(out) + '}';
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
        std::string out("{ ");
        for(auto const & e : v)
        {
            out += show(e) + ' ';
        }
        return std::move(out) + '}';
    }

} // namespace more_show


#endif /* SHOW_HPP */
