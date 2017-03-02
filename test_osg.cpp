// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common/cmd_line.h"

int main( int argc, char** argv )
{  
    cmd_line::arg_map am;
    if (!am.parse(cmd_line::naive_parser().add_arg("name", true), argc, argv))
    {
#if 0
        LogError("Invalid command line");
        return 1;
#endif
    }

    optional<std::string> name;
    if (am.contains("name")) 
        name = am.extract<std::string>("name");


    auto fp = fn_reg::function<int( int argc, char** argv )>("av_scene");// main_phys_viewer main_morph "main_anim_test" main_mrt "av_scene"  main_grass_test  main_asio_test main_patched_lowlevel

    if(name) 
        fp = fn_reg::function<int( int argc, char** argv )>(*name);
    
    if(fp)
        return fp(argc, argv);
	
    return 0;
}
