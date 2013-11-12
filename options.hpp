#pragma once

#include <generic_type.hpp>

namespace opt {

    GENERIC_TYPE(Image, ( image,  std::string),
                        ( qcow ,  std::string)  )

    // specialized show...
    //
    
    inline 
    std::string show(Image const &o)
    {
        switch(o.type())
        {
        case Image::image:
            return "-o " + std::get<0>(o.arg_as<std::string>());

        case Image::qcow:
            return "-q " + std::get<0>(o.arg_as<std::string>());
                          
        case Image::unknown:
            throw std::runtime_error("show: internal error");
        }

        return "";
    }

    GENERIC_TYPE(Term, ( tty,  int ),
                       ( vnc , int ))

    // specialized show...
    //
    inline 
    std::string show(Term const &o)
    {
        switch(o.type())
        {
        case Term::tty:
            return "-t " + std::to_string(std::get<0>(o.arg_as<int>()));

        case Term::vnc:
            return "-v " + std::to_string(std::get<0>(o.arg_as<int>()));

        case Image::unknown:
            throw std::runtime_error("show: internal error");
        }
        
        return "";
    }


    GENERIC_TYPE(Gateway, ( nodefaultgw ) ,
                          ( defaultgw, std::string) )


} // namespace opt
