#include <variant.hpp>      // more!
#include <key_value.hpp>    // more!

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

        DECLARE_KEY(header);
        DECLARE_KEY(footer);
        DECLARE_KEY(nodes);
        DECLARE_KEY(switches);

        typedef more::key_value::document
        <
                options<true, '#', '='>,
                pair<header,    Strings>, 
                pair<footer,    Strings>,
                pair<nodes,     std::vector<Node>>,
                pair<switches,  std::vector<more::variant<VirtualSwitch, Switch>>>
                
        > type;


    } // namespace parser
    } // namespace basic

} // namespace topo
