#pragma once

//
// Local includes
//

#include <osg/Geometry>

#include "av/avWeather/Weather.h"

//
// Local namespaces
//

namespace avWeather
{

    //
    // Cloud class
    // Implements elliptical cloud
    // We assume, it is correctly placed and turned on the upper hierarchy level
    //

    class Cloud : public osg::Geometry
    {

    public:

        // constructor
        Cloud(size_t nID);


        //
        // Base cloud interface
        //

        // set ellipsoid
        void SetEllipsoid(float fRadX, float fRadY, float fRadZ, float fHeight);
        // set father precipitation type
        void SetPrecipitationDensity(PrecipitationType pType, float fDensity);


    public:

        // get back AABB for fast clipping
        const osg::BoundingBox & GetEllipsoidAABB() const { return m_aabbEllipsoid; }
        // get height correction
        float GetHeightCorrection() const { return m_fHeightCorrected - m_fHeight; }

    public:

        // special culling method
        void cull(osgUtil::CullVisitor * pCV);


    private:

        // geometric data
        float m_fRadX, m_fRadY, m_fRadZ, m_fHeight, m_fHeightCorrected;

        // current precipitation type and density
        PrecipitationType m_ptType;
        float m_fDensity;

        // AABB data for clipping
        osg::BoundingBox m_aabbEllipsoid;

        // vertex data just for sorting
        osg::ref_ptr<osg::Vec3Array> m_paEllipsePointPos;
        // DIP call
        osg::ref_ptr<osg::DrawElementsUShort> m_dipDrawElem;

        // uniform with radii and height
        osg::ref_ptr<osg::Uniform> m_uniformRadiiHeight;
        osg::ref_ptr<osg::Uniform> m_uniformInvRadiiSize;

        // uniform with color control
        osg::ref_ptr<osg::Uniform> m_uniformColorBottom, m_uniformColorTop;
    };

}