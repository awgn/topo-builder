#pragma once 

#include <network.hpp>

#include <vector>

namespace topo {

    enum class builder_type
    {
        script,
        run
    };

    int builder(topo::builder_type type, std::vector<node> nodes);

    
} // namespace topo
