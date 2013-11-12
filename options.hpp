#pragma once

#include <generic_type.hpp>

namespace opt {

    GENERIC_TYPE(image, { "image",  1 },
                        { "qcow" ,  1 })

    // specialized show...
    //
    inline 
    std::string show(image_type const &o)
    {
        if(o.ctor == "image")
        {
            return "-o " + o.args.at(0);    
        }
        if(o.ctor == "qcow")
        {
            return "-q " + o.args.at(0);    
        }

        throw std::logic_error("internal error");
    }

    GENERIC_TYPE(term, { "tty",  1 },
                       { "vnc" , 1 })

    // specialized show...
    //
    inline 
    std::string show(term_type const &o)
    {
        if(o.ctor == "tty")
        {
            return "-t " + o.args.at(0);    
        }
        if(o.ctor == "vnc")
        {
            return "-v " + o.args.at(0);    
        }

        throw std::logic_error("internal error");
    }




} // namespace opt
