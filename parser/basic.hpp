#include <key_value.hpp>

#include <vector>
#include <string>

#include <network.hpp>

using namespace more::key_value;

namespace topo {
                                       
    enum class parser_type
    {
        basic
    };

    namespace basic {
    namespace parser {

        // MAP_KEY(Strings, header)
        // MAP_KEY(Strings, footer)
        // MAP_KEY(std::vector<Node>, nodes)
        // MAP_KEY(std::vector<Switch>, switches)
        
        DECLARE_KEY(header);
        DECLARE_KEY(footer);
        DECLARE_KEY(nodes);
        DECLARE_KEY(switches);

        typedef more::key_value::parser
        <
                options<true, '#', '='>,
                pair<header,    Strings>, 
                pair<footer,    Strings>,
                pair<nodes,     std::vector<Node>>,
                pair<switches,  std::vector<Switch>>
                
        > type;


    } // namespace parser
    } // namespace basic

} // namespace topo
