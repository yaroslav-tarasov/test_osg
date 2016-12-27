#include "stdafx.h"
#include "av/precompiled.h"
#include <osg/ProxyNode>

namespace {

    void initDataPaths()
    {
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas\\misc");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");  
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\misc");
        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\images");   

        osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models\\human");
    }

    class MyReadFileCallback : public osgDB::Registry::ReadFileCallback
    {
    public:
        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
        {
            std::cout<<"before readNode"<<std::endl;
            // note when calling the Registry to do the read you have to call readNodeImplementation NOT readNode, as this will
            // cause on infinite recusive loop.
            osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(fileName,options);
            std::cout<<"after readNode"<<std::endl;
            return result;
        }
    };
}

int main_async_lt( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    //osg::setNotifyLevel( osg::INFO ); 
    
    initDataPaths();
    
    osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = new osgDB::ReaderWriter::Options;
    local_opt->setReadFileCallback(new MyReadFileCallback);


    osg::ref_ptr<osg::Group> root= new osg::Group;
    //auto object_file = osgDB::readNodeFile("ka_50/ka_50.dae", local_opt);


    {
        osg::ProxyNode* pn = new osg::ProxyNode();
        pn->setFileName(0,"ka_50/ka_50.dae");
        pn->setDatabaseOptions(local_opt);

        root->addChild(pn);
    }
    //root->addChild(object_file);  

    {
        osg::ProxyNode* pn = new osg::ProxyNode();
        pn->setFileName(0,"ka_50/ka_50.dae");
        pn->setDatabaseOptions(local_opt);
        auto mt = new osg::PositionAttitudeTransform;
        mt->setAttitude(osg::Quat (osg::inDegrees(90.f)  , osg::Z_AXIS ));
        mt->addChild(pn);
        root->addChild(mt);
    }

    osgViewer::Viewer viewer(arguments);

    viewer.apply(new osgViewer::SingleScreen(1));

    viewer.setSceneData( root.get() );
    return viewer.run();
}

AUTO_REG(main_async_lt)