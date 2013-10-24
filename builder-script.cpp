#include <builder-script.hpp>

#include <iostream>
#include <sstream>

#include <show.hpp>
#include <print.hpp>
                                                                

namespace topo
{
    namespace script {
   
    namespace
    {
        line
        make_bridge_cmdline(std::string name,
                            int base,
                            int n_if)
        {
            return more::sprint("vnet-setup.hs -B %1 -z -m %2 -n %3", name, base, n_if);
        }
        
        line
        make_kvm_setup_cmdline()
        {
            return "setkvm.sh";
        }
        
        line
        make_startvm_on_tty_cmdline(std::string image, int tty, std::string vmlinuz, std::string core, std::vector<int> ts)
        {
            std::ostringstream tap;

            for(auto t : ts)
            {
                tap << "tap" << t << " " << std::endl;
            }

            return more::sprint("startmv.sh -k -n -t %1 -I \"%2\" -o %3 -l %4 -c %5 </dev/zero &>log-%1.txt &",
                                    tty,
                                    tap.str(),
                                    image,
                                    vmlinuz,
                                    core);
        }

        line
        make_startvm_on_vnc_cmdline(std::string image, int display, std::string vmlinuz, std::string core, std::vector<int> ts)
        {
            std::ostringstream tap;

            for(auto t : ts)
            {
                tap << "tap" << t << " " << std::endl;
            }

            return more::sprint("startmv.sh -k -n -v %1 -I \"%2\" -o %3 -l %4 -c %5 </dev/zero &>log-%1.txt &",
                                    display,
                                    tap.str(),
                                    image,
                                    vmlinuz,
                                    core);
        }

    }

    /////////// public functions...
    
    std::vector<line> make_bridges(Switches ss)
    {
        std::vector<line> ret;

        int base = 1;
        for(auto & s : ss)
        {
            auto l = node_links(s);

            ret.push_back( make_bridge_cmdline(node_name(s), base, l) ); 

            base += l;
        }

        return ret;
    }

    std::vector<line> make_kvm()
    {
        std::vector<line> ret;

        ret.push_back(make_kvm_setup_cmdline());

        return ret;
    }


    std::vector<line> make_vms(Topology const &ns)
    {
        std::vector<line> ret;

        ret.push_back(make_kvm_setup_cmdline());

        return ret;
    }


    } // namespace script
}
