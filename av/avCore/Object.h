#pragma once

// frwds
namespace avCore
{
    class InstancedAnimationManager; 
}


namespace avCore 
{
	class Object : public osg::Object
	{
        friend Object* createObject(std::string name, bool fclone);
    public:
        typedef std::map<std::string, osg::ref_ptr<osg::Node> >  AnimationContainersType;

	public:

		// constructor and destructor
		Object();

		/** Copy constructor using CopyOp to manage deep vs shallow copy. */
		Object(const Object& object, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
		  
		Object(osg::Node& node, const std::string  & name);
		  
	    META_Object(osg, Object);

        bool         PreUpdate();
        osg::Node*   getOrCreateNode();

        inline void  setName(const std::string& name ) {_name = name;}           
		inline void  addAnimation(const std::string& name, osg::Node*);
        inline bool  hwInstanced() const { return _hw_instanced;};
        
        bool         parentMainInstancedNode(osg::Group* parent); 
      
    private:
        void         setupInstanced();
        inline osg::Node*   getNode() { return _node.get();}

	private:
		osg::ref_ptr<osg::Node>                           _node;
		AnimationContainersType                           _anim_containers;
		osgAnimation::AnimationMap                        _animations;
		osg::ref_ptr<osgAnimation::BasicAnimationManager> _manager;
        osg::ref_ptr<avCore::InstancedAnimationManager>   _inst_manager;
	//  Settings
	private:
		bool											  _hw_instanced;
        std::string                                       _name;
	};

    typedef std::map< std::string, osg::ref_ptr<Object> > ObjectMap; 
	typedef std::vector< osg::ref_ptr<Object> >           ObjectClones; 

    struct ObjectManager
    {
        friend class Object;
        friend Object* createObject(std::string name, bool fclone);


        boost::optional<ObjectMap::value_type> Find(const std::string& name);
        void           releaseAll();
        bool           PreUpdate();
        
        static ObjectManager& get();
    
    private:
        void           Register( Object* obj );
        void           Register(const std::string& name, Object* obj );
        ObjectManager(){};
    private:
        ObjectMap     objCache_;
        ObjectClones  objClones_;
    };


    Object*    createObject(std::string name, bool fclone=true);
    void       releaseObjectCache();

}