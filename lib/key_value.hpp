/* ----------------------------------------------------------------------------
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _KEY_VALUE_HPP_
#define _KEY_VALUE_HPP_ 

#include <type_traits.hpp>      // more!
#include <typemap.hpp>          // more!

#ifdef LEXEME_DEBUG
#include <show.hpp>             // more!
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <memory>
#include <functional>
#include <type_traits>
#include <tuple>
#include <limits>
#include <stdexcept>
#include <cassert>

#include <cxxabi.h>

#define MAP_KEY(t,k)  struct k { \
    typedef std::pair<k,t> type; \
    static constexpr bool has_default = false; \
    static constexpr const char * str() \
    { return # k; } \
};


#define MAP_KEY_VALUE(t,k,v)  struct k { \
    typedef std::pair<k,t> type; \
    static constexpr bool has_default = true; \
    static constexpr const char * str() \
    { return # k; } \
    static t default_value() \
    { return v; } \
};


//////////////////////////////////
//  key-value config file parser 


namespace more { 

    namespace details {

#ifdef LEXEME_DEBUG
        const char * const BOLD  = "\E[1m";
        const char * const RESET = "\E[0m";
        const char * const BLUE  = "\E[1;34m";
        const char * const RED   = "\E[1;31m";
#endif

        static inline
        std::string
        demangle(const char *name)
        {
            int status;
            auto deleter = [](void *a) { ::free(a); };

            std::unique_ptr<char, decltype(deleter)> 
                ret(abi::__cxa_demangle(name, 0, 0, &status), deleter);

            if (status < 0) {
                return std::string("?");
            }
            return ret.get();
        }        
        
        template <typename Tp>
        inline std::string type_name()
        {
            return demangle(typeid(Tp).name());
        }

        class streambuf : public std::streambuf 
        {
            enum class state 
            {
                none, string1, string2, comment, backslash 
            };

        public:
            streambuf(std::streambuf *in, char commkey = '#')
            : m_in(in)
            , m_line(1)
            , m_commkey(commkey)
            , m_state(state::none)
            {
                if (m_commkey == '\'' ||
                    m_commkey == '"')
                    throw std::runtime_error("streambuf: invalid comment-key");
            }

            virtual int_type underflow()
            {
                int_type c = m_in->sgetc();
                if (c == EOF)
                    return c;

                auto ns = next_(c);

                return ns == state::comment ||
                    ( ns == state::backslash && m_state == state::comment) ? ' ' : c;
            }

            virtual int_type uflow()
            {
                int_type c = m_in->sbumpc();
                if (c == EOF)
                    return c;
                
                if (c == '\n')
                    m_line++;

                auto prev = m_state; 
                m_state = next_(c);
                m_prev  = prev;

                return m_state == state::comment ||
                ( m_state == state::backslash && m_prev == state::comment) ? ' ' : c;
            }

            int line() const
            { 
                return m_line; 
            }

        private:

            state next_(int_type c)
            {
                switch(m_state)
                {
                case state::none:
                    return  c == '\''   ? state::string1   :
                    c == '"'            ? state::string2   :
                    c == m_commkey      ? state::comment   : 
                    c == '\\'           ? state::backslash : state::none;

                case state::string1:
                    return  c == '\''   ? state::none      : 
                    c == '\\'           ? state::backslash : state::string1;

                case state::string2:
                    return  c == '"'    ? state::none      :
                    c == '\\'           ? state::backslash : state::string2;

                case state::comment:
                    return  c == '\n'   ? state::none      : 
                    c == '\\'           ? state::backslash : state::comment;

                case state::backslash:
                    return m_prev;
                }

                return state::none;
            }

            std::streambuf * m_in;
            int     m_line;
            char    m_commkey;

            state   m_state, m_prev;
        };

        template <class CharT, class Traits>
        inline int line_number(std::basic_istream<CharT,Traits> &in)
        {
            streambuf * ln = dynamic_cast<streambuf *>(in.rdbuf());
            if (ln) {
                return ln->line();
            }
            return -1;
        }

        template <class CharT, class Traits>
        inline
        std::basic_istream<CharT,Traits> &
        ignore_line(std::basic_istream<CharT,Traits> &in_)
        {
            in_.ignore(std::numeric_limits<std::streamsize>::max(), in_.widen('\n'));
            return in_;
        }

        // key-value helper 
        //
        template <typename KEY, typename TYPE, bool has_default>
        struct get_default
        {
            static TYPE value()
            { return KEY::default_value(); }
        };
        template <typename KEY, typename TYPE>
        struct get_default<KEY, TYPE, false>
        {
            static TYPE value()
            { return TYPE(); }
        };

        // detail::tuple_helper<sizeof...(Ti)>::parse_lexeme(*this, tup) &&
        //
        template <size_t N>
        struct tuple_helper
        {
            template <typename L, typename ...Ti>
            static bool parse_lexeme(L &lex,  std::tuple<Ti...> &tup)
            {
                typename std::tuple_element<sizeof...(Ti)-N,
                         typename std::tuple<Ti...>   
                         >::type elem{}; 

                bool ok = lex.log_ret(lex.parse_lexeme(elem)).first;
                if (!ok) {
                    return false;
                }

                std::get<sizeof...(Ti) - N>(tup) = std::move(elem);
                return tuple_helper<N-1>::parse_lexeme(lex, tup);
            }
        };
        template <>
        struct tuple_helper<1>
        {
            template <typename L, typename ...Ti>
            static bool parse_lexeme(L &lex,  std::tuple<Ti...> &tup)
            {
                typename std::tuple_element<sizeof...(Ti)-1,
                         typename std::tuple<Ti...>   
                         >::type elem{};

                bool ok = lex.log_ret(lex.parse_lexeme(elem)).first;
                if (ok)
                    std::get<sizeof...(Ti) - 1>(tup) = std::move(elem);
                
                return ok;
            }
        };

        // get a mutable version of a generic type
        // 

        template <typename T>
        struct mutable_type
        {
            typedef typename std::remove_const<T>::type type;
        };
        template <typename T, typename V> 
        struct mutable_type<std::pair<T,V>>
        {
            typedef std::pair<typename std::remove_const<T>::type,
                    typename std::remove_const<V>::type> type;
        };
        template <typename ... Ti>
        struct mutable_type<std::tuple<Ti...>>
        {
            typedef std::tuple<typename std::remove_const<Ti>::type...> type;
        };

        //////////////// insert helpers

        template <typename T>
        bool insert_check(const std::pair<T,bool> &p)
        {
            return p.second;
        }
        template <typename T>
        bool insert_check(const T &)
        {
            return true;
        }

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
        template <typename C, typename K>
        typename std::enable_if<more::traits::has_key_type<C>::value, bool>::type 
        insert(C &cont, K && value)
        {
            return insert_check(cont.insert(std::forward<K>(value)));
        }

        // enabled for std::map, std::multimap, std::unordered_map,
        // std::unordered_multimap
        // 
        template <typename C, typename T, typename V>
        typename std::enable_if<more::traits::has_key_type<C>::value, bool>::type 
        insert(C &cont, std::pair<T,V> && value)
        {
            return insert_check(cont.insert(std::move(value)));
        }

    } // namespace details

    /////////////////////////////////////////////////////////////////////////////////////////

    template <typename ...T> struct key_value_pack;

    typedef std::tuple<
            bool,        // whether it is strict or non strict parsing 
            char,        // assign separator -> default =
            char,        // comment          -> default #
            std::string> parser_options;

    enum 
    {
        parser_mode,
        assign_key,
        comment_key,
        target_name
    };

    template <typename CharT, typename Traits>
    class lexer 
    {
        std::basic_istream<CharT, Traits> & m_in;
        parser_options m_option;

    public:

        lexer(std::basic_istream<CharT, Traits> &in, parser_options opt)
        : m_in(in)
        , m_option(std::move(opt))
        {
            m_in.unsetf(std::ios::dec);
            m_in.unsetf(std::ios::hex);
            m_in.unsetf(std::ios::oct);
        }

        ~lexer()
        {}

        bool _(std::basic_istream<char>::int_type c)
        {
            decltype(c) _c;

            if(!(m_in >> std::ws)) {
                return false;
            }
            _c = m_in.peek();
            if (!m_in) {
                return false;
            }

            if (c == _c) {
                m_in.get();
                assert(m_in);
                return true;
            }
            return false;
        }

        bool _(const char *s)
        {
            std::string _s; 
            m_in >> _s;

            return (m_in && _s.compare(s) == 0) ? true : false;
        }                                                            

        typedef std::pair<bool, std::string> ret_t;

        // wrapper for log/debug 
        //

        template <typename Tp, typename L = int>
        static inline
        ret_t make_ret(bool v, const L &elem = L())
        {
#ifdef LEXEME_DEBUG
            return v ? std::make_pair(true,  std::string(details::BLUE) + "lex<" + details::type_name<Tp>() + ">" + details::RESET + " -> [" + show(elem) + "]") :
                       std::make_pair(false, std::string(details::RED)  + "lex<" + details::type_name<Tp>() + ">" + details::RESET + " -> [...]");
#else
            (void)v; (void)elem; return std::make_pair(v, "");
#endif
        }

        static inline 
        ret_t log_ret(const ret_t &ret)
        {
#ifdef LEXEME_DEBUG
            std::cout << "   " << ret.second << std::endl;
#endif
            return ret;
        }

        // very generic parser for types supporting operator>> ...
        //

        template <typename T>
        typename std::enable_if<!more::traits::is_container<T>::value,ret_t>::type 
        parse_lexeme(T &lex)
        {
            static_assert(more::traits::has_extraction_operator<T>::value, "parse_lexeme: *** T must have a valid extraction operator>> ***");
            
            typename std::remove_const<T>::type e;

            if(m_in >> e)
                lex = std::move(e);

            return make_ret<T>(static_cast<bool>(m_in), lex);
        }        

        // parser for string literal:    
        //

        ret_t parse_lexeme(const char * &lex)
        {
            std::string s;

            auto ret = log_ret(parse_lexeme(s));

            if (!ret.first)
                return make_ret<const char *>(false);

            lex = strdup(s.c_str());    
            return make_ret<const char *>(true, lex);
        }

        // parser for raw pointer: 
        //

        template <typename T>
        ret_t parse_lexeme(T * &ptr)
        {
            if (!ptr)
                ptr = new T;

            auto ret = log_ret(parse_lexeme(*ptr));

            if (!ret.first)
                return make_ret<T>(false);

            return make_ret<T>(true, (void *)ptr);
        }

        // parser for shared_ptr<T>:
        //

        template <typename T>
        ret_t parse_lexeme(std::shared_ptr<T> &ptr)
        {
            if (!ptr)
                ptr.reset(new T);

            auto ret = log_ret(parse_lexeme(*ptr));
            if (!ret.first)
                return make_ret<T>(false);

            return make_ret<T>(true, (void *)ptr.get());
        }

        // parser for std::string:
        //
        ret_t parse_lexeme(std::string &lex)
        {
            typedef std::string::traits_type traits_type;
            
            m_in >> std::noskipws >> std::ws;

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
                c = static_cast<traits_type::char_type>(m_in.peek());
                if (c == traits_type::eof())
                    break;

                switch(state)
                {
                    case pstate::null:
                    {
                        if (c == '"')       { m_in.get(); state = pstate::quoted_string; break;}
                        if (raw_char(c))    { m_in.get(); state = pstate::raw_string; str.push_back(c); break;}
                        if (c == '\\')      { m_in.get(); state = pstate::escaped_char;  break;} 
                        
                        stop = true;
                    
                    } break;

                    case pstate::raw_string:
                    {
                        if (raw_char(c))    { m_in.get(); state = pstate::raw_string; str.push_back(c); break; }
                        if (c == '\\')      { m_in.get(); state = pstate::escaped_char; break; } 
                        
                        stop = true;

                    } break;

                    case pstate::escaped_char:
                    {
                        m_in.get(); state = pstate::raw_string; str.push_back(c);

                    } break;
                    
                    case pstate::quoted_string:
                    {
                        if (c == '"')       { m_in.get(); state = pstate::quoted_string; stop = true; quoted = true; break;}
                        if (c == '\\')      { m_in.get(); state = pstate::escaped_char2; break;} 
                        
                        m_in.get(); str.push_back(c); 
                        
                    } break;
                    
                    case pstate::escaped_char2:
                    {
                        m_in.get(); str.push_back(c); state = pstate::quoted_string;
                    
                    } break;
                }
            }

            lex = std::move(str);
            m_in >> std::skipws;

            return make_ret<std::string>((!quoted && lex.size() == 0) ? false : true, lex); 
        }

        // parser for boolean
        //

        ret_t parse_lexeme(bool &lex)
        {
            m_in >> std::noboolalpha;

            if (!(m_in >> lex)) {
                m_in.clear();
                m_in >> std::boolalpha >> lex;
            }
            
            return make_ret<bool>(m_in ? true : false, lex);
        }

        // parser for generic pairs
        // 

        template <typename T, typename V>
        ret_t parse_lexeme(std::pair<T,V> &lex)
        { 
            T first{}; V second{};

            bool ok;
            if (_('('))
            {
                ok =  parse_lexeme(first).first   &&
                      parse_lexeme(second).first  &&
                      _(')');  
            }
            else if (_('[')) {
                ok = parse_lexeme(first).first  &&
                     _("->")                    &&
                     parse_lexeme(second).first &&
                     _(']');
            }
            else {
                ok = parse_lexeme(first).first &&
                     _("->")                   &&
                     parse_lexeme(second).first;   
            }

            if (ok)
                lex = std::make_pair(first,second);

            return make_ret<std::pair<T,V>>(ok, lex);
        }

        // parser for generic tuple
        // 

        template <typename ...Ti>
        ret_t parse_lexeme(std::tuple<Ti...> &lex)
        {
            std::tuple<Ti...> tup;

            bool ok = _('(') && details::tuple_helper<sizeof...(Ti)>::parse_lexeme(*this, tup) && _(')');

            if (ok)
                lex = std::move(tup);          

            return make_ret<std::tuple<Ti...>>(ok, lex);
        }

        // parser for generic containers:
        //

        template <typename C>
        typename std::enable_if<more::traits::is_container<C>::value,ret_t>::type 
        parse_lexeme(C & lex)
        {
            bool ok = true;

            if (!_('[')) 
                return make_ret<C>(false);

            do {
                if (_(']')) 
                    break;

                typename details::mutable_type<
                    typename C::value_type>::type value;
                
                auto ret = log_ret(parse_lexeme(value));
                
                if ((ok = ret.first)) {
                    if (!details::insert(lex,std::move(value))) {
                        std::clog << std::get<target_name>(m_option) << 
                        ": insert error (dup value at line " << 
                        details::line_number(m_in) << ")" << std::endl;
                        return make_ret<C>(false);
                    }
                }
            }
            while(ok);
            return make_ret<C>(ok, lex);
        }

        // recursive parser for key_value_pack
        //
        template <typename ...Ti>
        ret_t parse_lexeme(key_value_pack<Ti...> &elem, bool bracket = true)
        {
            if (bracket &&  ! _('{')) {
                std::clog << std::get<target_name>(m_option) << 
                ": parse error: missing open bracket (line " << 
                details::line_number(m_in) << ")" << std::endl;
                return make_ret<key_value_pack<Ti...>>(false);
            }

            key_value_pack<Ti...> tmp;

            while(m_in) {

                m_in >> std::noskipws >> std::ws;

                std::string key;
                key.reserve(16);

                // parse the key 
                //
                std::string::traits_type::char_type c = '\0';

                while ((m_in >> c) && !isspace(c) && c != std::get<assign_key>(m_option) ) {
                    key.push_back(c);
                }

                if (key.empty())
                    continue;

                // got the key...
                //
                if (bracket && !key.compare("}")) {
                    bracket = false;
                    break;
                }
#ifdef LEXEME_DEBUG
                std::cout << details::BOLD << ":: key[" << show(key) << "]" << details::RESET << std::endl;
#endif
                m_in >> std::skipws;

                // parse separator ('=')
                //
                if (c != std::get<assign_key>(m_option)) {
                    m_in >> c;
                    if (c != std::get<assign_key>(m_option)) {
                        std::clog << std::get<target_name>(m_option) << ": parse error: key[" << key << "] missing separator '" 
                        << std::get<assign_key>(m_option) << "' (line "<< details::line_number(m_in) << ")" << std::endl;
                        return make_ret<key_value_pack<Ti...>>(false);
                    }
                }

                m_in >> std::ws;

                // parse value for the current key (or skip it)...
                // 

                if (!tmp.has_key(key) && !std::get<parser_mode>(m_option)) 
                {
                    int level = 0;

                    do {
                        c = static_cast<std::string::traits_type::char_type>(m_in.peek());
                        if (!m_in) {
                            std::clog << std::get<target_name>(m_option) << ": parse error at key '" 
                            << key << "' missing brackets (line "<< details::line_number(m_in) << ")" << std::endl;

                            return make_ret<key_value_pack<Ti...>>(false);
                        }
                        if ( c == '[' || c == '(') {
                            level++;
                        }
                        else if ( c == ']' || c == ')') {
                            if (--level < 0) {
                                std::clog << std::get<target_name>(m_option) << ": parse error at key '" 
                                << key << "' unbalanced brackets (line "<< details::line_number(m_in) << ")" << std::endl;

                                return make_ret<key_value_pack<Ti...>>(false);
                            }
                        }
                    }
                    while (level > 0 && m_in.get());

                    if (c == ']' || c == ')')
                        m_in.get();
                    else
                        m_in >> details::ignore_line;

                    continue;
                }

                // parse the value...
                // 
                if (!tmp.parse(m_in, key, m_option, *this)) {
                    return make_ret<key_value_pack<Ti...>>(false);
                }
            }
            if (bracket) { 
                std::clog << std::get<target_name>(m_option) << ": parse error: missing close bracket (line "<< details::line_number(m_in) << ")" << std::endl;
                return make_ret<key_value_pack<Ti...>>(false);
            }
            
            // parsing correct...

            m_in >> std::skipws;

            elem  = tmp;
            return make_ret<key_value_pack<Ti...>>(true, true);
        }
    };

    template <typename CharT, typename Traits>
    lexer<CharT, Traits> 
    make_lexer(std::basic_istream<CharT, Traits> &in, const parser_options &opt)
    {
        return lexer<CharT, Traits>(in, opt);
    }

    //////////////////////////////////////////////////////////////////////////
    //   options 
    
    namespace key_value_opt {

        struct options
        {
            parser_options m_opt;

            options()
            : m_opt(false,'=', '#', std::string())
            {}

            options &
            strict() 
            { 
                std::get<parser_mode>(m_opt) = true; return *this; 
            }

            options &
            non_strict() 
            { 
                std::get<parser_mode>(m_opt) = false; return *this; 
            }

            options &
            separator(char c) 
            { 
                std::get<assign_key>(m_opt) = c; return *this; 
            }

            options &
            comment(char c) 
            { 
                std::get<comment_key>(m_opt) = c; return *this; 
            }

            operator parser_options()
            {
                return m_opt;
            }
        };

        static inline options
        strict() 
        { 
            return options().strict(); 
        }

        static inline options
        non_strict() 
        { 
            return options().non_strict(); 
        }

        static inline options
        separator(char c) 
        { 
            return options().separator(c); 
        }

        static inline options
        comment(char c) 
        { 
            return options().comment(c); 
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //   key_value_pack

    template <typename ...T> struct key_value_pack;

    template <typename T0, typename ...Ti>
    struct key_value_pack<T0, Ti...>
    {
    public:

        typedef more::type::typemap<typename T0::type, typename Ti::type...>    map_type;
        typedef typename T0::type::first_type                                   key_type;
        typedef typename T0::type::second_type                                  value_type;

        typedef key_value_pack<Ti...>                                           parser_type;

        parser_type  m_parser;
        key_type     m_key;
        value_type   m_value;

        key_value_pack()
        : m_parser()
        , m_key()
        , m_value(details::get_default<key_type, value_type, key_type::has_default>::value()) 
        { }


        key_value_pack(const char *name, 
                         const parser_options &mode = std::make_tuple(false, '=', '#', "pack")) 
        : m_parser()
        , m_key()
        , m_value(details::get_default<key_type, value_type, key_type::has_default>::value()) 
        {
            if(!this->load(name, mode))
                throw std::runtime_error("key_value_pack");
        }

        virtual ~key_value_pack()
        {}

        // get method
        //
        
        template <typename Key>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get() 
        { 
            return get_<Key>(std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); 
        }

        template <typename Key>
        typename std::add_lvalue_reference<
        typename std::add_const<typename more::type::get<map_type, Key>::type>::type>::type
        get() const
        { 
            return const_cast<key_value_pack *>(this)->get_<Key>(std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); 
        }

        template <typename Key, int N>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get_(std::integral_constant<int,N>) 
        { 
            return m_parser.template get_<Key>(std::integral_constant<int, N-1>()); 
        }

        template <typename Key>
        typename std::add_lvalue_reference<value_type>::type
        get_(std::integral_constant<int,0>) 
        { 
            return m_value; 
        }

    public:

        // run-time parser 
        //
        
        template <typename CharT, typename Traits>
        bool parse(std::basic_istream<CharT, Traits> &in, const std::string &key, 
                   const parser_options &mode, lexer<CharT, Traits> &lex)
        { 
            return parse_(in, key, *this, mode, lex); 
        }

        template <typename CharT, typename Traits, typename _T0, typename ..._Ti>
        static bool parse_(std::basic_istream<CharT, Traits> &in, const std::string &key, key_value_pack<_T0, _Ti...> &that, 
                           const parser_options &mode, lexer<CharT, Traits> &lex)
        {
            if (key == _T0::type::first_type::str()) {

                if (!lex.log_ret(lex.parse_lexeme(that.m_value)).first || in.fail()) {

                    std::clog << std::get<target_name>(mode) << ": parse error: key[" << _T0::type::first_type::str() 
                    << "] unexpected argument at line " << details::line_number(in) << std::endl;
                    return false;
                }
                return true;
            }
            return parse_(in, key, that.m_parser, mode, lex);
        }

        template <typename CharT, typename Traits>
        static bool parse_(std::basic_istream<CharT, Traits> &in, const std::string &key, 
                           key_value_pack<> &, const parser_options &mode, lexer<CharT, Traits>&)
        {
            // unknown key-value...
            //
            
            if (std::get<parser_mode>(mode)) {   // strict mode: dump-error 
                std::clog << std::get<target_name>(mode) << ": parse error: key[" << key << "] unknown (line " << 
                details::line_number(in) << ")" << std::endl;
                return false;
            }

            // non-strict mode: skip this line
            in >> details::ignore_line;
            return true;
        }

        // predicate: has_key 
        //

        bool has_key(const std::string &key) 
        {
            return has_key_(key, *this);
        }

        template <typename _T0, typename ..._Ti>
        static bool has_key_(const std::string &key, key_value_pack<_T0, _Ti...> &that)
        {
            if (key == _T0::type::first_type::str()) {
                return true;
            }
            return has_key_(key, that.m_parser);
        }
        static bool has_key_(const std::string &, key_value_pack<> &) 
        {
            return false;
        }

    public:

        bool load(const char *name, parser_options mode = std::make_tuple(false, '=', '#', "")) 
        {
            std::ifstream sc(name);
            std::get<target_name>(mode) = std::string(name);

            if (!sc) {
                std::clog << name << ": parse error: no such file" << std::endl;
                return false;
            }

            details::streambuf sb(sc.rdbuf(), std::get<comment_key>(mode));
            std::istream in(&sb);    
            return load(in, mode);
        }

        template <typename CharT, typename Traits>
        bool load(std::basic_istream<CharT, Traits> &in, parser_options mode = std::make_tuple(false, '=', '#', "unnamed")) 
        {
            auto lex = make_lexer(in, mode);
            return lex.parse_lexeme(*this, false).first;
        }
    };

    template <>
    struct key_value_pack<> {};

#ifdef LEXEME_DEBUG

    template <typename ...Ts>
    inline std::string
    show(const key_value_pack<Ts...> &, const char * = nullptr)
    {
        return details::type_name<key_value_pack<Ts...>>() + "{ ... }";    
    }

#endif
    
    //////////////////////////////////////////////////////////////////////////
    //   get utility functions


    template<typename T, typename ...Ti>
    inline 
    typename std::add_lvalue_reference<
        typename more::type::get<typename key_value_pack<Ti...>::map_type, T>::type>::type 
    get(key_value_pack<Ti...> &p) 
    {                                           
        return p.template get<T>();
    }

    template<typename T, typename ...Ti>
    inline 
    typename std::add_lvalue_reference<
        typename std::add_const<
            typename more::type::get<typename key_value_pack<Ti...>::map_type, T>::type>::type>::type 
    get(const key_value_pack<Ti...> &p) 
    {       
        return p.template get<T>();
    }

    template <typename CharT, typename Traits, typename ...Ti>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, key_value_pack<Ti...> const&)
    {
        return out << "key_value_pack<Ti...>";
    }

} // namespace more

#endif /* _KEYVALUE_HPP_ */
