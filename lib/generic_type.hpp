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
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct base_args
    { 
        virtual ~base_args() { }; 
    };

    template <typename ...Ts>
    struct args_pack : public base_args
    {
        std::tuple<Ts...> value;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename Tp>
    struct base_type 
    {
        template <typename Typ, typename CharT, typename Traits>
        friend typename std::basic_ostream<CharT, Traits> &
        operator<<(std::basic_ostream<CharT,Traits>& out, base_type<Typ> const& that);
    
        template <typename Typ, typename CharT, typename Traits>
        friend typename std::basic_istream<CharT, Traits> &
        operator>>(std::basic_istream<CharT,Traits>& in, base_type<Typ>&  that);

    private:

        std::string ctor_;
        std::shared_ptr<generic::base_args> args_;

    public:

        virtual ~base_type() {}

        virtual void type(std::string const &t) = 0;

        template <typename ... Ts>
        std::tuple<Ts...> &
        data_as()
        {
            if (ctor_.empty())    
                throw std::logic_error("object not constructed");

            return dynamic_cast<args_pack<Ts...> &>(*args_).value;
        }

        template <typename ... Ts>
        std::tuple<Ts...> const &
        data_as() const
        {
            if (ctor_.empty())    
                throw std::logic_error("object not constructed");
            
            return dynamic_cast<args_pack<Ts...> const &>(*args_).value;
        }
    };


    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, base_type<Tp> const& that)
    {
        auto it = Tp::visitor_map().find(that.ctor_);
        if (it == std::end(Tp::visitor_map()))
            throw std::runtime_error(demangle(typeid(Tp).name()) + ": " + that.ctor_ + " unknown constructor");

        out << that.ctor_;
        
        it->second->show(that.ctor_, out, that.args_);

        return out;
    }

    template <typename Tp, typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, base_type<Tp>&  that)
    {
        std::string ctor_;

        if (!(in >> ctor_))
            return in;

        auto it = Tp::visitor_map().find(ctor_);
        if (it == std::end(Tp::visitor_map()))
            throw std::runtime_error(demangle(typeid(Tp).name()) + ": " + ctor_ + " unknown constructor");

        that.type(ctor_);

        it->second->read(ctor_, in, that.args_);
        
        that.ctor_ = std::move(ctor_);

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct base_visitor
    {
        virtual void read(std::string const &base_type, std::istream &stream, std::shared_ptr<base_args> &) = 0;
        virtual void show(std::string const &base_type, std::ostream &stream, std::shared_ptr<base_args> const &) = 0;
    };


    template <typename ... Ts>
    struct visitor : public base_visitor
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

        virtual void read(std::string const &type, std::istream &in, std::shared_ptr<base_args> &args)
        {
            auto sp = std::make_shared<args_pack<Ts...>>();

            more::tuple_for_each(sp->value, read_on(type, in));
            
            args = std::move(sp);
        }

        virtual void show(std::string const &type, std::ostream &out, std::shared_ptr<base_args> const &args)
        {
            auto t = dynamic_cast<args_pack<Ts...> const &>(*args).value;
            more::tuple_for_each(t, show_on(type, out));
        }
    
    };

} // namespace generic 


#define GENERIC_GET_CTOR(x, ...)     x
#define GENERIC_GET_CSTR(x, ...)     #x
#define GENERIC_GET_PAIR(x, ...)   { #x, std::make_shared<generic::visitor<__VA_ARGS__>>() }

#define GENERIC_TYPE(T, ...) \
    struct T ## _type \
    { \
        static std::map<std::string, std::shared_ptr<generic::base_visitor>> & \
        visitor_map() \
        { \
            static std::map<std::string, std::shared_ptr<generic::base_visitor>> instance = { FOR_EACH(GENERIC_GET_PAIR, __VA_ARGS__) }; \
            return instance; \
        } \
    }; \
    \
    struct T : generic::base_type<T ## _type> \
    { \
        enum type_ctor { unknown, FOR_EACH(GENERIC_GET_CTOR,__VA_ARGS__) }; \
        \
        T() \
        : type_(unknown) \
        {} \
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

