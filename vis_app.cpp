#include "stdafx.h"

int main( int argc, char** argv )
{  

    auto fp = fn_reg::function<BOOST_TYPEOF(main)>("visapp2");

    if(fp)
        return fp(argc, argv);

    return 0;
}