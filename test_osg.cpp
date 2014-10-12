// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

std::function<int( int, char**)> pmain[] = {
                                              main_scene      //0
                                             ,main_select     //1
                                             ,main_shadows    //2
                                             ,main_shadows_2  //3
                                             ,main_hud        //4
                                             ,main_shadows_3  //5
											 ,main_texturedGeometry
											 ,main_TestState
                                           };       

int main( int argc, char** argv )
{
    return pmain[7](argc, argv);
}
