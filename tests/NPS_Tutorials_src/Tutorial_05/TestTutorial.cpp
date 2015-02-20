#include <osg/PositionAttitudeTransform>
#include <osg/Group>
#include <osg/Node>
#include <osgDB/ReadFile> 
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>



int main()
{
osg::Node* tankNode = NULL;
osg::Group* root = NULL;
osgViewer::Viewer viewer;
osg::Vec3 tankPosit; 
osg::PositionAttitudeTransform* tankXform;

tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");

root = new osg::Group();
tankXform = new osg::PositionAttitudeTransform();

root->addChild(tankXform);
tankXform->addChild(tankNode);

tankPosit.set(5,0,0);
tankXform->setPosition( tankPosit ); 
viewer.setCameraManipulator(new osgGA::TrackballManipulator());

//viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
viewer.setSceneData( root );
viewer.realize();

while( !viewer.done() )
{
viewer.frame();
}
}
