// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

class LogFileHandler : public osg::NotifyHandler
{
    public:
        LogFileHandler( const std::string& file )
        { _log.open( file.c_str() ); }
        virtual ~LogFileHandler() { _log.close(); }
        virtual void notify(osg::NotifySeverity severity,
            const char* msg)
        { _log << msg; }
    protected:
        std::ofstream _log;
};

std::function<int( int, char**)> pmain[] = {
                                              main_scene            //0
                                             ,main_select           //1
                                             ,main_shadows          //2
                                             ,main_shadows_2        //3
                                             ,main_hud              //4
                                             ,main_shadows_3        //5
											 ,main_texturedGeometry //6
											 ,main_TestState        //7
                                             ,main_tess_test        //8 
                                           };       

int main( int argc, char** argv )
{  
    osg::setNotifyLevel( osg::INFO );
    osg::setNotifyHandler( new LogFileHandler("goddamnlog.txt") );

    osg::notify(osg::INFO) << "Start this program \n";
    return pmain[8](argc, argv);
}
