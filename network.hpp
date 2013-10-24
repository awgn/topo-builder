#pragma once 

#include <string>
#include <tuple>
#include <vector>
#include <map>

#include <netaddress.hpp>

namespace topo {

    //  
    //  port: [ "eth0" 192.168.0.1/24 -> vswitch0 ]
    //
    //
    
    using Port = std::pair<
                            std::tuple<std::string,    // id/name
                                       net::address>   // network_address/mask
                            ,
                            std::string>;              // switch-id


    inline std::string 
    port_name(Port const &p)
    {
        return std::get<0>(p.first);
    }
    
    inline net::address 
    port_address(Port const &p)
    {
        return std::get<1>(p.first);
    }

    inline std::string
    port_link(Port const &p)
    {
        return p.second;
    }

    //
    // node:  ( "vrouter" 
    //          [
    //              [ "eth0" 192.168.0.1/24 -> vswitch0 ]
    //              [ "eth1" 192.168.1.1/24 -> vswitch1 ]
    //          ]
    //        )
    //


    using Node = std::tuple<std::string,        // id/name
                            std::vector<Port>>  // port list
                            ;

    inline std::vector<Port>
    node_ports(Node const &n)
    {
        return std::get<1>(n);
    }

    inline std::string 
    node_name(Node const &n)
    {
        return std::get<0>(n);
    }

    //
    // Switch...
    //

    using Switch = std::pair<std::string, uint32_t>;

    inline std::string
    node_name(Switch const &s)
    {
        return s.first;
    }


    inline int
    node_links(Switch const &s)
    {
        return s.second;
    }

    //
    // Switches... many Switch, organised as a map
    //
    
    using Switches = std::map<std::string, uint32_t>;


    using Topology = std::vector<Node>;

} // namespace topo
