#include <cmdopt.hpp>

namespace opt 
{
    OPTION(image, { "type", { "--type", true  } },
                  { "help", { "-h",     false } }  
        )
}

int
main(int argc, char *argv[])
{
    cmd::option<opt::image> o;

    std::cin >> o;

    std::cout << o;

    return 0;
}

