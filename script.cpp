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
        make_bridge_cmdline(std::string const &name,
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
     
        inline std::string
        show_alternate(const net::address &addr)
        {
            std::string s;
            
            char buf [16] = { '\0' };
            char mask[16] = { '\0' };

            inet_ntop(AF_INET, &addr.addr(), buf,  sizeof(buf));
            inet_ntop(AF_INET, &addr.mask(), mask, sizeof(mask)); 

            return s + std::string(buf) + 'm' + std::string(mask);
        }


        line
        make_startvm_cmdline(opt::image_type const &image, 
                             opt::term_type const &term,
                             std::vector<Port> const &ports,
                             std::string const &vmlinuz, 
                             std::string const &core, 
                             std::vector<int> const &ts)
        {
            if (ts.empty())
                throw std::logic_error("make_startvm_cmdline: no taps available");
            
            std::string tap_opt;

            auto t = std::begin(ts);

            do 
            {
                tap_opt += "tap" + std::to_string(*t++);
            }
            while (t != std::end(ts) ? (tap_opt += ' ', true) : false);

            // append extra flags to guest kernel...
            //
            
            std::string append;
            if (global::instance().append_netinfo && !ports.empty())
            {
                append += "-a ifaces=";

                auto it = std::begin(ports);
                auto it_e = std::end(ports);
                
                int n = 0;

                do 
                {
                    append += "eth" + std::to_string(n) + "-" +  show_alternate(it->first); 
                    ++it; ++n;
                }
                while([&]() -> bool 
                {
                    return it != it_e ? (append += ",", true) : false;
                }()); 
            }

            return more::sprint("startmv.sh -k -n %1 -I \"%2\" %3 -l %4 -c %5 %7 </dev/zero &>log-%6.txt &",
                                    term,
                                    tap_opt,
                                    image,
                                    vmlinuz,
                                    core,
                                    term.args[0],
                                    append
                                    );
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
        
    
    std::vector<line> make_vms(Nodes const &ns, TapMap const &tm)
    {
        std::vector<line> ret;
            
        auto & g = global::instance();

        for(auto & n : ns)
        {
            auto t = tm.find(node_name(n));
            if (t == std::end(tm))
                throw std::logic_error("make_vms: internal error");

            g.index++;

            ret.push_back(make_startvm_cmdline(node_image(n), 
                                               node_term(n),
                                               node_ports(n),
                                               g.kernel,
                                               g.core,
                                               t->second));
        }

        return ret;
    }


    } // namespace script
}
