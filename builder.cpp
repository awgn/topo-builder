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


    int builder(builder_type t, Topology ns)
    {
        switch(t)
        {
        case builder_type::script:
            return builder_script(std::move(ns));

        case builder_type::run:
            return builder_run(std::move(ns));

        default:
            throw std::runtime_error("builder: internal error");
        }
    }

    int builder_script(Topology ns)
    {
        auto s = get_switches(ns);

        auto out = make_switches(s);

        std::cout << show (out) << std::endl; 

        return 0;
    }

    int builder_run(Topology ns)
    {
        return 0;
    }

} // namespace topo
