#pragma once

//
// Local namespaces
//

namespace avLights
{

    //
    // Base visible cultural lights manager class
    //

    class PointLightsManager : public osg::Node
    {
        // internal drawable structure with vertex arrays for rendering and so on
        friend class CollectedCultLights;

    public:

        // constructor
        PointLightsManager();

        // function, which is called by users, who want to place visible light
        // position must be passed in view space coordinate system
        void AddVisibleLight(const osg::Vec3f & vViewPos, const osg::Vec4ub & cColor, float fSize, float fVisDist);

        // scene call this function according to current weather conditions
        void SetWeatherConditions(float fFogDensity, float fRealVisDist, float fIllumination);

    public:

        // cloning not assumed
        osg::Object * cloneType()                           const { return NULL; }
        osg::Object * clone    (const osg::CopyOp & copyop) const { return NULL; }

        // META_Node realization
        bool         isSameKindAs(const osg::Object * obj) const { return static_cast<const PointLightsManager *>(obj) != NULL; }
        const char * className   ()                        const { return "PointLightsManager"; }
        const char * libraryName ()                        const { return "avScene"; }


    private:

        // update callback
        void update(osg::NodeVisitor * nv);
        // cull callback
        void cull(osg::NodeVisitor * nv);

    private:

        // internal drawable structure with vertex arrays for rendering and so on
        class CollectedCultLights;
        std::vector< osg::ref_ptr<CollectedCultLights> > m_aPassesBulk;

        // currently used pass
        size_t m_nCurrentPass;
        CollectedCultLights * m_pCurrentBulk;

        // saved weather state set pointer
        osg::ref_ptr<osg::StateSet> m_pSavedBanksSSMainView;
        osg::ref_ptr<osg::StateSet> m_pSavedBanksSSReflections;

        // saved data for weather
        float m_fIlluminationCur;
        float m_fFogDensityCur;
        float m_fVisDistCur;
    };

}
