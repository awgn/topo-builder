/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#pragma once 

#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <array>
#include <chrono>

#include <iostream>

#include <type_traits.hpp>
#include <cxxabi.hpp>

inline namespace more_read {

    //////////////////////////////////////////////////////
    //
    // forward declaration...
    //

    template <typename T, typename CharT, typename Traits>
    T read(std::basic_istream<CharT,Traits>&in);

    template <typename T>
    std::pair<T, std::string> read(std::string const &ref);

    namespace details
    {
        // consume a specified char, return true or false 
        //

        template <typename CharT, typename Traits>
        bool consume(char c, std::basic_istream<CharT,Traits>& in)
        {
            decltype(c) _c;

            if(!(in >> std::ws)) 
                return false;

            _c = in.peek();
            if (!in) 
                return false;

            if (c == _c) {
                in.get();
                assert(in);
                return true;
            }

            return false;
        }

        // consume a specified string, throw an exception if not matching 
        //

        template <typename CharT, typename Traits>
        void consume(const char *s, std::basic_istream<CharT, Traits> &in)
        {
            std::string _s; 
            if (!(in >> _s) || _s.compare(s) != 0) 
                throw std::runtime_error("read: " + _s + " (" + s  + ") parse error");
        }                                                            

        template <typename Tp>
        struct decay
        {
            typedef typename std::decay<Tp>::type type;
        };
        template <typename T1, typename T2>
        struct decay<std::pair<T1,T2>>
        {
            typedef std::pair<typename std::decay<T1>::type, 
                              typename std::decay<T2>::type> type;
        };
        template <typename ...Ts>
        struct decay<std::tuple<Ts...>>
        {
            typedef std::tuple<typename std::decay<Ts>::type...> type;
        };

        template <typename Tp>
        std::string
        error(const char *msg)
        {
            return "read<" + demangle(typeid(Tp).name()) + ">: " + msg;
        }

        template <typename T, int N>
        struct read_on
        {
            template <typename CharT, typename Traits>
            static inline
            void apply(T &tupl, std::basic_istream<CharT, Traits> &in)
            {
                constexpr size_t I = std::tuple_size<T>::value - N;

                std::get<I>(tupl) = read<typename std::tuple_element<I, T>::type>(in);

                read_on<T,N-1>::apply(tupl, in);
            }
        }; 
        template <typename T>
        struct read_on<T, 0>
        {
            template <typename CharT, typename Traits>
            static inline
            void apply(T &, std::basic_istream<CharT, Traits> &)
            {}
        };
        
        // enabled for std::vector, std::dequeue, std::list
        // 
        template <typename C, typename V>
        typename std::enable_if<!more::traits::has_key_type<C>::value && 
                                !more::traits::has_container_type<C>::value, bool>::type 
        insert(C &cont, V &&value)
        {
            cont.push_back(std::forward<V>(value));
            return true;
        }

        // enabled for std::stack, std::queue, std::priority_queue
        // 
        template <typename C, typename V>
        typename std::enable_if<!more::traits::has_key_type<C>::value && 
                                more::traits::has_container_type<C>::value, bool>::type 
        insert(C &cont, V &&value)
        {
            cont.push(std::forward<V>(value));
            return true;
        }

        // enabled for std::set, std::multiset, std::unordered_set,
        // std::unordered_multiset
        // 
        template <typename C, typename V>
        typename std::enable_if<more::traits::has_key_type<C>::value, bool>::type 
        insert(C &cont, V && value)
        {
            return cont.insert(std::forward<V>(value)).second;
        }

        // enabled for std::map, std::multimap, std::unordered_map,
        // std::unordered_multimap
        // 
        template <typename C, typename T, typename V>
        typename std::enable_if<more::traits::has_key_type<C>::value, bool>::type 
        insert(C &cont, std::pair<T,V> && value)
        {
            return cont.insert(std::move(value)).second;
        }

    } // namespace details


    // utility for tag dispatching...
    //
    
    template <typename Tp>
    struct read_tag
    {
        typedef Tp type;
    };

    //
    // specializations for specific types...
    //

    template <typename CharT, typename Traits>
    bool 
    read(read_tag<bool>, std::basic_istream<CharT,Traits>&in)
    {
        bool ret;
        in >> std::noboolalpha;

        if (!(in >> ret)) {
            in.clear();
            if (!(in >> std::boolalpha >> ret))
                throw std::runtime_error(details::error<bool>("parse error"));
        }
        
        return ret;
    }

    // pointers:
    //
    
    template <typename CharT, typename Traits>
    char const *
    read(read_tag<const char *>, std::basic_istream<CharT,Traits>&in)
    {
        std::string tmp;

        if (!(in >> tmp)) {
            throw std::runtime_error(details::error<const char *>("parse error"));
        }

        if (!tmp.empty())
        {
            char * ptr = reinterpret_cast<char *>(malloc(tmp.size()+1));
            std::strcpy(ptr, tmp.c_str());
            return ptr;
        }
        else 
            return nullptr;
    }

    template <typename CharT, typename Traits, typename Tp>
    Tp *
    read(read_tag<Tp *>, std::basic_istream<CharT,Traits>&in)
    {
        auto ret = new Tp{};

        if (!(in >> *ret)) {
            delete ret;
            throw std::runtime_error(details::error<Tp *>("parse error"));
        }
        
        return ret;
    }

    template <typename CharT, typename Traits, typename Tp>
    std::shared_ptr<Tp>
    read(read_tag<std::shared_ptr<Tp>>, std::basic_istream<CharT,Traits>&in)
    {
        auto ret = std::make_shared<Tp>();

        if (!(in >> *ret)) 
            throw std::runtime_error(details::error<Tp *>("parse error"));
        
        return ret;
    }

    template <typename CharT, typename Traits, typename Tp>
    std::unique_ptr<Tp>
    read(read_tag<std::unique_ptr<Tp>>, std::basic_istream<CharT,Traits>&in)
    {
        auto ret = std::unique_ptr<Tp>(new Tp{});

        if (!(in >> *ret)) {
            throw std::runtime_error(details::error<Tp *>("parse error"));
        }
        
        return ret;
    }

    // pair<T1,T2>:
    //
    
    template <typename T1, typename T2, typename CharT, typename Traits>
    std::pair<T1,T2> 
    read(read_tag<std::pair<T1,T2>>, std::basic_istream<CharT,Traits>&in)
    {
        if (details::consume('(', in)) {

            T1 a = read<T1>(in); 
            T2 b = read<T2>(in);

            if (!details::consume(')', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            return std::make_pair(std::move(a), std::move(b));
        }
        else if (details::consume('[', in)) {

            T1 a = read<T1>(in); details::consume("->", in); 
            T2 b = read<T2>(in);

            if (!details::consume(']', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            return std::make_pair(std::move(a), std::move(b));
        }
        else {

            T1 a = read<T1>(in); details::consume("->", in); 
            T2 b = read<T2>(in);

            return std::make_pair(std::move(a), std::move(b));
        }
    }
    
    // std::array<T,N>:
    //

    template <typename T, size_t N, typename CharT, typename Traits>
    std::array<T,N> 
    read(read_tag<std::array<T,N>>, std::basic_istream<CharT,Traits>&in)
    {
        std::array<T, N> ret;

        if (!details::consume('[', in))
            throw std::runtime_error(details::error<std::array<T,N>>("parse error"));

        for(auto & e : ret)
        {
            e = read<T>(in);
        }

        if (!details::consume(']', in))
            throw std::runtime_error(details::error<std::array<T,N>>("parse error"));

        return ret;
    }

    // std::chrono::duration<Rep, Period>:
    //
    
    template <typename Rep, typename Period, typename CharT, typename Traits>
    std::chrono::duration<Rep, Period>
    read(read_tag<std::chrono::duration<Rep, Period>>, std::basic_istream<CharT,Traits>&in)
    {
        typedef std::chrono::duration<Rep, Period> Duration;
        
        int64_t value; char c; std::string unit;

        if (!(in >> value >> c >> unit))
            throw std::runtime_error(details::error<std::chrono::duration<Rep, Period>>("parse error"));

        if (unit.compare("ns") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(value));

        if (unit.compare("us") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::microseconds(value));
        
        if (unit.compare("ms") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::milliseconds(value));

        if (unit.compare("s") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::seconds(value));
        
        if (unit.compare("m") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::minutes(value));

        if (unit.compare("h") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::hours(value));

        throw std::runtime_error(details::error<std::chrono::duration<Rep, Period>>("parse error"));
    }
             
    // std::chrono::time_point<Clock, Duration>:
    //
    
    template <typename Clock, typename Dur, typename CharT, typename Traits>
    std::chrono::time_point<Clock, Dur>
    read(read_tag<std::chrono::time_point<Clock, Dur>>, std::basic_istream<CharT,Traits>&in)
    {
        return std::chrono::time_point<Clock, Dur>( read<Dur>(in) );
    }
    
    // std::tuple<Ts...>:
    //

    template <typename ...Ts, typename CharT, typename Traits>
    std::tuple<Ts...> 
    read(read_tag<std::tuple<Ts...>>, std::basic_istream<CharT,Traits>&in)
    {
        std::tuple<Ts...> ret;

        if (!details::consume('(', in))
            throw std::runtime_error(details::error<std::tuple<Ts...>>("parse error"));

        details::read_on<std::tuple<Ts...>, sizeof...(Ts)>::apply(ret, in);

        if (!details::consume(')', in))
            throw std::runtime_error(details::error<std::tuple<Ts...>>("parse error"));

        return ret;
    }

#ifdef MORE_VARIANT_HPP
    template <typename ...Ts> struct read_variant;
    template <typename T0, typename ...Ts>
    struct read_variant<T0, Ts...>
    {
        template <typename ...Ti, typename CharT, typename Traits>
        static more::variant<Ti...>
        run(std::basic_istream<CharT, Traits> &in)
        {
            try
            {
                auto v = read<T0>(in);
                return more::variant<Ti...>(v);
            }
            catch(...)
            {
                return read_variant<Ts...>::template run<Ti...>(in);
            }
        }
    };
    template <>
    struct read_variant<>
    {
        template <typename ...Ti, typename CharT, typename Traits>
        static more::variant<Ti...>
        run(std::basic_istream<CharT, Traits> &)
        {
            throw std::runtime_error(details::error<more::variant<Ti...>>("parse error")); 
        }
    };

    template <typename ...Ts, typename CharT, typename Traits>
    more::variant<Ts...> 
    read(read_tag<more::variant<Ts...>>, std::basic_istream<CharT,Traits>&in)
    {
        return read_variant<Ts...>::template run<Ts...>(in);
    }
#endif

    // std::string:
    //

    template <typename CharT, typename Traits>
    std::string 
    read(read_tag<std::string>, std::basic_istream<CharT,Traits>&in)
    {
        typedef std::string::traits_type traits_type;

        in >> std::noskipws >> std::ws;

        std::string str; str.reserve(32);
        traits_type::char_type c;

        enum class pstate { null, raw_string, escaped_char, quoted_string, escaped_char2 };
        auto state = pstate::null;

        auto raw_char = [](traits_type::char_type c) -> bool 
        {
            return std::isalnum(c) || traits_type::eq(c, '_') || traits_type::eq(c, '-');
        };

        bool stop = false, quoted = false; 

        while (!stop)
        {     
            c = static_cast<traits_type::char_type>(in.peek());
            
            if (c == traits_type::eof())
                break;

            switch(state)
            {
            case pstate::null:
                {
                    if (c == '"')       { in.get(); state = pstate::quoted_string; break;}
                    if (raw_char(c))    { in.get(); state = pstate::raw_string; str.push_back(c); break;}
                    if (c == '\\')      { in.get(); state = pstate::escaped_char;  break;} 

                    stop = true;

                } break;

            case pstate::raw_string:
                {
                    if (raw_char(c))    { in.get(); state = pstate::raw_string; str.push_back(c); break; }
                    if (c == '\\')      { in.get(); state = pstate::escaped_char; break; } 

                    stop = true;

                } break;

            case pstate::escaped_char:
                {
                    in.get(); state = pstate::raw_string; str.push_back(c);

                } break;

            case pstate::quoted_string:
                {
                    if (c == '"')       { in.get(); state = pstate::quoted_string; stop = true; quoted = true; break;}
                    if (c == '\\')      { in.get(); state = pstate::escaped_char2; break;} 

                    in.get(); str.push_back(c); 

                } break;

            case pstate::escaped_char2:
                {
                    in.get(); str.push_back(c); state = pstate::quoted_string;

                } break;
            }
        }

        in >> std::skipws;

        if (!quoted && str.empty())
            throw std::runtime_error(details::error<std::string>("parse error"));

        return str;
    }
    
    // container:
    //
    
    template <typename Tp, typename CharT, typename Traits>
    typename std::enable_if<
        more::traits::is_container<Tp>::value,
    Tp>::type
    read(read_tag<Tp>, std::basic_istream<CharT,Traits>& in)
    {
        Tp ret;

        if (!details::consume('[', in))
            throw std::runtime_error(details::error<Tp>("parse error"));

        while (!details::consume(']', in))
        {
            details::insert(ret, read<typename Tp::value_type>(in));
        }

        return ret;
    }
    
    // generic types supporting extraction operator>>():
    //
    
    template <typename Tp, typename CharT, typename Traits>
    typename std::enable_if<
        !more::traits::is_container<Tp>::value,
    Tp>::type
    read(read_tag<Tp>, std::basic_istream<CharT,Traits>& in)
    {
        Tp ret{};
        if (!(in >> ret))
            throw std::runtime_error(details::error<int>("parse error"));
        return ret;
    }
    
    //////////////////////////////////////////////////////////////////////////////////
    //
    // interfaces...
    //

    // try_read: work with file and string streams:
    
    template <typename T, typename CharT, typename Traits>
    inline bool try_read(std::basic_istream<CharT,Traits>&in)
    {
        auto p = in.tellg();
        try 
        {
            read<T>(in);
            in.clear();
            if (!in.seekg(p))
                std::runtime_error("try_read: seekg");
            return true;
        }
        catch(...)
        {
            in.clear();
            if (!in.seekg(p))
                std::runtime_error("try_read: seekg");
            return false;
        }
    }

    template <typename T, typename CharT, typename Traits>
    inline 
    T read(std::basic_istream<CharT,Traits>&in)
    {
        auto pos = in.tellg();

        try
        {
            return read(read_tag<typename details::decay<T>::type>(), in);
        }
        catch(...)
        {
            in.clear();
            if (!in.seekg(pos))
                std::runtime_error("read: seekg");
            throw;
        }
    }

    template <typename T>
    std::pair<T, std::string> read(std::string const &ref)
    {
        std::istringstream in(ref);

        auto value = read<T>(in);
        auto res   = in.str();
        auto count = static_cast<std::string::size_type>(in.rdbuf()->in_avail());
        if (count == static_cast<std::string::size_type>(-1))
            throw std::runtime_error("read: in_avail");
        
        return std::make_pair(std::move(value), res.substr(res.size()-count, count));
    }

} // namespace more_read

