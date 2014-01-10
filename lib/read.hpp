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


    //
    // specializations for specific types...
    //

    template <typename CharT, typename Traits>
    void 
    read(bool &ret, std::basic_istream<CharT,Traits>&in)
    {
        in >> std::noboolalpha;

        if (!(in >> ret)) {
            in.clear();
            if (!(in >> std::boolalpha >> ret))
                throw std::runtime_error(details::error<bool>("parse error"));
        }
    }

    // pointers:
    //
    
    template <typename CharT, typename Traits>
    void
    read(const char * &ret, std::basic_istream<CharT,Traits>&in)
    {
        std::string tmp;

        if (!(in >> tmp)) {
            throw std::runtime_error(details::error<const char *>("parse error"));
        }

        if (!tmp.empty())
        {
            auto ptr = reinterpret_cast<char *>(malloc(tmp.size()+1));
            std::strcpy(ptr, tmp.c_str());
            ret = reinterpret_cast<const char *>(ptr);
        }
        else
            ret = nullptr;
    }

    template <typename CharT, typename Traits, typename Tp>
    void
    read(Tp * &ret, std::basic_istream<CharT,Traits>&in)
    {
        auto ptr = new Tp{};

        if (!(in >> *ptr)) {
            delete ptr;
            throw std::runtime_error(details::error<Tp *>("parse error"));
        }
        
        ret = ptr;
    }

    template <typename CharT, typename Traits, typename Tp>
    void
    read(std::shared_ptr<Tp> &ret, std::basic_istream<CharT,Traits>&in)
    {
        auto ptr = std::make_shared<Tp>();

        if (!(in >> *ptr)) 
            throw std::runtime_error(details::error<Tp *>("parse error"));
        
        ret = std::move(ptr);
    }

    template <typename CharT, typename Traits, typename Tp>
    void
    read(std::unique_ptr<Tp> &ret, std::basic_istream<CharT,Traits>&in)
    {
        auto ptr = std::unique_ptr<Tp>(new Tp{});

        if (!(in >> *ptr)) {
            throw std::runtime_error(details::error<Tp *>("parse error"));
        }
        
        ret = std::move(ptr);
    }

    // pair<T1,T2>:
    //
    
    template <typename T1, typename T2, typename CharT, typename Traits>
    void
    read(std::pair<T1,T2> &ret, std::basic_istream<CharT,Traits>&in)
    {
        if (details::consume('(', in)) {

            T1 a = read<T1>(in); 
            T2 b = read<T2>(in);

            if (!details::consume(')', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            ret = std::make_pair(std::move(a), std::move(b));
        }
        else if (details::consume('[', in)) {

            T1 a = read<T1>(in); details::consume("->", in); 
            T2 b = read<T2>(in);

            if (!details::consume(']', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            ret = std::make_pair(std::move(a), std::move(b));
        }
        else {

            T1 a = read<T1>(in); details::consume("->", in); 
            T2 b = read<T2>(in);

            ret = std::make_pair(std::move(a), std::move(b));
        }
    }
    
    // std::array<T,N>:
    //

    template <typename T, size_t N, typename CharT, typename Traits>
    void
    read(std::array<T,N> &ret, std::basic_istream<CharT,Traits>&in)
    {
        if (!details::consume('[', in))
            throw std::runtime_error(details::error<std::array<T,N>>("parse error"));

        for(auto & e : ret)
        {
            e = read<T>(in);
        }

        if (!details::consume(']', in))
            throw std::runtime_error(details::error<std::array<T,N>>("parse error"));
    }

    // std::chrono::duration<Rep, Period>:
    //
    
    template <typename Rep, typename Period, typename CharT, typename Traits>
    void
    read(std::chrono::duration<Rep, Period> &ret, std::basic_istream<CharT,Traits>&in)
    {
        typedef std::chrono::duration<Rep, Period> Duration;
        
        int64_t value; char c; std::string unit;

        if (!(in >> value >> c >> unit))
            throw std::runtime_error(details::error<std::chrono::duration<Rep, Period>>("parse error"));

        if (unit.compare("ns") == 0)
            ret = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(value));
        else if (unit.compare("us") == 0)
            ret = std::chrono::duration_cast<Duration>(std::chrono::microseconds(value));
        else if (unit.compare("ms") == 0)
            ret = std::chrono::duration_cast<Duration>(std::chrono::milliseconds(value));
        else if (unit.compare("s") == 0)
            ret = std::chrono::duration_cast<Duration>(std::chrono::seconds(value));
        else if (unit.compare("m") == 0)
            ret = std::chrono::duration_cast<Duration>(std::chrono::minutes(value));
        else if (unit.compare("h") == 0)
            ret = std::chrono::duration_cast<Duration>(std::chrono::hours(value));
        else
            throw std::runtime_error(details::error<std::chrono::duration<Rep, Period>>("parse error"));
    }
             
    // std::chrono::time_point<Clock, Duration>:
    //
    
    template <typename Clock, typename Dur, typename CharT, typename Traits>
    void
    read(std::chrono::time_point<Clock, Dur>& ret, std::basic_istream<CharT,Traits>&in)
    {
        ret = std::chrono::time_point<Clock, Dur>( read<Dur>(in) );
    }
    
    // std::tuple<Ts...>:
    //

    template <typename ...Ts, typename CharT, typename Traits>
    void 
    read(std::tuple<Ts...> &ret, std::basic_istream<CharT,Traits>&in)
    {
        if (!details::consume('(', in))
            throw std::runtime_error(details::error<std::tuple<Ts...>>("parse error"));

        details::read_on<std::tuple<Ts...>, sizeof...(Ts)>::apply(ret, in);

        if (!details::consume(')', in))
            throw std::runtime_error(details::error<std::tuple<Ts...>>("parse error"));
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
    void
    read(more::variant<Ts...>& ret, std::basic_istream<CharT,Traits>&in)
    {
        ret = read_variant<Ts...>::template run<Ts...>(in);
    }
#endif

    // std::string:
    //

    template <typename CharT, typename Traits>
    void
    read(std::string &ret, std::basic_istream<CharT,Traits>&in)
    {
        typedef std::string::traits_type traits_type;

        in >> std::noskipws >> std::ws;

        ret.reserve(32);
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
                    if (raw_char(c))    { in.get(); state = pstate::raw_string; ret.push_back(c); break;}
                    if (c == '\\')      { in.get(); state = pstate::escaped_char;  break;} 

                    stop = true;

                } break;

            case pstate::raw_string:
                {
                    if (raw_char(c))    { in.get(); state = pstate::raw_string; ret.push_back(c); break; }
                    if (c == '\\')      { in.get(); state = pstate::escaped_char; break; } 

                    stop = true;

                } break;

            case pstate::escaped_char:
                {
                    in.get(); state = pstate::raw_string; ret.push_back(c);

                } break;

            case pstate::quoted_string:
                {
                    if (c == '"')       { in.get(); state = pstate::quoted_string; stop = true; quoted = true; break;}
                    if (c == '\\')      { in.get(); state = pstate::escaped_char2; break;} 

                    in.get(); ret.push_back(c); 

                } break;

            case pstate::escaped_char2:
                {
                    in.get(); ret.push_back(c); state = pstate::quoted_string;

                } break;
            }
        }

        in >> std::skipws;

        if (!quoted && ret.empty())
            throw std::runtime_error(details::error<std::string>("parse error"));

    }
    
    // container:
    //
    
    template <typename Tp, typename CharT, typename Traits>
    typename std::enable_if<
        more::traits::is_container<Tp>::value>::type
    read(Tp &ret, std::basic_istream<CharT,Traits>& in)
    {
        if (!details::consume('[', in))
            throw std::runtime_error(details::error<Tp>("parse error"));

        while (!details::consume(']', in))
        {
            details::insert(ret, read<typename Tp::value_type>(in));
        }
    }
    
    // generic types supporting extraction operator>>():
    //
    
    template <typename Tp, typename CharT, typename Traits>
    typename std::enable_if<
        !more::traits::is_container<Tp>::value>::type
    read(Tp &ret, std::basic_istream<CharT,Traits>& in)
    {
        if (!(in >> ret))
            throw std::runtime_error(details::error<int>("parse error"));
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
            typename details::decay<T>::type ret;
            read(ret, in);
            return ret;
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

