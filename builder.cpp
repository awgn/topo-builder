#include <builder.hpp>
#include <show.hpp>

#include <vector>
#include <string>


namespace topo
{
    int builder(builder_type t, std::vector<node> nodes)
    {
        std::cout << show (nodes) << std::endl;
        return 0;
    }

} // namespace topo
