#pragma once 

#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <sstream>

#include <cxxabi.h>

namespace
{
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
}

// 
// declare a command line option:
// 
// OPTION(type, { "test",  { "--test", true } },
//              { "prova", { "-p",    false } }  
//    )
// 
// type opt;
// 
// std::cin  >> opt;
// std::cout << opt;
// 


namespace generic {

    template <typename Tp>
    struct option 
    {
        std::string opt;
        std::vector<std::string> args;
    };

    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, option<Tp> const& that)
    {
        out << that.opt;
        for(auto const & x : that.args)
            out << ' ' << x;
        return out;
    }

    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, option<Tp>&  that)
    {
        std::string opt, arg;

        if (!(in >> opt))
            return in;

        auto it = Tp::options().find(opt);
        if (it == std::end(Tp::options()))
            throw std::runtime_error("parse error: kind '" + demangle(typeid(Tp).name()) + "' unknown option type " + opt );

        that.opt = it->second.first;
        that.args.clear();

        for(int i = 0; i < it->second.second; ++i)
        {
            if (!(in >> arg))
                return in;

            that.args.push_back(std::move(arg));
        }

        return in;
    }
    
    template <typename Tp>
    inline std::string
    show(const option<Tp> &opt, const char * n = nullptr)
    {
        std::string s;
        if (n) {
            s += std::string(n) + ' ';
        }
        std::ostringstream ss; ss << opt;
        return s + ss.str();
    }


} // namespace generic 


#define OPTION_KIND(family, ...) \
    struct family \
    { \
        static std::map<std::string, std::pair<std::string, int> > & \
        options() \
        { \
            static std::map<std::string, std::pair<std::string, int> > instance = { __VA_ARGS__ }; \
            return instance; \
        } \
    }; \
    typedef generic::option<family> family ## _type;

