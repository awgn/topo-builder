#include <builder.hpp>
#include <builder-script.hpp>

#include <vector>
#include <string>


namespace topo
{
    Switches
    get_switches(Topology const &ns)
    {
        Switches ret;

        for(auto &node : ns)
        {
            for(auto &port : node_ports(node))
            {
                ret[port_link(port)]++;
            }
        }

        return ret;
    }


    int builder(Topology ns)
    {
        auto s = get_switches(ns);

        auto out = script::make_switches(s);

        std::cout << show (out) << std::endl; 

        return 0;
    }


} // namespace topo
