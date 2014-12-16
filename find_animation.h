#pragma once

class FindAnimationVisitor : public osg::NodeVisitor 
{ 
public: 
    FindAnimationVisitor(): 
      osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)  
      { 
          //apc=NULL; 
          setTraversalMask(0xffffffff); 
          setNodeMaskOverride(0xffffffff); 
      } 


      //найденные анимации 
      typedef std::vector< osg::ref_ptr<osg::AnimationPathCallback> > NodeList; 
      NodeList _foundNodes; 
      //и соответственно объекты, содержащие эти самые анимации 
      typedef std::vector< osg::ref_ptr<osg::Transform> > TransformList; 
      TransformList _foundTransforms; 

      virtual void apply(osg::Transform& node); 
      virtual void apply(osg::Group& group); 
      /* 
      virtual void apply(osg::MatrixTransform& node); 
      virtual void apply(osg::AnimationPathCallback& node); 
      */ 


private: 
    //Путь анимации 
    //osg::ref_ptr<osg::AnimationPathCallback> apc; 
}; 




void FindAnimationVisitor::apply(osg::Transform& node) 
{ 
    //apc = dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback()); 
    osg::ref_ptr<osg::AnimationPathCallback> test = dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback()); 
    if (test.get()) 
    { 
        //std::cout << "I find animation!!!!"; 
        //Запоминаем анимацию 
        _foundNodes.push_back (dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback())); 
        //и объект к которому она прикреплена 
        _foundTransforms.push_back (&node); 
    } 
    traverse(node); 
} 


void FindAnimationVisitor::apply(osg::Group& group)  
{ 
    traverse(group);  
}  