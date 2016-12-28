// animextr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>
#include <osg/TextureRectangle>

#include "av/avUtils/visitors/find_node_visitor.h"
#include "av/avUtils/empty_scene.h"
#include "av/avUtils/animutils.h"
#include "av/avUtils/osg_helpers.h"

#include "av/avCore/InstancedData.h"

#include "core_anim.h"


namespace {

    //
    //  TODO model name and animation param ---->  args 
    //

const float frames_on_take =  150.0;
const std::string model_path = "crow";

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


inline osg::Node* loadAnimation( const std::string& path , const std::string& aname)
{
    auto anim = osgDB::readNodeFile( path + "/" + aname + ".fbx");
    anim->setName(aname);
    return  anim;
}

}

int _tmain(int argc, _TCHAR* argv[])
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);
    viewer.apply(new osgViewer::SingleScreen(0));
    viewer.setUpViewInWindow(0, 0, 5, 5);
    
    initDataPaths();

    osg::ref_ptr<osg::Group> root = new osg::Group;

    //
    //  TODO Anims ---->  args
    //

    auto anim_first   = loadAnimation(model_path, "idle");
    auto anim_second  = loadAnimation(model_path, "flap");
    auto anim_third   = loadAnimation(model_path, "soar");


    auto object_file = osgDB::readNodeFile( model_path + "/flap.fbx");

    avAnimation::InstancedAnimationManager im(anim_first);   

    im.setupRigGeometry (object_file, true);

    //
    //  TODO PAT ---->  args or file
    //

    auto pat = new osg::PositionAttitudeTransform; 
    pat->addChild(object_file);
    pat->setAttitude(
        osg::Quat(osg::inDegrees(0.0)   ,osg::X_AXIS,
        osg::inDegrees(0.0)   ,osg::Y_AXIS,
        osg::inDegrees(0.0)   ,osg::Z_AXIS)
        );

    //
    //  TODO Scale ---->  args or file
    //
    pat->asTransform()->asPositionAttitudeTransform()->setScale(osg::Vec3(0.5,0.5,0.5));

    osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;
    ph_ctrl->setName("phys_ctrl");
    ph_ctrl->setUserValue("id",0);
    ph_ctrl->addChild( pat );
    root->addChild(ph_ctrl);

    using namespace avAnimation;

    AnimationManagerFinder finder;
    AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();

    anim_first->accept(finder);
    if (finder.getBM()) {
        pat->addUpdateCallback(finder.getBM());
        AnimtkViewerModelController::setModel(finder.getBM());
        AnimtkViewerModelController::addAnimation(anim_second); 
        AnimtkViewerModelController::addAnimation(anim_third); 

        // We're safe at this point, so begin processing.
        mc.setPlayMode(osgAnimation::Animation::ONCE);
        mc.next();
        mc.play();

        viewer.setSceneData(root);

        const AnimtkViewerModelController::AnimationDurationMap& ad_map = mc.getDurations();

        for (auto it = ad_map.begin(); it!=ad_map.end(); ++it)
        {
            const std::string& cn       = mc.getCurrentAnimationName();

            for (double t=0.0;t<ad_map.at(cn);t+= ad_map.at(cn)/frames_on_take )
            {
                viewer.frame(t);
            }

            mc.stop();
            mc.next();
            mc.play();
        }


        osg::ref_ptr<osg::TextureRectangle> tex = im.createAnimationData();

        std::string filename =  model_path + "/data.row";
        {
            std::ofstream image_data_file(filename, std::ios_base::binary);
            auto bts = binary::wrap(im.getImageData());
            image_data_file.write(binary::raw_ptr(bts),binary::size(bts));
        }


    } else {
        osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
    }

	return 0;
}

