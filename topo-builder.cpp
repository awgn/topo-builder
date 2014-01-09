#include <basic_parser.hpp>
#include <builder.hpp>
#include <global.hpp>
#include <network.hpp>

#include <string>
#include <stdexcept>
#include <iostream>

void usage(const char *name)
{
    throw std::runtime_error(std::string("usage: ") + name + " [OPTIONS]\n\n" 
          "VM options:\n"
          "   -c, --config file           Specify config file\n"
          "   -a, --append-netinfo        Pass ip address, mask and gw to guest kernel image\n"
          "   -k, --kernel file           Specify the kernel image (default: Core/boot/vmlinuz)\n" 
          "   -C, --core file             Specify the core file    (default: Core/boot/core.gz)\n" 
          "General:\n"
          "   -h, --help                  Display help message\n" 
          "   -v, --verbose               Verbose mode");
}


using namespace topo;

int
main(int argc, char *argv[])
try
{
    ///////////////////////////////////////////////////////////////////
    // parse command line options

    if (argc < 2)
        usage(argv[0]);
    
    const char *config_file = nullptr;

    topo::parser_type pt = topo::parser_type::basic;

    auto is_opt = [](const char *arg, const char *opt, const char *opt2) 
    {
        return (opt  && strcmp(arg, opt) == 0) ||
               (opt2 && strcmp(arg, opt2) == 0);
    };

    for(int i = 1; i < argc; ++i)
    {
        if (is_opt(argv[i], "-c", "--config")) 
        {
            if (++i == argc)
            {
                throw std::runtime_error("argument missing");
            }

            config_file = argv[i];
            continue;
        }
        
        if (is_opt(argv[i], "-k", "--kernel")) 
        {
            if (++i == argc)
            {
                throw std::runtime_error("argument missing");
            }

            global::instance().kernel = argv[i];
            continue;
        }

        if (is_opt(argv[i], "-C", "--core")) 
        {
            if (++i == argc)
            {
                throw std::runtime_error("argument missing");
            }

            global::instance().core = argv[i];
            continue;
        }

        if (is_opt(argv[i], "-v", "--verbose"))
        {
            global::instance().verbose = true;
            continue;
        }

        if (is_opt(argv[i], "-a", "--append-netinfo"))
        {
            global::instance().append_netinfo = true;
            continue;
        }

        if (is_opt(argv[i], "-h", "--help"))
            usage(argv[0]);

        throw std::runtime_error(argv[i] + std::string(": unknown option"));
    }

    if (!config_file)
    {
        throw std::runtime_error(std::string(argv[0]) + ": config file missing");
    }
   
    switch(pt)
    {
    case topo::parser_type::basic:
        {
            topo::basic::parser::type config;
            
            ///////////////////////////////////////////////////////////////////
            // parse config file...

            if (!parse(config_file, config))
                throw std::runtime_error("parse error in config file!");

            auto & ss = config.get<topo::basic::parser::switches>();

            Switches switches;

            for(auto & s : ss)
            {
                Switch tmp;

                switch(s.which())
                {
                    case 0: 
                    {
                        auto & v = s.get<VirtualSwitch>();
                        tmp = Switch { node_name(v), node_type(v), std::vector<std::string>{} };

                    } break;
                    case 1: 
                    {
                        tmp = s.get<Switch>();

                    } break;
                    default:
                        throw std::runtime_error("internal error");
                }

                switches.push_back(std::move(tmp));
            }

            return topo::builder(std::move(config.get<topo::basic::parser::header>()),
                                 std::move(switches),
                                 std::move(config.get<topo::basic::parser::nodes>()),
                                 std::move(config.get<topo::basic::parser::footer>())
                                 );

        } break;
    default: throw std::runtime_error("internal error");
    };

    return 0;
}
catch(std::exception &e)
{
    std::cerr << e.what() << std::endl;
}

