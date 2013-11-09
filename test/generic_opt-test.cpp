#include <generic_opt.hpp>

namespace opt 
{
    OPTION_KIND(image,  { "type", { "--type", true  } },
                        { "help", { "-h",     false } }  
        )
}

int
main(int argc, char *argv[])
{
    generic::option<opt::image> o;

    std::cin >> o;

    std::cout << o;

    return 0;
}

