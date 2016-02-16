// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main( int argc, char** argv )
{  

    auto fp = fn_reg::function<int( int argc, char** argv )>("av_scene");// main_phys_viewer main_morph "main_anim_test" main_mrt "av_scene"  main_grass_test  main_asio_test main_patched_lowlevel

    if(fp)
        return fp(argc, argv);
	
    return 0;
}
