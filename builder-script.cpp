#include <builder-script.hpp>

#include <iostream>

#include <show.hpp>
#include <print.hpp>
                                                                

namespace topo
{
    std::string
    make_bridge_cmd(std::string name,
                    int base,
                    int n_if)
    {
        return more::sprint("vnet-setup.hs -B %1 -z -m %2 -n %3", name, base, n_if);
    }
                        
    
    std::vector<std::string> make_switches(Switches ss)
    {
        std::vector<std::string> ret;

        std::cout << show(ss) << std::endl;
        
        int base = 1;
        for(auto & s : ss)
        {
            auto l = node_links(s);

            ret.push_back( make_bridge_cmd(node_name(s), base, l) ); 

            base += l;
        }

        return ret;
    }

}
