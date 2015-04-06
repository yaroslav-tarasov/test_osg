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

struct krv_data
{
	float x;
	float y; 
	float h;
	float fi;
	float fiw;
	float kr;
	float v;
    float w; 
	float vb;
	float tg;
	float time;
};

std::ostream &operator <<(std::ostream &os, const krv_data &kp) {
	using namespace std;

	for(size_t i = 0 ; i < sizeof(kp)/sizeof(float); ++i)
		os << *((float*)(&kp) + i*sizeof(float))  << "  ";
	return os;
}

struct value_getter
{
	value_getter(std::string const& line)
	{
		boost::split(values_, line, boost::is_any_of(" \t="), boost::token_compress_on);
	}

	template <class T>
	T get(size_t index)
	{
		return boost::lexical_cast<T>(values_[index]);
	}

private:
	std::vector<std::string> values_;
};

int main( int argc, char** argv )
{  
    osg::setNotifyLevel( osg::WARN );
    osg::setNotifyHandler( new LogFileHandler("goddamnlog.txt") );
    
	
	krv_data kd;

	{

		std::ifstream ifs("log_AFL319.txt");
		
		int num =0;
		while (ifs.good())
		{
			char buf[0x400] = {};
			ifs.getline(buf, 0x400);

			std::string line = buf;
			value_getter items(line);

			std::cout << line;
		} 


	}

	osg::notify(osg::INFO) << "Start this program \n";
    
    osgDB::Registry::instance()->setOptions(new osgDB::Options("dds_flip dds_dxt1_rgba ")); // dds_flip dds_dxt1_rgba  

    auto fp = fn_reg::function<int( int argc, char** argv )>("av_scene");

    if(fp)
        return fp(argc, argv);

	BOOST_TYPEOF(fp) func;
    return 0;
}
