#pragma once 


 namespace avEnv
 {
     osg::Group* createPrerender(osg::Node* reflectedSubgraph, /*osg::NodePath reflectorNodePath,*/ unsigned int unit, const osg::Vec4& clearColor,  osg::Camera::RenderTargetImplementation renderImplementation);

 }