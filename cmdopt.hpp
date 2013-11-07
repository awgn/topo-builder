#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

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


#define OPTION(family, ...) \
    struct family \
    { \
        static std::map<std::string, std::string> opts; \
    }; \
    std::map<std::string, std::string> family::opts = { __VA_ARGS__ };


namespace cmd {

    template <typename Tp>
    struct option 
    {
        std::string opt;
        std::string arg;
    };


    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, option<Tp> const& that)
    {
        return out << that.opt << ' ' << that.arg;
    }

    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, option<Tp>&  that)
    {
        std::string opt, arg;

        if (!(in >> opt))
            return in;

        auto it = Tp::opts.find(opt);
        if (it == std::end(Tp::opts))
            throw std::runtime_error("family option '" + demangle(typeid(Tp).name()) + "' : unknown type '" + opt + "'");

        if (!(in >> arg))
            return in;
        
        that.opt = it->second;
        that.arg = std::move(arg);

        return in;
    }

}

/* 
 
   OPTION(type, { "test", "--test" },
                { "prova", "-p"    }  
      )
 */

