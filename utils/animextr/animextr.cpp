// animextr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "empty_scene.h"
#include "animutils.h"

#include "osg_helpers.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>
#include <osg/TextureRectangle>

#include "utils/visitors/find_node_visitor.h"

#include "av/avCore/InstancedData.h"
#include "core_anim.h"


inline osg::Node* loadAnimation(std::string aname)
{
    auto anim = osgDB::readNodeFile("crow/" + aname + ".fbx");
    anim->setName(aname);
    return  anim;
}

int _tmain(int argc, _TCHAR* argv[])
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);
    //arguments.reportRemainingOptionsAsUnrecognized();
    viewer.apply(new osgViewer::SingleScreen(0));
    viewer.setUpViewInWindow(0, 0, 5, 5);


    osg::ref_ptr<osg::Group> root = new osg::Group;

    auto anim_file = osgDB::readNodeFile("crow/idle.fbx")  ;
    auto anim_idle    = loadAnimation("flap");
    auto anim_running = loadAnimation("soar");


    auto object_file = osgDB::readNodeFile("crow/flap.fbx");

    avAnimation::InstancedAnimationManager im(anim_file);   

    im.setupRigGeometry (object_file, true);

    auto pat = new osg::PositionAttitudeTransform; 
    pat->addChild(object_file);
    pat->setAttitude(
        osg::Quat(osg::inDegrees(0.0)   ,osg::X_AXIS,
        osg::inDegrees(0.0)   ,osg::Y_AXIS,
        osg::inDegrees(0.0)   ,osg::Z_AXIS)
        );

    pat->asTransform()->asPositionAttitudeTransform()->setScale(osg::Vec3(0.5,0.5,0.5));

    osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;
    ph_ctrl->setName("phys_ctrl");
    ph_ctrl->setUserValue("id",0);
    ph_ctrl->addChild( pat );
    root->addChild(ph_ctrl);

    using namespace avAnimation;

    AnimationManagerFinder finder;
    AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();

    anim_file->accept(finder);
    if (finder.getBM()) {
        pat->addUpdateCallback(finder.getBM());
        AnimtkViewerModelController::setModel(finder.getBM());
        AnimtkViewerModelController::addAnimation(anim_idle); 
        AnimtkViewerModelController::addAnimation(anim_running); 

        // We're safe at this point, so begin processing.
        mc.setPlayMode(osgAnimation::Animation::ONCE);
        mc.next();
        mc.play();

        viewer.setSceneData(root);

        const AnimtkViewerModelController::AnimationDurationMap& ad_map = mc.getDurations();

        for (auto it = ad_map.begin(); it!=ad_map.end(); ++it)
        {
            const std::string& cn       = mc.getCurrentAnimationName();

            for (double t=0.0;t<ad_map.at(cn);t+= ad_map.at(cn)/150.0 )
            {
                viewer.frame(t);
            }

            mc.stop();
            mc.next();
            mc.play();
        }


        osg::ref_ptr<osg::TextureRectangle> tex = im.createAnimationData();

        std::string filename = "crow/data.row";
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

