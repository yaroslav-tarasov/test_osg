#include <osg/Group>
bool setupScene(osg::ref_ptr<osg::Group> &rootNode,
                osg::ref_ptr<osg::Group> &ownTank  )

// Setup a scene, return (via reference arguments) handles
// to the root node and the tank model.  
// Attach an update callback to the tank transform so the tank
// circles above our terrain model.
// Function returns false if it can't load
//  models or arguments are not NULL, true otherwise.
{

   rootNode = new osg::Group();
   osg::ref_ptr<osg::Node> terrainNode = 
      osgDB::readNodeFile("../NPS_Data/Models/JoeDirt/JoeDirt.flt");
   if (!terrainNode)
   {
      std::cout << " no terrain! " << std::endl;
      return false;
   }
   rootNode->addChild(terrainNode.get());

   ownTank = (osg::Group*) 
      osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   if( ! ownTank)
   {
      std::cout << "no Tank" << std::endl;
      return false;
   }

   osg::MatrixTransform * tankXForm = 
      new osg::MatrixTransform();
   tankXForm->setUpdateCallback( new circleAimlessly() );
   rootNode->addChild(tankXForm);
   tankXForm->addChild(ownTank.get());

   return true;
}
