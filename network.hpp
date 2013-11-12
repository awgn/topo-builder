#pragma once 

#include <string>
#include <tuple>
#include <vector>
#include <map>

#include <netaddress.hpp>
#include <show.hpp>

#include <options.hpp>

namespace topo {

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  port: [ "eth0" 192.168.0.1/24 -> vswitch0 ]
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
    //              [ "eth0" 192.168.0.1/24 -> vswitch0 ]
    //              [ "eth1" 192.168.1.1/24 -> vswitch1 ]
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
        bridge,
        macvtap,
        macvtap2,
        vale
    };

    template <typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits>& out, switch_type const&  that)
    {
        switch(that)
        {
        case switch_type::bridge:
            return out << "bridge";
        case switch_type::macvtap:
            return out << "macvtap";
        case switch_type::macvtap2:
            return out << "macvtap2";
        case switch_type::vale:
            return out << "vale";
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
      
        if (s.compare("bridge") == 0)
        {
            return that = switch_type::bridge, in;
        }
        if (s.compare("macvtap") == 0)
        {
            return that = switch_type::macvtap, in;
        }
        if (s.compare("macvtap2") == 0)
        {
            return that = switch_type::macvtap2, in;
        }
        if (s.compare("vale") == 0)
        {
            return that = switch_type::vale, in;
        }

        in.setstate(std::ios_base::failbit);         
        return in;
    }    
    
    inline std::string
    show(const switch_type &st, const char * n = nullptr)
    {
        std::string s;
        if (n) {
            s += std::string(n) + ' ';
        }
        std::ostringstream ss; ss << st;
        return s + ss.str();
    }

    typedef std::pair<std::string, switch_type> Switch;

    inline std::string
    node_name(Switch const &s)
    {
        return s.first;
    }


    inline switch_type
    node_type(Switch const &s)
    {
        return s.second;
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


    typedef std::map<std::string, SwitchInfo> SwitchMap;

    typedef std::map<std::string, std::vector<int>> TapMap;

    ///////////////////////////////////////////////////////////////////////////
    //
    // Topology 
    //
    
    typedef std::vector<Node> Nodes;

} // namespace topo
