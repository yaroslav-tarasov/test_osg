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
    static const unsigned nMaxLights = 24;

    // light affect type
    // local - means it doesn't affect sea and may be other ships (but do affect other dynamic objects forsure)
    // global - it affects anything and is always on top of the list
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
        void AddLight( LightInfluence dlInfluence, LightType dlType,
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
        const char * libraryName ()                        const { return "svScene"; }

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

        LightNodeHandler( LightInfluence maxInfluenceToUse );

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
