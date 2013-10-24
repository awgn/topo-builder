#include <builder-script.hpp>

#include <iostream>

#include <show.hpp>
#include <print.hpp>
                                                                

namespace topo
{
    namespace script {
   
    line
    make_vm_cmdline(std::string image)
    {

    }

    line
    make_bridge_cmdline(std::string name,
                        int base,
                        int n_if)
    {
        return more::sprint("vnet-setup.hs -B %1 -z -m %2 -n %3", name, base, n_if);
    }
                        
    
    std::vector<line> make_switches(Switches ss)
    {
        std::vector<line> ret;

        int base = 1;
        for(auto & s : ss)
        {
            auto l = node_links(s);

            ret.push_back( make_bridge_cmdline(node_name(s), base, l) ); 

            base += l;
        }

        return ret;
    }

    } // namespace script
}
