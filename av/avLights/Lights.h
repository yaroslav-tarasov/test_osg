#pragma once

//
// Local includes
//

#include <osg/Node>
#include <osg/Geometry>



namespace osgUtil {
    class CullVisitor;
};


//
// Module namespace
//

namespace avScene
{

    // maximum number of visible dynamic lights per object
    static const unsigned nMaxLights = 124;
    static const unsigned nMaxLMLights = 1000;

    // light affect type
    enum LightInfluence
    {
        GlobalInfluence = 0,
        LocalInfluence
    };

    // light type
    enum LightType
    {
        PointLight = 0,
        ConicalLight
    };

    // light external and processed info struct
    struct LightExternalInfo;
    struct LightProcessedInfo;
    
    // light processed info struct
    struct LightProcessedInfo
    {
        osg::Vec4f lightVSPosAmbRatio;
        osg::Vec4f lightVSDirSpecRatio;
        osg::Vec4f lightAttenuation;
        osg::Vec3f lightDiffuse;
    };
    
    // light external info struct
    struct LightExternalInfo
    {
        unsigned     uPriority;
        cg::point_3f vPosWorld;
        cg::vector_3 vDirWorld;
        cg::range_2f rDistAtt, rConeAtt;
        cg::colorf   cDiffuse;
        float        fAmbRatio, fSpecRatio; 
        bool         bHighPriority;

        bool operator< ( const LightExternalInfo & second ) const
        {
            return uPriority < second.uPriority;
        }
    };

    //
    // Lights manager class
    //

    struct LightNodeHandler;

    class Lights : public osg::Node
    {
        friend struct LightNodeHandler;

    public:

        // constructor and destructor
        Lights();


    public:

        // new style - add active light info
        // each update must be called to place active light here
        void AddLight( LightInfluence dlInfluence, LightType dlType, bool HighPriority,
                       const cg::point_3f & vWorldPos, const cg::vector_3 & vWorldDir,
                       const cg::range_2f & rDistAtt, const  cg::range_2f & rConeAtt,
                       const cg::colorf & cDiffuse, const float & fAmbRatio, const float & fSpecRatio );

        //
        // OSG node interfaces
        //

        // cloning not assumed
        osg::Object * cloneType()                           const { return NULL; }
        osg::Object * clone    (const osg::CopyOp & copyop) const { return NULL; }

        // META_Node realization
        bool         isSameKindAs(const osg::Object * obj) const { return static_cast<const Lights *>(obj) != NULL; }
        const char * className   ()                        const { return "Lights"; }
        const char * libraryName ()                        const { return "avScene"; }

        const std::vector<LightProcessedInfo>&  GetProcessedLights() {return m_aMainProcessedLights;}
        const std::vector<LightExternalInfo>&  GetVisibleLights()    {return m_aMainVisibleLights;}

    private:

        // tricky mega-accept
        void accept( osg::NodeVisitor & nv );

        // update callback
        void update( osg::NodeVisitor * nv );
        // cull callback
        void cull( osg::NodeVisitor * nv );


    private:

        // currently active lights to test on culling pass
        std::vector<LightExternalInfo> m_aFrameActiveLights;

        // currently visible lights for current cull pass
        std::vector<LightExternalInfo>  m_aMainVisibleLights;
        std::vector<LightProcessedInfo> m_aMainProcessedLights;
        // currently active lights to test on culling pass
        std::vector<LightExternalInfo>  m_aReflVisibleLights;
        std::vector<LightProcessedInfo> m_aReflProcessedLights;
    };


    //
    // Lights handler class
    // It must be presetned and called in the place where you want to handle dynamic lights
    //

    struct LightNodeHandler
    {

    public:

        LightNodeHandler( LightInfluence maxInfluenceToUse, bool init = false );

    public:

        // on cull handling
        void onCullBegin( osgUtil::CullVisitor * pCV, const osg::BoundingSphere * pSpherePtr = NULL );
        void onCullEnd  ( osgUtil::CullVisitor * pCV );

    private:

        // lights pack stateset already prepared
        struct LightsPackStateSet
        {
            osg::ref_ptr<osg::Uniform> LightVSPosAmbRatio;
            osg::ref_ptr<osg::Uniform> LightVSDirSpecRatio;
            osg::ref_ptr<osg::Uniform> LightAttenuation;
            osg::ref_ptr<osg::Uniform> LightDiffuse;
            osg::ref_ptr<osg::Uniform> LightsActiveNum;
            osg::ref_ptr<osg::StateSet> pStateSet;

            LightsPackStateSet();
        } m_lightsMain, m_lightsRefl;

        // father reference
        const Lights * m_pFatherRef;

        // max influence used
        unsigned m_uMaxPriority;
        // whether state was pushed or not
        bool m_bStatePushed;

        // last time active flags
        bool m_bMainWasActive, m_bReflWasActive;

        static bool   _init;
            
    };

    //
    // Helper for cull method attaching with convenient way-to-go
    //

    class DynamicLightsObjectCull : public osg::NodeCallback
    {
    public:

        // constructor
        DynamicLightsObjectCull( LightInfluence maxInfluenceToUse );

        // cull oparator
        void operator()( osg::Node * node, osg::NodeVisitor * nv );

    private:

        // associated handler
        LightNodeHandler m_LightsHandler;
    };

}
