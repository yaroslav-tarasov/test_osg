#pragma once

//
// Local includes
//

#include <osg/Geometry>

#include "av/avWeather/Weather.h"

//
// Forward includes
//

namespace avScene
{
    class ScreenTexture;
}

//
// Local namespaces
//

namespace avWeather
{

    //
    // Fog bank class
    // Implements cylindrical local fog functionality
    // Cylinder is elliptical with specified density factor and vertical density falloff
    // We assume, it is correctly placed and turned on the upper hierarchy level
    //

    class LocalFogBank : public osg::Geometry
    {
    friend class Weather;

    public:

        // constructor
        LocalFogBank();


        //
        // Base local fog interface
        //

        // set bank radii and height (min height is assuming as tide level, which is zero for Environment node, under which we are placed)
        void SetEllipse( float fRadX, float fRadY, float fMaxH );
        // set precipitation, density and portion
        void SetPrecipitationDensityPortion( PrecipitationType ptType, float fInnerInt, float fPortion, float fLowerBoundDensityScale );

    public:

        // get back AABB for fast clipping
        const osg::BoundingBox & GetCyliderAABB() const { return m_aabbCylinder; }

        // special culling method
        void cull( osgUtil::CullVisitor * pCV );


    private:

        // draw tricky implementation
        virtual void drawImplementation( osg::RenderInfo & renderInfo ) const;


    private:

        // geometric data
        float m_fRadX, m_fRadY, m_fMaxH;
        // density and portion
        float m_fDensity, m_fPortion, m_fDensityLow;
        // precipitation type
        PrecipitationType m_ptType;

        // AABB data for clipping
        osg::BoundingBox m_aabbCylinder;
        // AABB calculation routine
        void _recalcBound();

        // local 2 view matrix
        osg::Matrix m_mViewToLocal;

        // uniform with radii and height
        osg::ref_ptr<osg::Uniform> m_uniformRadiiHeight;
        // uniform with radii and height reciprocals
        osg::ref_ptr<osg::Uniform> m_uniformRcpRadiiHeight;
        // uniform with density control
        osg::ref_ptr<osg::Uniform> m_uniformDensity;
        // uniform with color control
        osg::ref_ptr<osg::Uniform> m_uniformColor;
        // uniforms update routine
        void _resetUniforms();

        // reflection stateset (some tricks with depth texture)
        osg::ref_ptr<osg::StateSet> m_ssReflectionSubstitute;

        // screen depth texture
        osg::ref_ptr<avScene::ScreenTexture> m_texDepth;
    };

}