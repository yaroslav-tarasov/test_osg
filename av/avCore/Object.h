#pragma once

//
// frwrds
//

namespace avCore
{
	struct  InstancesManager;
}

namespace avCore 
{
	struct ObjectInfo 
	{
		virtual bool        hwInstanced() const =0;;
	};
	
	struct ObjectControl : ObjectInfo
	{
		 virtual osg::Node*  getOrCreateNode()=0;
		 virtual bool        parentMainInstancedNode(osg::Group* parent)=0;
	};

	class Object : public osg::Object
		         , public ObjectControl
	{
        friend struct ObjectManager;
		friend ObjectControl* createObject(std::string name, uint32_t seed,bool fclone);
    public:
        typedef std::map<std::string, osg::ref_ptr<osg::Node> >  AnimationContainersType;

	public:

		// constructor and destructor
		Object();

		/** Copy constructor using CopyOp to manage deep vs shallow copy. */
		Object(const Object& object, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
		  
		Object(osg::Node& node, const std::string  & name, bool fhw_inst);
		  
	    META_Object(osg, Object);

        bool         PreUpdate();
        osg::Node*   getOrCreateNode();

        inline void  setName(const std::string& name ) {_name = name;}           
        inline void  addAnimation(const std::string& name, osg::Node*);
		inline void  addAnimation(const std::string& hw_anim_file);
        

      
    private:
        void         setupInstancedHWAnimated(const std::string& hw_anim_file);
        inline osg::Node*   getNode() { return _node.get();}
		bool         hwInstanced() const;
        bool         parentMainInstancedNode(osg::Group* parent); 
		void         setSeed (uint32_t seed){ _seed = seed;}
		uint32_t     getSeed () const { return _seed;}

	private:
		osg::ref_ptr<osg::Node>                           _node;
		AnimationContainersType                           _anim_containers;
		osgAnimation::AnimationMap                        _animations;
		osg::ref_ptr<osgAnimation::BasicAnimationManager> _manager;
        osg::ref_ptr<avCore::InstancesManager>            _inst_manager;

		uint32_t                                          _seed;
	//  Settings
	private:
        std::string                                       _name;
	};

    typedef std::map< std::string, osg::ref_ptr<Object> > ObjectMap; 
	typedef std::list< osg::ref_ptr<Object> >           ObjectClones; 

    struct ObjectManager
    {
        friend class Object;
        friend ObjectControl* createObject(std::string name, uint32_t seed,bool fclone);
		friend void           releaseObject( uint32_t seed );

        boost::optional<ObjectMap::value_type> Find(const std::string& name);
        void           releaseAll();
	    void           releaseObject(uint32_t seed);
        bool           PreUpdate();
        
        static ObjectManager& get();
    
    private:
        void           RegisterClone( Object* obj );
        void           RegisterPrototype(const std::string& name, Object* obj );

        ObjectManager(){};
    private:
        ObjectMap     objCache_;
        ObjectClones  objClones_;
    };


    ObjectControl*   createObject (std::string name, uint32_t seed, bool fclone=true);
	void             releaseObject( osg::Node* node );
    void             releaseObjectCache();

}