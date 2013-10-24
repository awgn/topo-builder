/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#ifndef _MORE_PRINT_HPP_
#define _MORE_PRINT_HPP_ 

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <cassert>

//////////////////////////////////////////////////////////////////
// print and sprint functions: both are inspired to boost::format
// but are just ~5x times faster. Nicola
//

namespace more { 
    
    ///////////////////////////////////////////////////////////////////////
    // flags manipulator: example -> flag<std::ios::hex>(42)
    // 

    template <std::ios_base::fmtflags Fs, typename Tp>
    struct _flags
    {
        _flags(Tp val)
        : value(val)
        {}
        Tp value;
    };

    template <typename CharT, typename Traits, typename Tp, std::ios_base::fmtflags Fs>
    inline typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const _flags<Fs,Tp> & rhs)
    {
        std::ostream tmp(out.rdbuf());
        tmp.flags(Fs);
        return tmp << rhs.value;
    }
    
    // helper function...

    template <std::ios_base::fmtflags Fs, typename Tp>
    _flags<Fs,Tp> flags(Tp n)
    {
        return _flags<Fs,Tp>(n);
    }

    ///////////////////////////////////////////////////////////////////////////
    // print: example -> more::print(cout, "%1 %2", std::string("hello"), 
    //                                              this_thread::get_id());
    //

    namespace detail {

        template <typename CharT, typename Traits>
        inline void stream_on(std::basic_ostream<CharT, Traits> &, int, int) 
        {
            throw std::runtime_error("%format error%");
        }

        template <typename CharT, typename Traits, typename T, typename ... Ts>
        inline void stream_on(std::basic_ostream<CharT, Traits> &out, int n, int x, const T &arg0, const Ts& ...args) 
        {
            if (n != x) {
                stream_on(out, n, x+1, args...);
                return;
            }
            out << arg0;
        }
    } // namespace detail

    template <typename CharT, typename Traits, typename T0>
    inline void print(std::basic_ostream<CharT, Traits> &out, const T0 &a0)
    {
        out << a0;
    }

    template <typename CharT, typename Traits, typename ... Ts>
    void print(std::basic_ostream<CharT, Traits> &out, const char *fmt, const Ts&... args)
    {
        enum class state { zero, percent, digit }; 
        state s = state::zero;
        int n = 0;

        for(const char * p = fmt; *p != '\0'; ++p)
        {
            const char c = *p;
            switch(s)
            {
            case state::zero: 
                {
                    if(c != '%') {
                        out.put(c); continue;
                    }
                    s = state::percent; continue;      
                }
            case state::percent:
                {
                    if(isdigit(c)) {
                        n = (n*10)+(c-'0');
                        s = state::digit; continue;
                    }
                    if(c == '%')  {
                        out.put('%');
                        s = state::zero; continue;
                    }
                    throw std::runtime_error("%format error%");
                }
            case state::digit:
                {
                    if (isdigit(c)) {
                        n = (n*10)+(c-'0');
                        continue;
                    }
                    if (c == '%') {
                        detail::stream_on(out, n, 1, args...);
                        n = 0; s = state::percent; continue;
                    }
                    detail::stream_on(out, n, 1, args...);
                    out.put(c); 
                    n = 0; s = state::zero; continue;
                }
            }    
        }
        if (s == state::digit)
            detail::stream_on(out, n, 1, args...);
        
        if (s == state::percent)
            throw std::runtime_error("%format error%");
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // print: example -> more::sprint("%1 %2", std::string("hello"), 
    //                                          this_thread::get_id());
    //
 
    template <typename ... Ts>
    inline std::string sprint(const Ts& ... args)
    {
        std::ostringstream out;
        print(out, args...);
        return out.str();  
    }

    // portable memory streambuffer
    //

    struct membuf : public std::streambuf
    {
        membuf(char *buffer, size_t size)
        {
            this->setp(buffer,buffer+size);
        }
            
        char *end() 
        {
            return this->pptr();
        }
    };

    template <typename ... Ts>
    inline void bprint(char *buffer, size_t len, const Ts& ... args)
    {
        membuf sb(buffer,len-1);
        std::ostream out(&sb);
        print(out, args...);
        *sb.end() = '\0';
    }
 
} // namespace more

#endif /* _MORE_PRINT_HPP_ */
