#pragma once

//
// Local includes
//

#include <osg/Node>
#include <osg/Geometry>

//
// Forward declarations
//

namespace osgUtil
{
    class CullVisitor;
}

//
// Module namespace
//

namespace avWeather
{

    // different precipitation types
    enum PrecipitationType
    {
        // 3 types which introduce some particles around us
        PrecipitationSnow = 0,
        PrecipitationRain,
        PrecipitationHail,

        // number of particles weather types
        PrecipitationNumOf,
        // fog has no particles, but treated as precipitation
        PrecipitationFog = PrecipitationNumOf,

        // number of total weather types
        LocalWeatherNumOf,
    };

    // static function of getting precipitation color based on its type
    osg::Vec3f GetPrecipitationColor(PrecipitationType pType);


    // forward class defining local weather
    class LocalWeather;

    //
    // Global weather class
    //

    class Weather : public osg::Node
                  
    {

    public:

        // constructor
        Weather();

    public:

        //
        // Interface for local weather
        //

        // remove local weather bank (by ID)
        typedef unsigned int WeatherBankIdentifier;
        void RemoveLocalWeatherBank(WeatherBankIdentifier nID);
        // add or update local weather bank (by ID)
        void UpdateLocalWeatherBank(WeatherBankIdentifier nID,
            double dLat, double dLon, float fHeading,
            float fEllipseRadX, float fEllipseRadY, float fHeight,
            PrecipitationType pType, float fIntensity, float fCentralPortion);

        //
        // Internal callbacks and methods
        //

        // called back by local weather to update precipitation intensity
        void SetLocalPrecipitationIntensity(PrecipitationType pType, float fLocalInt);

        // get stateset with uniforms for proper fogging
        osg::StateSet * GetMainViewLocalFogBanksStateSet() const;
        osg::StateSet * GetReflectionsLocalFogBanksStateSet() const;

        //
        // OSG node interfaces
        //

        // cloning not assumed
        osg::Object * cloneType()                           const { return NULL; }
        osg::Object * clone    (const osg::CopyOp & copyop) const { return NULL; }

        // META_Node realization
        bool         isSameKindAs(const osg::Object * obj) const { return static_cast<const Weather *>(obj) != NULL; }
        const char * className   ()                        const { return "Weather"; }
        const char * libraryName ()                        const { return "avWeather"; }

    private:

        // tricky mega-accept
        void accept(osg::NodeVisitor& nv);

        // update callback
        void update(osg::NodeVisitor * nv);
        // cull callback
        void cull(osg::NodeVisitor * nv);

    private:

        // for culling pass - add global precipitation to rendering bin
        void _registerPrecipitation(osgUtil::CullVisitor * pCV, osg::Geometry * pPrep);

        // all precipitation handlers
        std::vector< osg::ref_ptr<osg::Geometry> > m_pPrecipitations;

        // last camera position
        osg::Vec3f m_vSavedCamPos;
        osg::Vec3f m_vSavedCamVel;
        float      m_fSavedTimeStamp;

        // uniform with depth discarding enabled flag
        osg::ref_ptr<osg::Uniform> m_uniformDepthDiscardingEnabled;

        // uniform with world-to-ship-depth matrix
        osg::ref_ptr<osg::Uniform> m_uniformWorld2ShipDepthTex;

        // local weather banks
        typedef std::map< WeatherBankIdentifier, osg::ref_ptr<LocalWeather> > LocalBanksMap;
        LocalBanksMap m_mapLocalBanks;

        // local weather fog banks uniforms pack
        static const int m_nMaxVisibleBanks = 2;
        struct BanksStateSetPack
        {
            osg::ref_ptr<osg::Uniform> bankViewToLocal;
            osg::ref_ptr<osg::Uniform> bankRadiiHeight;
            osg::ref_ptr<osg::Uniform> bankRcpRadiiHeight;
            osg::ref_ptr<osg::Uniform> bankColor;
            osg::ref_ptr<osg::Uniform> bankDensityControl;
            osg::ref_ptr<osg::StateSet> ssFogBanks;

            BanksStateSetPack();
        };
        BanksStateSetPack m_BanksMainView, m_BanksRefl;
    };

} // end namespace avWeather


