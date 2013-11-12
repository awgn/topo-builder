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

namespace generic {

    template <typename Tp>
    struct type 
    {
        std::string ctor;
        std::vector<std::string> args;
    };

    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, type<Tp> const& that)
    {
        out << that.ctor;
        for(auto const & a : that.args)
            out << ' ' << a;
        return out;
    }

    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, type<Tp>&  that)
    {
        std::string ctor, arg;

        if (!(in >> ctor))
            return in;

        auto it = Tp::ctors().find(ctor);
        if (it == std::end(Tp::ctors()))
            throw std::runtime_error("parse error: type '" + demangle(typeid(Tp).name()) + "' unknown constructor " + ctor);

        that.ctor = ctor;
        that.args.clear();

        for(int i = 0; i < it->second; ++i)
        {
            if (!(in >> arg))
                return in;

            that.args.push_back(std::move(arg));
        }

        return in;
    }
    
    template <typename Tp>
    inline std::string
    show(const type<Tp> &opt, const char * n = nullptr)
    {
        std::string s;
        if (n) {
            s += std::string(n) + ' ';
        }
        std::ostringstream ss; ss << opt;
        return s + ss.str();
    }

} // namespace generic 


#define GENERIC_TYPE(family, ...) \
    struct family \
    { \
        static std::map<std::string, int> & \
        ctors() \
        { \
            static std::map<std::string, int> instance = { __VA_ARGS__ }; \
            return instance; \
        } \
    }; \
    typedef generic::type<family> family ## _type;

