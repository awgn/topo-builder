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

        // declare a vector of switching services:
        //
        MAP_KEY(std::vector<node>, nodes)

        typedef more::key_value_pack<nodes> type;


    } // namespace parser

} // namespace basic

} // namespace topo
