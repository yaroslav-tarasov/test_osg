#pragma once

namespace avScene
{

class LightManager 
    : public osg::Node
{
private:

    LightManager();
    ~LightManager();


public:

    static void Create();
    static void Release();

    static LightManager * GetInstance();

public:

    void update  ( osg::NodeVisitor * nv );
    void addLight(uint32_t id, osg::MatrixTransform* mt);

private:

    static osg::ref_ptr<LightManager> g_Instance;

    struct Light;
    typedef std::map<unsigned,Light> LightsMap;
    LightsMap m_LightsMap;
};


} // namespace svScene
