#pragma once

//
// Local includes
//

#include <osg/Geometry>

//
// Local namespaces
//

namespace avWeather
{

    //
    // Base precipitation class
    // Implements scrolling box functionality and maintains it
    // Spreads vertex positions randomly inside defined box
    //

    class PrecipitationBase : public osg::Geometry
    {

    public:

        // constructor
        PrecipitationBase(const char * szMaterialName, size_t nParticlesNumber, const osg::Vec3f & vBoxHalfSize, float fMeanLifeTime = 10.f);

        //
        // Setters
        //

        // set precipitation intensity
        void SetIntensity(float fIntensity);
        // set wind direction and force
        void SetWind(const osg::Vec2f & vWindDir, float fForce);

        // set camera position
        void SetViewerPos(const osg::Vec3f & vPos, const osg::Vec3f & vDir);
        // set camera speed
        void SetCameraVelocity(const osg::Vec3f & vCamVel);

        // set rendering data
        void SetRenderData(float fScrClarity, float fMagicIllum);

        //
        // Getters
        //

        // get precipitation intensity
        float GetIntensity() const;
        // get active state
        bool IsActive() const;


    protected:

        // particles number
        size_t m_nParticlesNumber;
        // DIP call
        osg::ref_ptr<osg::DrawArrays>  m_dipDrawArrays;

        // box half size
        osg::Vec3f m_vBoxHalf;

        // uniform with scroll LBN (left-bottom-near) point
        osg::ref_ptr<osg::Uniform> m_uniformScrollLBN;
        // uniform with LTP offset in weather plane
        osg::ref_ptr<osg::Uniform> m_uniformWP2LTPBoxOffset;

        // active flag
        bool       m_bActive;

        // saved intensity
        float      m_fIntensity;

        // camera velocity
        osg::Vec3f m_vCamVel;

        // wind data
        osg::Vec3f m_vWindDir;
        float      m_fWindSpeed;
        // uniform with wind vector
        osg::ref_ptr<osg::Uniform> m_uniformWindVec;

        // uniform with rendering related data
        osg::ref_ptr<osg::Uniform> m_uniformRenderData;

    };

    // smart pointer definition
    typedef osg::ref_ptr<PrecipitationBase> PrecipitationBasePtr;
}