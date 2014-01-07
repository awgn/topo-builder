#include <script.hpp>
#include <global.hpp>

#include <iostream>
#include <sstream>
#include <string>

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
                            std::vector<std::string> const &links,
                            int base,
                            int n_if)
        {
            std::string opt_type, opt_links;

            switch(t)
            {
                case topo::switch_type::bridge:  opt_type = more::sprint("-B %1",    name); break;
                case topo::switch_type::macvtap: opt_type = more::sprint("-V %1",    name); break;
                case topo::switch_type::openvs:  opt_type = more::sprint("-O %1",    name); break;
                default: throw std::runtime_error("make_bridge_cmdline: internal error");
            }

            /* link physical ethernet */

            if (!links.empty())
            {
                opt_links += "-L ";
                for(auto & l : links)
                {
                    opt_links += l; 

                    if (&l != &links.back()) 
                        opt_links += ','; 
                }
            }

            return more::sprint("vnet-setup.sh %1 -z -m %2 -n %3 %4", opt_type, base, n_if, opt_links);
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
        make_startvm_cmdline(opt::Image         const &image, 
                             opt::Term          const &term,
                             std::vector<Port>  const &ports,
                             opt::Gateway       const &gateway,
                             std::string        const &vmlinuz, 
                             std::string        const &core, 
                             std::vector<std::pair<switch_type, int>> const &tmap,
                             int id)
        {
            if (tmap.empty())
                throw std::logic_error("make_startvm_cmdline: no taps available");
            
            std::string tap_opt, macvtap_opt;

            for(auto & elem : tmap)
            {
                switch(elem.first)
                {
                    case switch_type::bridge :  tap_opt    += "tap"     + std::to_string(elem.second) + ' '; break;
                    case switch_type::openvs :  tap_opt    += "tap"     + std::to_string(elem.second) + ' '; break;
                    case switch_type::macvtap: macvtap_opt += "macvtap" + std::to_string(elem.second) + ' '; break;
                }
            }

            if (!tap_opt.empty())
                tap_opt = "-I \"" + std::string(tap_opt, 0, tap_opt.size()-1) + "\""; 

            if (!macvtap_opt.empty())
                macvtap_opt = "-i \"" + std::string(macvtap_opt, 0, macvtap_opt.length()-1) + "\""; 

            // append extra flags to guest kernel...
            //
            
            std::string append_opt;

            if (global::instance().append_netinfo && !ports.empty())
            {
                append_opt += "-a \"ifaces=";

                auto it = std::begin(ports);
                auto it_e = std::end(ports);
                
                int n = 0;

                do 
                {
                    append_opt += "eth" + std::to_string(n) + "-" +  show_alternate(it->first); 
                    ++it; ++n;
                }
                while([&]() -> bool 
                {
                    return it != it_e ? (append_opt += ",", true) : false;
                }()); 

                // eventually add the default gw if specified...
                //
                
                if (gateway.type() == opt::Gateway::defaultgw) 
                {
                    auto a = std::get<0>(gateway.data_as<net::address>());
                    append_opt += " gw=" + show(a.addr_ip()) + " norute";
                }
                
                append_opt += "\"";
            }

            auto image_opt = opt::show(image);
            auto term_opt  = opt::show(term);

            return more::sprint("startvm.sh -k -n %1 %2 %3 %4 -l %5 -c %6 %8 </dev/zero &>log-%7.txt &",
                                    term_opt,
                                    macvtap_opt,
                                    tap_opt,
                                    image_opt,
                                    vmlinuz,
                                    core,
                                    id,
                                    append_opt
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

            if (nlink > 0)
            {
                auto const &sw = get_switch(s.second);

                ret.push_back( make_bridge_cmdline(node_name(sw),
                                                   node_type(sw),
                                                   node_links(sw),
                                                   base, nlink) ); 
                base += nlink;
            }

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
                                               node_gateway(n),
                                               g.kernel,
                                               g.core,
                                               t->second,
                                               g.index));
        }

        return ret;
    }


    } // namespace script
}
