#include <key_value.hpp>

#include <vector>
#include <string>

#include <network.hpp>

namespace topo {
                                       
    enum class parser_type
    {
        basic
    };

    namespace basic {
    namespace parser {

        // declare a vector of string:
        //
        
        MAP_KEY(Strings, header)


        MAP_KEY(Strings, footer)

        // declare a vector of nodes:
        //
        
        MAP_KEY(std::vector<Node>, nodes)

        // declare a vector of switching services:
        //

        MAP_KEY(std::vector<Switch>, switches)
        

        typedef more::key_value_pack<header, nodes, switches, footer> type;


    } // namespace parser
    } // namespace basic

} // namespace topo
