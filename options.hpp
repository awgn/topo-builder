#pragma once

#include <generic_opt.hpp>

namespace opt {

    // option<image>:
    //
    // image opt1.img or
    // qcow  opt1.img
    //
    
    OPTION_KIND(image, { "image", { "-o", 1 } },
                       { "qcow" , { "-q", 1 } }
           )


    OPTION_KIND(term, { "tty",    { "-t", 1 } },
                      { "vnc" ,   { "-v", 1 } }
           )


} // namespace opt
