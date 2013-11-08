#include <string>


class global {

    private:

    global()
    : verbose(false)
    , append_ip(false)
    , index(2)
    , kernel("Core/boot/vmlinuz")
    , core("Core/boot/core.gz")
    {}

    ~global()
    {}

    public:

    global(global const&  other) = delete;

    global& 
    operator=(global const&  other) = delete;

    static global& 
    instance()
    {
        static global one;
        return one;
    }

    bool verbose; 
    bool append_ip;
    int index;

    std::string kernel;
    std::string core;
};

