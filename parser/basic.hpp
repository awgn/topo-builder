#include <key_value.hpp>

#include <vector>
#include <string>

#include <network.hpp>

namespace topo {

namespace basic {

    // declare a vector of switching services:
    //
    MAP_KEY(std::vector<node>, virtual_routers)


    typedef more::key_value_pack<virtual_routers> parser;


} // namespace parser

} // namespace topo
