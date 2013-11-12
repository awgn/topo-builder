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

#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

#include <cxxabi.h>

#include <tuple_ext.hpp>
#include <macro.hpp>

namespace generic {

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


    template <typename Tp>
    struct base_type 
    {
    private:
        std::string ctor_;

    public:

        std::string ctor() const
        {
            return ctor_;
        }

        void ctor(std::string name)
        {
            ctor_ = std::move(name);
        }
    
        virtual void type(const std::string &) = 0;

        template <typename ... Ts>
        std::tuple<Ts...> &
        arg_as()
        {
            std::tuple<Ts...> ret;

            auto it = Tp::ctors_map().find(ctor_);
            if (it == std::end(Tp::ctors_map()))
                throw std::logic_error("internal error");

           return * reinterpret_cast<std::tuple<Ts...> *>(it->second->args());
        }

        template <typename ... Ts>
        std::tuple<Ts...> const &
        arg_as() const
        {
            std::tuple<Ts...> ret;

            auto it = Tp::ctors_map().find(ctor_);
            if (it == std::end(Tp::ctors_map()))
                throw std::logic_error("internal error");

           return * reinterpret_cast<std::tuple<Ts...> *>(it->second->args());
        }
    };


    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, base_type<Tp> const& that)
    {
        out << that.ctor();
        
        auto & exec = Tp::ctors_map()[that.ctor()];
        if (exec)
            exec->show(that.ctor(), out);

        return out;
    }


    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, base_type<Tp>&  that)
    {
        std::string ctor_, arg_;

        if (!(in >> ctor_))
            return in;

        auto it = Tp::ctors_map().find(ctor_);
        if (it == std::end(Tp::ctors_map()))
            throw std::runtime_error(demangle(typeid(Tp).name()) + ": parse error: " + ctor_ + " unknown constructor");

        that.ctor(ctor_);
        that.type(ctor_);

        auto & exec = Tp::ctors_map()[ctor_];
        if (exec)
            exec->read(ctor_, in);

        return in;
    }
    
    template <typename Tp>
    inline std::string
    show(const base_type<Tp> &opt, const char * n = nullptr)
    {
        std::string s;
        if (n) {
            s += std::string(n) + ' ';
        }
        std::ostringstream ss; ss << opt;
        return s + ss.str();
    }


    struct base_arg
    {
        virtual void read(std::string const &base_type, std::istream &stream) = 0;
        virtual void show(std::string const &base_type, std::ostream &stream) = 0;
        virtual void *args() = 0;
    };


    template <typename ... Ts>
    struct _arg : public base_arg
    {
        struct read_on
        {
            std::istream &in_;
            std::string const &type_;

            read_on(const std::string &type, std::istream &in)
            : in_(in)
            , type_(type)
            {}

            template <typename Tp>
            void operator()(Tp &data)
            {
                if (!(in_ >> data))
                    throw std::runtime_error(type_ + ": parse error");
            }
        };

        struct show_on
        {
            std::ostream &out_;
            std::string const &type_;
            
            show_on(std::string const &type, std::ostream &out)
            : out_(out)
            , type_(type)
            {}

            template <typename Tp>
            void operator()(Tp &data)
            {
                out_ << ' ' << data;
            }
        };

        virtual void read(std::string const &type, std::istream &in)
        {
            more::tuple_for_each(pack_, read_on(type, in));
        }

        virtual void show(std::string const &type, std::ostream &out)
        {
            more::tuple_for_each(pack_, show_on(type, out));
        }
    
        virtual void *args() 
        {
            return static_cast<void *>(&pack_);
        }

        std::tuple<Ts...> pack_;
    };


    template <typename ... Ts>
    std::shared_ptr<_arg<Ts...>>
    arg()
    {
        return std::make_shared<_arg<Ts...>>();
    }

} // namespace generic 


#define GENERIC_GET_CTOR(x, ...)   x
#define GENERIC_GET_CSTR(x, ...)    #x
#define GENERIC_GET_PAIR(x, ...)   { #x, generic::arg<__VA_ARGS__>() }


#define GENERIC_TYPE(T, ...) \
    struct T ## _type \
    { \
        static std::map<std::string, std::shared_ptr<generic::base_arg>> & \
        ctors_map() \
        { \
            static std::map<std::string, std::shared_ptr<generic::base_arg>> instance = { FOR_EACH(GENERIC_GET_PAIR, __VA_ARGS__) }; \
            return instance; \
        } \
    }; \
    struct T : generic::base_type<T ## _type> \
    { \
        enum type_ctor { unknown, FOR_EACH(GENERIC_GET_CTOR,__VA_ARGS__) }; \
        \
        type_ctor type() const \
        { \
            return type_; \
        } \
        \
        void \
        type(std::string const &t) \
        { \
            static std::vector<std::string> xs = { "", FOR_EACH(GENERIC_GET_CSTR, __VA_ARGS__) }; \
            auto it = std::find(std::begin(xs), std::end(xs), t); \
            if (it == std::end(xs)) \
                type_ = unknown; \
            else \
                type_ = static_cast<type_ctor>(std::distance(std::begin(xs), it)); \
        } \
        \
    private: \
        type_ctor type_; \
    };

