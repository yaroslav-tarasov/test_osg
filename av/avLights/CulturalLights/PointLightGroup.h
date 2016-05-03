#pragma once

//
// Local namespaces
//

namespace avLights
{

    //
    // Cultural light group node class
    //

    class PointLightGroup : public osg::Node
    {
    public:

        // constructor
        PointLightGroup();

        //
        // Interface functions
        //

        // base set up
        void ClearLights         ();
        void AddCulturalLight    ( const osg::Vec3f & vPos, const osg::Vec4ub & cCol, float fSize, float fVisDist );
        void AddNavigationalLight( const osg::Vec3f & vPos, const osg::Vec4ub & cCol, float fSize, float fVisDist,
                                   float fStartAngle, float fFinishAngle, const std::vector<float> * paBlinkSeq, float fTimeDelta );
        void SetEnableThreshold  ( float fThreshold );

        // get info
        size_t GetLightsNum() const;

    public:

        // cloning not assumed
        osg::Object * cloneType()                           const { return NULL; }
        osg::Object * clone    (const osg::CopyOp & copyop) const { return NULL; }

        // META_Node realization
        bool         isSameKindAs(const osg::Object * obj) const { return static_cast<const PointLightGroup *>(obj) != NULL; }
        const char * className   ()                        const { return "PointLightGroup"; }
        const char * libraryName ()                        const { return "avScene"; }

        // tricky accept method
        // yes, we have callbacks, but we don't want to make redundant work on pushing/popping and so on
        // so, we make accept() and call appropriate callbacks by ourselves
        void accept(osg::NodeVisitor & nv);

    private:

        // cull callback
        void cull(osg::NodeVisitor * nv);

    private:

        // bound info
        osg::BoundingBox m_aabbLights;
        float m_fMaxVisDistSqr;

        // base info for cultural light
        struct CultLightInfo
        {
            osg::Vec3f vPos;
            osg::Vec4ub cCol;
            float fSize;
            float fVisDist;
        };
        std::vector<CultLightInfo> m_aCultLights;

        // base info for navigational light
        struct NavLightInfo : public CultLightInfo
        {
            float fSectorStart;
            float fSectorEnd;
            float fSectorFadeInv;

            std::vector<osg::Vec4f> aBlinkSequenceData;
            float fTimeDelta;
            float fSequenceDuration;
        };
        std::vector<NavLightInfo> m_aNavLights;

        // enable threshold
        float m_fThresholdDelta;
    };

}
