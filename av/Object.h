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

		  inline osg::Node*  getNode() { return node_.get(); }
		  inline void        addAnimation(const std::string& name, osg::Node*);

	  private:
		  osg::ref_ptr<osg::Node>                         node_;
		  std::map<std::string, osg::ref_ptr<osg::Node> > animations_;
	  };

	  Object*    createObject(std::string name, bool fclone=true);
	  void       releaseObjectCache();

}