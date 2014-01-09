/* ----------------------------------------------------------------------------
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <read.hpp>     // more!
#include <show.hpp>     // more!
#include <cxxabi.hpp>   // more!

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <stdexcept>


namespace more {
    
    namespace key_value
    {
        namespace details
        {       
            template <typename T>
            inline std::string
            type_name()
            {
                std::string name = ::type_name<T>();

                auto n = name.find_last_of(":");
                if (n == std::string::npos)
                    return std::move(name);
                else
                    return name.substr(n+1);
            }

            // ensure the given char is read from the stream...
           
            template <typename CharT, typename Traits>
            bool read_char(std::basic_istream<CharT, Traits> & in, typename std::basic_istream<CharT, Traits>::int_type c)
            {
                decltype(c) _c;

                if(!(in >> std::ws)) 
                    return false;
                
                _c = in.peek();
                if (!in) 
                    return false;

                if (c == _c) 
                {
                    in.get();
                    return true;
                }
                else 
                {
                    return false;
                }
            }

            class streambuf : public std::streambuf 
            {
                typedef std::streambuf::pos_type    pos_type;
                typedef std::streambuf::off_type    off_type;

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
 
                virtual pos_type seekoff(off_type off, std::ios_base::seekdir way,
                                         std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
                {
                    return m_in->pubseekoff(off, way, which); 
                }
    
                
                virtual pos_type seekpos(pos_type sp, 
                                         std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
                {
                    return m_in->pubseekpos(sp, which);
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
                int              m_line;
                char             m_commkey;
                state            m_state, m_prev;
            };

            template <class CharT, class Traits>
            inline int line_number(std::basic_istream<CharT,Traits> &in)
            {
                streambuf * ln = dynamic_cast<streambuf *>(in.rdbuf());
                if (ln) 
                    return ln->line();

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

        } /// namespace details

        ///////////////////////////////////////////////////////////////////////////////////////////

        // key_value pair:

        template <typename Tp>
        struct key_base 
        { 
            static std::string
            str()
            {
                return details::type_name<Tp>();
            }
        };

#define DECLARE_KEY(k) struct k : more::key_value::key_base<k> { }

        template <typename Key, typename Value>
        struct pair
        {
            typedef Key     key_type;
            typedef Value   value_type;

            static_assert(std::is_base_of<key_base<Key>, Key>::value, "invalid key type (DECLARE_KEY)");
            Value   value;
        };

        template <typename Key, typename Value>
        inline
        std::string show(pair<Key,Value> const& p)
        {
            std::string ret = details::type_name<Key>();
            ret += ("= " + ::show(p.value));
            return ret;
        }


        // parser options:

        struct options_base { };

        template <bool s, char cm, char sep>
        struct options : options_base 
        {
            static constexpr bool strict    = s;
            static constexpr char comment   = cm;
            static constexpr char separator = sep;
        };


        // metafunctions:
        
        template <typename K, typename ...Ts> struct key_index;
        template <typename K, typename V, typename ...Ts>
        struct key_index<K, pair<K, V>, Ts...>
        {
            enum { value = 0 };
        };
        template <typename K, typename T0, typename ...Ts>
        struct key_index<K, T0, Ts...>
        {
            enum { value = 1 + key_index<K, Ts...>::value};
        };

        template <typename K, typename ...Ts> struct key_mapped_type;
        template <typename K, typename V, typename ...Ts>
        struct key_mapped_type<K, pair<K, V>, Ts...>
        {
            typedef V type;
        };
        template <typename K, typename T0, typename ...Ts>
        struct key_mapped_type<K, T0, Ts...>
        {
            typedef typename key_mapped_type<K, Ts...>::type type;
        };


        // parser:

        template <typename Opt, typename ...Ps>
        struct parser
        {
            static_assert(std::is_base_of<options_base, Opt>::value, "invalid options");

            template <typename K>
            typename key_mapped_type<K, Ps...>::type const &
            get() const
            {
                return std::get< key_index<K, Ps...>::value >(tuple_).value;
            }

            template <typename K>
            typename key_mapped_type<K, Ps...>::type &
            get() 
            {
                return std::get< key_index<K, Ps...>::value >(tuple_).value;
            }

            std::tuple<Ps...>  tuple_;
        };
        
        template <typename Opt, typename ...Ps>
        inline
        std::string show(parser<Opt, Ps...> const& par)
        {
            std::string ret;
            ret += ::show(par.tuple_);
            return ret;
        }


        // read_key:
        
        template <typename CharT, typename Traits, typename Opt, typename ...Ps> struct read_key;
        template <typename CharT, typename Traits, typename Opt, typename P0, typename ...Ps>
        struct read_key<CharT, Traits, Opt, P0, Ps...>
        {
            template <typename Par>
            static bool 
            run(std::string const &key, Par &par, std::basic_istream<CharT, Traits> &in)
            {
                auto name = details::type_name<typename P0::key_type>();
                
                if (key == name)
                {
                    auto & value = par.template get<typename P0::key_type>();
                    value = ::read<typename P0::value_type>(in);
                    return true;
                }
                else
                {
                    return read_key<CharT, Traits, Opt, Ps...>::run(key, par, in);
                }
            }
        };
        template <typename CharT, typename Traits, typename Opt>
        struct read_key<CharT, Traits, Opt>
        {
            template <typename Par>
            static bool
            run(std::string const &key, Par &, std::basic_istream<CharT, Traits> &)
            {
                if (Opt::strict)
                    throw std::runtime_error("unknown key '" + key + "'");

                return false;
            }
        };

        // read for parser type:
        
        template <typename CharT, typename Traits, typename Opt, typename ...Ps>
        inline parser<Opt, Ps...>
        read(read_tag<parser<Opt, Ps...>>, std::basic_istream<CharT,Traits>&in)
        {
            bool bracket = false;

            if (details::read_char(in, '{'))
            {
                bracket = true;
            }
            
            parser<Opt, Ps...> par;

            while(in)
            {
                in >> std::noskipws >> std::ws;
                
                // parse the key...
                
                std::string key; key.reserve(16);   

                std::string::traits_type::char_type c = '\0';

                while ((in >> c) && !isspace(c) && c != Opt::separator) 
                {
                    key.push_back(c);
                }

                if (key.empty())
                    continue;

                if (!key.compare("}"))
                {
                    bracket = false;
                    break;
                }

                in >> std::skipws;

                // parse separator:
                
                if (c != Opt::separator) 
                {
                    in >> c;
                    if (c != Opt::separator)
                        throw std::runtime_error("missing separator");
                }
                
                // parse the value for this key...
                
                if (!read_key<CharT, Traits, Opt, Ps...>::run(key, par, in))
                {
                    int level = 0;
                
                    in >> std::ws;

                    do {
                        c = static_cast<std::string::traits_type::char_type>(in.peek());
                        if (!in) 
                            throw std::runtime_error("missing brackets");

                        if ( c == '[' || c == '(') {
                            level++;
                        }
                        else if ( c == ']' || c == ')') {
                            if (--level < 0) 
                                throw std::runtime_error("unbalanced brackets");
                        }
                    }
                    while (level > 0 && in.get());

                    if (c == ']' || c == ')')
                        in.get();
                    else
                        in >> details::ignore_line;

                }
            }

            if (bracket)
                throw std::runtime_error("parser: unbalanced brackets");

            return par;
        }


        // parse: from string...

        template <typename Opt, typename ... Ps>
        bool parse(std::string content, parser<Opt, Ps...> &par) 
        {
            std::istringstream in(content);
            return parse(in, par, "");
        }


        // parse: from file... (filtering comments)

        template <typename Opt, typename ... Ps>
        bool parse(const char *filename, parser<Opt, Ps...> &par)
        {
            std::ifstream ifs(filename);
            if (!ifs)
            {
                std::clog << filename << ": parse error: no such file" << std::endl;
                return false;
            }

            details::streambuf sb(ifs.rdbuf(), Opt::comment);
            std::istream in(&sb);    
            return parse(in, par, filename);
        }
       

        // parse: from input stream...
        
        template <typename CharT, typename Traits, typename Opt, typename ... Ps>
        bool parse(std::basic_istream<CharT, Traits> &in, parser<Opt, Ps...> &par, const char *filename) 
        {
            try
            {
                par = ::read<parser<Opt, Ps...>>(in);
            }
            catch(std::exception &e)
            {
                std::cerr << filename << ':' << details::line_number(in) << ": parse error (" << e.what() << ")" << std::endl;
                return false;
            }
            return true;
        }

    } // namespace key_value

} // namespace more



