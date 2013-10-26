#include <script.hpp>
#include <global.hpp>

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
                            topo::switch_type t,
                            int base,
                            int n_if)
        {
            std::string opt_type;
            switch(t)
            {
            case topo::switch_type::bridge:   opt_type = more::sprint("-B %1", name); break;
            case topo::switch_type::macvtap:  break;
            case topo::switch_type::macvtap2: opt_type = "-2"; break;
            default: throw std::runtime_error("make_bridge_cmdline: internal error");
            }

            return more::sprint("vnet-setup.sh %1 -z -m %2 -n %3", opt_type, base, n_if);
        }


        line
        make_kvm_setup_cmdline()
        {
            return "setkvm.sh";
        }


        line
        make_startvm_cmdline(std::string image, bool vnc, int tty, std::string vmlinuz, std::string core, std::vector<int> ts)
        {
            std::string tty_opt = vnc ? more::sprint("-v %1", tty) : 
                                        more::sprint("-t %1", tty);

            std::ostringstream tap;

            for(auto t : ts)
            {
                tap << "tap" << t << " " << std::endl;
            }

            return more::sprint("startmv.sh -k -n %1 -I \"%2\" -o %3 -l %4 -c %5 </dev/zero &>log-%1.txt &",
                                    tty_opt,
                                    tap.str(),
                                    image,
                                    vmlinuz,
                                    core);
        }
    }

    /////////// public functions...
    
    std::vector<line> make_bridges(SwitchMap ss)
    {
        std::vector<line> ret;

        int base = 1;
        for(auto & s : ss)
        {
            auto nlink = std::get<1>(s.second);

            ret.push_back( make_bridge_cmdline(node_name(get_switch(s.second)),
                                               node_type(get_switch(s.second)),
                                                         base, nlink) ); 

            base += nlink;
        }

        return ret;
    }

    std::vector<line> make_kvm()
    {
        std::vector<line> ret;

        ret.push_back(make_kvm_setup_cmdline());

        return ret;
    }


    std::vector<line> make_vms(SwitchMap const &ss, Nodes const &ns)
    {
        std::vector<line> ret;

        for(auto & n : ns)
        {

            ret.push_back(make_kvm_setup_cmdline());
        }

        return ret;
    }


    } // namespace script
}
