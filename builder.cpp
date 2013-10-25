#include <builder.hpp>
#include <builder-script.hpp>
#include <global.hpp>

#include <show.hpp>

#include <vector>
#include <string>

namespace topo
{
    // given the list of Switch and Node, build the switch map
    //
    
    SwitchMap
    make_switch_map(Switches const &ss, Nodes const &ns)
    {
        SwitchMap ret;

        for(auto &s : ss)
        {
            ret[s.first] = std::make_tuple(s, 0, 0);
        }

        // compute the per-switch number of links...
        //

        for(auto &node : ns)
        {
            for(auto &port : node_ports(node))
            {
                auto it = ret.find(port_linkname(port));
                if (it == std::end(ret))
                    throw std::runtime_error("make_switch_map: " + port_linkname(port) + " not found");

                std::get<1>(it->second)++;
            }
        }
        
        // compute the per-switch tap ids...
        //

        int base = 1;

        for(auto &p : ret)
        {
            std::get<2>(p.second) = base;
            base += std::get<1>(p.second);
        }

        return ret;
    }

    //
    // main builder function...
    //


    int builder(Switches ss, Nodes ns)
    {
        auto sm = make_switch_map(ss, ns);

        auto br = script::make_bridges(sm);

        
        if (global::instance().verbose)
        {               
            std::cerr << "switch map: " << ::show (sm) << std::endl;

            std::cerr << "brige setup: " << ::show (br) << std::endl; 
        }

        
        return 0;
    }


} // namespace topo
