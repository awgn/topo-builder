#pragma once 

#include <string>
#include <tuple>
#include <vector>
#include <map>

#include <netaddress.hpp>
#include <show.hpp>

#include <options.hpp>

namespace topo {


    typedef std::vector<std::string> Strings;


    ///////////////////////////////////////////////////////////////////////////
    //  
    //  port: [ 192.168.0.1/24 -> vswitch0 ]
    //
    //
    
    typedef std::pair< net::address                // network_address/mask
                     , std::string> Port;          // switch-id


    inline net::address 
    port_address(Port const &p)
    {
        return p.first;
    }

    inline std::string
    port_linkname(Port const &p)
    {
        return p.second;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // node:  ( "vrouter" image "opt1.img" tty 2
    //          [
    //              [ 192.168.0.1/24 -> vswitch0 ]
    //              [ 192.168.1.1/24 -> vswitch1 ]
    //          ]
    //          gateway 192.168.0.100
    //        )
    //


    typedef std::tuple<std::string,         // id/name
                       opt::Image,          // image
                       opt::Term,           // term
                       std::vector<Port>,   // interfaces
                       opt::Gateway>        // gateway
                       Node;

    inline std::string 
    node_name(Node const &n)
    {
        return std::get<0>(n);
    }
    
    inline opt::Image
    node_image(Node const &n)
    {
        return std::get<1>(n);
    }
    
    inline opt::Term
    node_term(Node const &n)
    {
        return std::get<2>(n);
    }

    inline std::vector<Port>
    node_ports(Node const &n)
    {
        return std::get<3>(n);
    }

    inline opt::Gateway
    node_gateway(Node const &n)
    {
        return std::get<4>(n);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Switch...
    //

    enum class switch_type
    {
        macvtap,
        openvs,
        bridge,
        vale
    };

    template <typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, switch_type const&  that)
    {
        switch(that)
        {
        case switch_type::macvtap:  return out << "macvtap";
        case switch_type::openvs:   return out << "openvs";
        case switch_type::bridge:   return out << "bridge";
        case switch_type::vale:     return out << "vale";
        }
        return out;
    }


    template <typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, switch_type&  that)
    {
        std::string s;
        if (!(in >> s))
            return in;
      
        if (s.compare("macvtap") == 0)
        {
            return that = switch_type::macvtap, in;
        }
        if (s.compare("openvs") == 0)
        {
            return that = switch_type::openvs, in;
        }
        if (s.compare("bridge") == 0)
        {
            return that = switch_type::bridge, in;
        }
        if (s.compare("vale") == 0)
        {
            return that = switch_type::vale, in;
        }

        in.setstate(std::ios_base::failbit);         
        return in;
    }    
    
    inline std::string
    show(const switch_type &st)
    {
        std::string s;
        std::ostringstream ss; ss << st;
        return s + ss.str();
    }

    typedef std::tuple<std::string, switch_type, std::vector<std::string>> Switch;

    inline std::string
    node_name(Switch const &s)
    {
        return std::get<0>(s);
    }


    inline switch_type
    node_type(Switch const &s)
    {
        return std::get<1>(s);
    }
    
    inline std::vector<std::string>
    node_links(Switch const &s)
    {
        return std::get<2>(s);
    }

    typedef std::vector<Switch> Switches;


    ///////////////////////////////////////////////////////////////////////////
    //
    // SwitchMap

    typedef std::tuple<Switch, 
                       int,                  // num_links
                       int,                  // index
                       int> SwitchInfo;      // avail;

    inline Switch
    get_switch(SwitchInfo const &i)
    {
        return std::get<0>(i);
    }

    inline int
    get_num_links(SwitchInfo const &i)
    {
        return std::get<1>(i);
    }
    
    inline int
    get_index(SwitchInfo const &i)
    {
        return std::get<2>(i);
    }
    
    inline int
    get_avail(SwitchInfo const &i)
    {
        return std::get<3>(i);
    }


    typedef std::vector<std::pair<std::string, SwitchInfo>> SwitchMap;

    typedef std::map<std::string, std::vector<std::pair<switch_type, int>>> TapMap;

    ///////////////////////////////////////////////////////////////////////////
    //
    // Topology 
    //
    
    typedef std::vector<Node> Nodes;

} // namespace topo
