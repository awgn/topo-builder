#include <builder.hpp>
#include <script.hpp>
#include <global.hpp>

#include <show.hpp>

#include <vector>
#include <string>

namespace topo
{
    // given the list of Switch and Node, build the switch map
    //
    
    SwitchMap
    make_switch_map(Switches const &ss, Nodes const &ns)
    {
        SwitchMap ret;

        for(auto &s : ss)
        {
#ifdef USE_VECTOR_SWITCH
            ret.push_back(std::make_pair(s.first,std::make_tuple(s, 0, 0, 0)));
#else
            ret[s.first] = std::make_tuple(s, 0, 0, 0);
#endif
        }

        // compute the per-switch number of links...
        //

        for(auto &node : ns)
        {
            for(auto &port : node_ports(node))
            {
#ifdef USE_VECTOR_SWITCH
                auto it = std::find_if(std::begin(ret), 
                                       std::end(ret),
                                       [&](std::pair<std::string, SwitchInfo> const &elem) {
                                            return elem.first == port_linkname(port);
                                       });
#else
                auto it = ret.find(port_linkname(port));
#endif
                if (it == std::end(ret))
                    throw std::runtime_error("make_switch_map: " + port_linkname(port) + " not found");

                std::get<1>(it->second)++;
            }
        }
        
        // compute the per-switch tap ids...
        //

        int base = 1;

        for(auto &p : ret)
        {
            std::get<2>(p.second) = base;
            std::get<3>(p.second) = get_num_links(p.second);
            base += get_num_links(p.second);
        }

        return ret;
    }


    //
    // get tap index from SwitchInfo
    //

    int
    get_first_tap_avail(SwitchMap &sm, std::string const &name)
    {
#ifdef USE_VECTOR_SWITCH
        auto it = std::find_if(std::begin(sm), 
                               std::end(sm),
                               [&](std::pair<std::string, SwitchInfo> const &elem) {
                                    return elem.first == name;
                               });
#else
        auto it = sm.find(name);
#endif
        if (it == std::end(sm))
            throw std::runtime_error("get_tap_index: switch " + name + " not found");

        auto & info = it->second;

        if (get_avail(info) == 0)
            throw std::runtime_error("get_tap_index: internal error");

        std::get<3>(info)--;
        return std::get<2>(info)++;
    }

    //
    // main builder function...
    //

    int builder(Strings header, Switches ss, Nodes ns, Strings footer)
    {
        auto sm = make_switch_map(ss, ns);

        if (global::instance().verbose)
        {               
            std::cerr << "switches   : " << ::show (ss) << std::endl;
            std::cerr << "nodes      : " << ::show (ns) << std::endl;
        }
        
        TapMap tm;

        for(auto &n : ns)
        {
            std::vector<int> taps;

            for(auto p : node_ports(n))
            {
                auto tap = get_first_tap_avail(sm, port_linkname(p));

                taps.push_back(tap);
            }

            tm[node_name(n)] = std::move(taps);
        }

        // display maps...
        //
        
        if (global::instance().verbose)
        {
            std::cerr << "switch_map : " <<  ::show(sm) << std::endl;
            std::cerr << "tap_map    : " <<  ::show(tm) << std::endl;
        }
                 
        //////////////////////////////////////////////////////////////////
        //
        // dump script...
        //
        
        if (!header.empty())
        {
            std::cout << "# header..." << std::endl;
            for(auto &l : header)
            {
                std::cout << l << std::endl;
            }
        }

        auto br = script::make_bridges(sm);

        std::cout << "\n# make bridges..." << std::endl; script::show(br);

        // dump kvm setup...
        //
        
        auto kvm = script::make_kvm();

        std::cout << "\n# setup kvm" << std::endl; script::show(kvm);

        // dump VMs...
        //
        
        auto vms = script::make_vms(ns, tm);

        std::cout << "\n# start VMs..." << std::endl; script::show(vms);

        if (!footer.empty())
        {
            std::cout << "# footer..." << std::endl;
            for(auto &l : footer)
            {
                std::cout << l << std::endl;
            }
        }

        return 0;
    }


} // namespace topo
