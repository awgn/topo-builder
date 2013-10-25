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
                            net::address                // network_address/mask
                            ,
                            std::string>;              // switch-id


    inline net::address 
    port_address(Port const &p)
    {
        return p.first;
    }

    inline std::string
    port_link(Port const &p)
    {
        return p.second;
    }

    //
    // node:  ( "vrouter" "opt1.img"
    //          [
    //              [ "eth0" 192.168.0.1/24 -> vswitch0 ]
    //              [ "eth1" 192.168.1.1/24 -> vswitch1 ]
    //          ]
    //        )
    //


    using Node = std::tuple<std::string,        // id/name
                            std::string,        // image
                            std::vector<Port>>  // port list
                            ;

    inline std::string 
    node_name(Node const &n)
    {
        return std::get<0>(n);
    }
    
    inline std::string 
    node_image(Node const &n)
    {
        return std::get<1>(n);
    }

    inline std::vector<Port>
    node_ports(Node const &n)
    {
        return std::get<2>(n);
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
    // Switches... many Switch, organised in a map
    //
    
    using Switches = std::map<std::string, uint32_t>;


    using Topology = std::vector<Node>;

} // namespace topo
