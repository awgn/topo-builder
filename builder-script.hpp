#pragma once 

#include <network.hpp>
#include <string>
#include <vector>

namespace topo 
{
    namespace script
    {
        using line = std::string;

        std::vector<line> make_bridges(Switches ss);

        std::vector<line> make_kvm();

        std::vector<line> make_vms();
    }

} // namespace topo
