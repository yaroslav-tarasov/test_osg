// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "creators.h"
#include "animation_handler.h"
#include "find_node_visitor.h" 

osg::Matrix computeTargetToWorldMatrix( osg::Node* node ) // const
{
    osg::Matrix l2w;
    if ( node && node->getNumParents()>0 )
    {
        osg::Group* parent = node->getParent(0);
        l2w = osg::computeLocalToWorld( parent->
            getParentalNodePaths()[0] );
    }
    return l2w;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

#if 0
    // create the window to draw to.
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = 1920;
    traits->height = 1080;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (!gw)
    {
        osg::notify(osg::NOTICE)<<"Error: unable to create graphics window."<<std::endl;
        return 1;
    }
#endif

    osgViewer::Viewer viewer(arguments);
#if 0
    viewer.getCamera()->setGraphicsContext(gc.get());
    viewer.getCamera()->setViewport(0,0,1920,1080);
#endif

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    std::string animationName("Default");
    
    const osg::Quat quat0(0,          osg::X_AXIS,                      
                          0,          osg::Y_AXIS,
                          0,          osg::Z_AXIS ); 

    // auto model = osgDB::readNodeFile("an_124.dae");  /*a_319.3ds*/
    bool overlay = false;
    osgSim::OverlayNode::OverlayTechnique technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    //while (arguments.read("--object")) { technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY; overlay=true; }
    //while (arguments.read("--ortho") || arguments.read("--orthographic")) { technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY; overlay=true; }
    //while (arguments.read("--persp") || arguments.read("--perspective")) { technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY; overlay=true; }


    // load the nodes from the commandline arguments.
    osg::Node* model = creators::createModel(overlay, technique);

    if(model == nullptr)
    {
         osg::notify(osg::WARN) << "Can't load " <<  "an_124.dae";
        return 1;
    }
    else
    {
        osg::BoundingSphere loaded_bs = model->getBound();

        // create a bounding box, which we'll use to size the room.
        osg::BoundingBox bb;
        bb.expandBy(loaded_bs);

        // tilt the scene so the default eye position is looking down on the model.
        osg::MatrixTransform* rootnode = new osg::MatrixTransform;
        rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(30.0f),1.0f,0.0f,0.0f));
        rootnode->addChild(model);

        // run optimization over the scene graph
        //osgUtil::Optimizer optimzer;
        //optimzer.optimize(rootnode);

        // set the scene to render
        viewer.setSceneData(rootnode);

#ifdef ENABLE_CUSTOM_ANIMATION
        std::list<std::string>  l; 
        auto root_group = model->asGroup();
        if(root_group)
        for(unsigned int i=0; i< root_group->getNumChildren(); ++i)
        {
            std::cout << "Child nodes: " << root_group->getChild(i)->getName() << std::endl;
            l.push_back( root_group->getChild(i)->getName());
            if(l.back() == "Shassis_LO")
            {
                root_group->getChild(i)->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); 
                root_group->getChild(i)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                root_group->getChild(i)->getStateSet()->setRenderBinDetails(1, "DepthSortedBin"); 
                auto shassy = root_group->getChild(i)->asGroup();
                for(unsigned j=0; j< shassy->getNumChildren(); ++j)
                {
                     shassy->getChild(j)->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); 
                     shassy->getChild(j)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                     shassy->getChild(j)->getStateSet()->setRenderBinDetails(1, "DepthSortedBin"); 

                     std::string s_name = shassy->getChild(j)->getName();
                     if (s_name=="lg_left" || s_name=="lg_right" || s_name=="lg_forward" || s_name=="lp18" || s_name=="lp21")
                     {
                         int sign = (s_name=="lg_left" || s_name=="lp21")?1:(s_name=="lg_right" || s_name=="lp18")?-1:1;
                         auto mt_ = shassy->getChild(j)->asTransform()->asMatrixTransform();
                         //auto pos = shassy->getChild(j)->asTransform()->asPositionAttitudeTransform();->getPosition()
                         {
                             osg::Quat quat;
                             if(s_name!="lg_forward")
                             {
                                     quat = osg::Quat (0,   osg::X_AXIS,
                                                       0,   osg::Y_AXIS,
                                 sign*(0.8 * osg::PI_2 ),   osg::Z_AXIS );
                             }
                             else
                             {
                                      quat = osg::Quat (osg::PI_2*1.5,   osg::X_AXIS,
                                                                    0,   osg::Y_AXIS,
                                                                    0,   osg::Z_AXIS );
                                
                             }
                             
                             auto bound = shassy->getChild(j)->getBound();
                             // set up the animation path
                             osg::AnimationPath* animationPath = new osg::AnimationPath;
                             animationPath->insert(0.0,osg::AnimationPath::ControlPoint(bound.center()+ osg::Vec3d(0,bound.radius()/2,0)/* *computeTargetToWorldMatrix(shassy->getChild(j))*/,quat));
                             animationPath->insert(1.0,osg::AnimationPath::ControlPoint(bound.center()+ osg::Vec3d(0,bound.radius()/2,0)/* *computeTargetToWorldMatrix(shassy->getChild(j))*/,quat0));
                             animationPath->insert(0.0,osg::AnimationPath::ControlPoint(bound.center() + osg::Vec3d(0,bound.radius()/2,0)/* *computeTargetToWorldMatrix(shassy->getChild(j))*/,quat));

                             //animationPath->insert(1.0,osg::AnimationPath::ControlPoint(osg::Vec3d(1.,1.,1.)*mt_->getMatrix(),quat0));
                             //animationPath->insert(0.0,osg::AnimationPath::ControlPoint(osg::Vec3d(1.,1.,1.)*mt_->getMatrix(),quat));
                         
                             animationPath->setLoopMode(osg::AnimationPath::SWING);

                             mt_->setUpdateCallback(new osg::AnimationPathCallback(animationPath));
                         }


                         auto mat_ =  mt_->getMatrix();

                         //int sign = (s_name=="lp2")?1:(s_name=="lp3")?-1:0; 
                         //osg::Matrix zRot;
                         //zRot.makeRotate(sign*osg::PI_4, 0.0,0.0,1.0);
                                          
                         //mt_->setMatrix( zRot*mat_ );
                     

                         auto shassy_l = shassy->getChild(j)->asGroup();
                         for(unsigned k=0; k< shassy_l->getNumChildren(); ++k)
                         {
                             shassy_l->getChild(k)->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); 
                             shassy_l->getChild(k)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                             shassy_l->getChild(k)->getStateSet()->setRenderBinDetails(1, "DepthSortedBin"); 
                         
                         }
                     }
                }


            }

        }
#endif

        // viewer.setSceneData(model);

        osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
        viewer.addEventHandler(statesetManipulator.get());

        findNodeVisitor findNode("airplane"); 
        model->accept(findNode);

        auto node =  findNode.getFirst();
        if(node)
            viewer.addEventHandler(new AnimationHandler(node,animationName,
                   [&](){effects::insertParticle(model->asGroup(),node->asGroup(),osg::Vec3(00.f,00.f,00.f),0.f);}
            ));
          
        
        return viewer.run();
    }

    return 1;
}
