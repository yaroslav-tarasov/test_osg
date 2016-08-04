#pragma once

namespace avScene
{

class LightManager 
    : public osg::Node
{
	struct DelLight : osg::Referenced
	{
		DelLight(LightManager* lm, uint32_t lid)
			: lid (lid), lm(lm) {}

		~DelLight();

		uint32_t      lid;
	    LightManager* lm;
	};

	struct DelLights : osg::Referenced
	{
		~DelLights();
		std::list<osg::ref_ptr<DelLight>> d_lights;
	};

private:

    LightManager();
    ~LightManager();


public:
    struct Light
    {
        //osg::Transform * transform;
		boost::optional<osg::observer_ptr<osg::Transform>> parent_transform;
        cg::range_2f     spotFalloff;
        cg::range_2f     distanceFalloff;
        cg::colorf       color;

        cg::point_3f     position;
        cg::vector_3f    direction;

        bool             active;
        bool             high_priority;
        bool             lm_only;
        float            normal_coeff;

        inline Light() : /*transform(nullptr),*/ active(false), high_priority(false), lm_only(false), normal_coeff(0.f) { }
    };

public:
    static void Create();
    static void Release();

    static LightManager * GetInstance();



public:

    void      update  ( osg::NodeVisitor * nv );
    
	uint32_t  addLight( const Light& light);
	uint32_t  addLight(const Light& data, osg::Node* remove_with);

    void      removeAllLight();

   Light*     getLight(uint32_t id) {auto it = _LightsMap.find(id); if(it!=_LightsMap.end()) return &(*it).second; else return nullptr;};

private:
	uint32_t  genUID();

private:

    static osg::ref_ptr<LightManager> g_Instance;

    typedef std::map<unsigned,Light>  LightsMap;
    LightsMap                         _LightsMap;
    mutable OpenThreads::Mutex        _lightManagerMapMutex;
};


} // namespace avScene
