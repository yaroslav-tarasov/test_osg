// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


//std::function<int( int, char**)> pmain[] = {
//                                              main_scene            //0
//                                             ,main_select           //1
//                                             ,main_shadows          //2
//                                             ,main_shadows_2        //3
//                                             ,main_shadows_3        //4
//                                             ,main_hud              //5
//											   ,main_texturedGeometry //6
//											   ,main_TestState        //7
//                                             ,main_tess_test        //8 
//                                             ,main_tex_test         //9
//                                             ,main_bump_map         //10
//                                             ,main_exp_test         //11
//                                             ,main_bi               //12
//                                             ,main_teapot           //13
//                                             ,main_spark            //14
//                                             ,main_scene2           //15
//											   ,main_dubins           //16
//                                             ,main_net              //17
//                                              main_rp
//                                           };    



int main( int argc, char** argv )
{  

    auto fp = fn_reg::function<int( int argc, char** argv )>("av_scene");//main_morph "main_anim_test" main_mrt "av_scene"  main_grass_test  main_asio_test main_patched_lowlevel

    if(fp)
        return fp(argc, argv);
	
    return 0;
}
