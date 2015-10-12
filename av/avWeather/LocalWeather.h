#pragma once

//
// Local includes
//

#include "av/avWeather/Weather.h"

#define GOT_FOG

//
// Local namespaces
//

namespace avWeather
{

    // forward declarations
    class LocalFogBank;
    class Cloud;


    //
    // Local weather bank class
    // Implements cylindrical local weather functionality
    //

    class LocalWeather : /*public avCore::Transformation,*/ public osg::Referenced
    {
    friend class Weather;

    public:

        // constructor
        LocalWeather(Weather * pFather, size_t nID);


    public:

        //
        // Base local weather interface
        //

        // set bank radii and height (min height is assuming as tide level, which is zero for Environment node, under which we are placed)
        void SetEllipse(float fRadX, float fRadY, float fMaxH);
        // set precipitation, density and portion
        void SetPrecipitationDensityPortion(PrecipitationType pType, float fInnerInt, float fPortion);

        // get precipitation type
        const PrecipitationType GetPrecipitationType() const { return m_ptType; }


    public:

        //
        // Transformation interface
        //

        // react on matrix has been changed
        virtual void SetWorldTransformationMatrix(const osg::Matrix & mMatrix);


    public:

        // special culling method
        // return value - is local fog part visible
        bool cull(osgUtil::CullVisitor * pCV);


    private:

        // father ref
        osg::ref_ptr<Weather> m_pFatherRef;

        // geometric data
        float m_fRadX, m_fRadY, m_fMaxH;
        // precipitation type
        PrecipitationType m_ptType;
        // density and portion
        float m_fDensity, m_fPortion;

        // model matrix
        osg::Matrix m_mModelMatrix;
#ifdef GOT_FOG
        // fog bank object
        osg::ref_ptr<LocalFogBank> m_pFogBank;
#endif
        // local cloud object
        osg::ref_ptr<Cloud> m_pCloud;
    };

}