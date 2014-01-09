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
#include <memory>
#include <cxxabi.h>

inline namespace more_cxxabi
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
    
    template <typename T>
    inline std::string
    type_name()
    {
        return demangle(typeid(T).name());
    }
    
    template <typename Tp>
    std::string __type_of(typename std::remove_reference<Tp>::type &&)
    {
        auto name = demangle(typeid(Tp).name());
        if (std::is_const<
             typename std::remove_reference<Tp>::type>::value)
            name.append(" const");

        if (std::is_volatile<
             typename std::remove_reference<Tp>::type>::value)
            name.append(" volatile");

        return name.append("&&");
    }

    template <typename Tp>
    std::string __type_of(typename std::remove_reference<Tp>::type &)
    {
        auto name = demangle(typeid(Tp).name());
        if (std::is_const<
             typename std::remove_reference<Tp>::type>::value)
            name.append(" const");

        if (std::is_volatile<
             typename std::remove_reference<Tp>::type>::value)
            name.append(" volatile");

        return name.append("&");
    }

    template <typename T>
    inline std::string
    type_of(T && arg)
    {
        return __type_of<T>(std::forward<T>(arg));
    }


}
