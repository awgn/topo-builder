#pragma once 

#include <network.hpp>
#include <string>
#include <vector>

namespace topo 
{
    namespace script
    {
        using line = std::string;

        std::vector<line> make_switches(Switches ss);
    }

} // namespace topo
