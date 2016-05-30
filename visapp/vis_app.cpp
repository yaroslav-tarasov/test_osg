#include "stdafx.h"

int main( int argc, char** argv )
{  

    if(auto fp = fn_reg::function<BOOST_TYPEOF(main)>("visapp2"))
        return fp(argc, argv);

    return 0;
}