#pragma once 

#define STRINGIFY(x) #x 

#define STR(x) STRINGIFY(x)
#define FIXME(x) __pragma(message(__FILE__ "(" STR(__LINE__) "): " "fixme: " STRINGIFY(x) ));


#define Assert(x) if(x){};
#define avAssert(x) assert(x)
#define avError(message, ...) {             \
    char buff [1024];						\
    sprintf(buff, message, __VA_ARGS__);    \
                                            \
    OutputDebugStringA(buff);               \
    assert(false);                          \
}


class logger:
    public boost::noncopyable
{
public:

    static bool need_to_log(boost::optional<bool> ntl = boost::none)
    {
        static bool blog(false);
        if(ntl)
            blog=*ntl;
        return blog;
    };
private:
    logger() {}
};

struct force_log
{
    force_log() {logger::need_to_log(true);}
    ~force_log(){logger::need_to_log(false);}
};

#define LOG_ODS_MSG( msg )                                                                \
    do {                                                                                  \
    if(logger::need_to_log()) {                                                     \
    std::stringstream logger__str;                                                        \
    logger__str << std::setprecision(8) << msg ;                                          \
    OutputDebugStringA(logger__str.str().c_str());                                         \
    }                                                                                     \
    } while(0)