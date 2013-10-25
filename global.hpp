#include <string>


class global {

    private:

    global()
    : verbose(false)
    , vnc(false)
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
    bool vnc;
};

