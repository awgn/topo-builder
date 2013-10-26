#pragma once 

#include <network.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace topo 
{
    namespace script
    {
        using line = std::string;

        std::vector<line> make_bridges(SwitchMap ss);

        std::vector<line> make_kvm();

    	std::vector<line> make_vms(Nodes const &ns, TapMap const &tm);

        inline void show(std::vector<std::string> const &script)
        {
            for(auto & line : script)
            {
                std::cout << "sh " << line << std::endl;
            }
        }
    }

} // namespace topo
