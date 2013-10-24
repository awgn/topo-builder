#pragma once 

#include <string>
#include <tuple>
#include <vector>

#include <netaddress.hpp>

namespace topo {

    //  port: ( "eth0" 192.168.0.1/24 -> vswitch0 )
    //

    using port = std::pair<
                            std::tuple<std::string,    // id/name
                                       net::address>   // network_address/mask
                            ,
                            std::string>;              // switch-id


    inline std::string 
    port_id(port const &p)
    {
        return std::get<0>(p.first);
    }
    
    inline net::address 
    port_address(port const &p)
    {
        return std::get<1>(p.first);
    }


    inline std::string
    port_link(port const &p)
    {
        return p.second;
    }


    // node:  ( "vrouter" 
    //          [
    //              ( "eth0" 192.168.0.1/24 -> vswitch0 )
    //              ( "eth1" 192.168.1.1/24 -> vswitch1 )
    //          ]
    //        )
    //

    using node = std::tuple<std::string,        // id/name
                            std::vector<port>>  // port list
                            ;


} // namespace topo
