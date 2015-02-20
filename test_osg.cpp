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
        { 
            static std::string str_severity[] =
            {
                "ALWAYS",
                "FATAL",
                "WARN",
                "NOTICE",
                "INFO",
                "DEBUG_INFO",
                "DEBUG_FP"
            };
      
            _log << str_severity[severity] << ": " << msg;
        
        }
    protected:
        std::ofstream _log;
};


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
//                                           };       


int main( int argc, char** argv )
{  
    osg::setNotifyLevel( osg::WARN );
    osg::setNotifyHandler( new LogFileHandler("goddamnlog.txt") );

    osg::notify(osg::INFO) << "Start this program \n";
    
    osgDB::Registry::instance()->setOptions(new osgDB::Options("dds_flip dds_dxt1_rgba ")); // dds_flip dds_dxt1_rgba  

    auto fp = fn_reg::function<int( int argc, char** argv )>("main_scene2");

    if(fp)
        return fp(argc, argv);

    return 0;
}
