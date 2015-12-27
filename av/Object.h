#pragma once

namespace creators 
{
	  class Object : public osg::Object
	  {

	   public:

		  // constructor and destructor
		  Object();

		  /** Copy constructor using CopyOp to manage deep vs shallow copy. */
		  Object(const Object& object, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
		  
		  Object(osg::Node& node);
		  
	      META_Object(osg, Object);

		  inline osg::Node*  getNode() { return _node.get(); }
		  inline void        addAnimation(const std::string& name, osg::Node*);

	  private:
		  osg::ref_ptr<osg::Node>                           _node;
		  std::map<std::string, osg::ref_ptr<osg::Node> >   _anim_containers;
		  osgAnimation::AnimationMap                        _animations;
		  osg::ref_ptr<osgAnimation::BasicAnimationManager> _manager;
	  };

	  Object*    createObject(std::string name, bool fclone=true);
	  void       releaseObjectCache();

}