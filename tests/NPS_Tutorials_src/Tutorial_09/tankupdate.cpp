class tankDataType : public osg::Referenced
{
public:
   tankDataType(osg::Node*n); 
   void updateTurretRotation();
   void updateGunElevation();
   void setRotate(bool r, bool l) {rotate = r;left = l;}
protected:
   osgSim::DOFTransform* tankTurretNode;
   osgSim::DOFTransform* tankGunNode;
   double rotation;
   double elevation;
   bool rotate;
   bool up;
   bool left;
};

void tankDataType::updateTurretRotation()
{   
   if (rotate)
   {
      if (left) rotation += 0.001;
      else rotation -= 0.001;
      tankTurretNode->setCurrentHPR( osg::Vec3(rotation,0,0) );
   }
}


void tankDataType::updateGunElevation()
{
   if (elevation > .5)
      up = false;
   if (elevation < .01)
      up = true;
   if (up) 
     elevation += 0.0001;
   else
     elevation -= 0.0001;
   tankGunNode->setCurrentHPR( osg::Vec3(0,elevation,0) );
}


tankDataType::tankDataType(osg::Node* n)
{
   rotation = 0;
   elevation = 0;
   rotate = false;

   findNodeVisitor findNode("turret"); 
   n->accept(findNode);
   tankTurretNode = 
      dynamic_cast <osgSim::DOFTransform*> (findNode.getFirst());

   findNodeVisitor findGun("gun"); 
   n->accept(findGun);
   tankGunNode = 
      dynamic_cast< osgSim::DOFTransform*> (findGun.getFirst());
}

tankDataType* tankOneData = NULL;

void stopTurretRotation()
{
   tankOneData->setRotate(false,true);
}

void rotateTurretLeft()
{
   tankOneData->setRotate(true,true);
}
void rotateTurretRight()
{
   tankOneData->setRotate(true,false);
}
void toggleAA()
{
   std::cout << "code to toggle AntiAliasing goes here" << std::endl;
}
class tankNodeCallback : public osg::NodeCallback 
{
public:
   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {
      osg::ref_ptr<tankDataType> tankData = 
         dynamic_cast<tankDataType*> (node->getUserData() );
      if(tankData != NULL)
      {
         tankData->updateTurretRotation();
         tankData->updateGunElevation();
      }
      traverse(node, nv); 
   }
};
