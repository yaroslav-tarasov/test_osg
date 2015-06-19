/* -*-c++-*- OpenSceneGraph - Ephemeris Model Copyright (C) 2005 Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#ifndef OSG_EPHEMERIS_EPHEMERIS_MODEL_DEF
#define OSG_EPHEMERIS_EPHEMERIS_MODEL_DEF

#include "SkyDome.h"
#include "StarField.h"
#include "Moon.h"
#include "av/FogLayer.h"

#include "EphemerisData.h"
#include "EphemerisEngine.h"
#include "DateTime.h"

#include "av/Utils.h"

namespace avSky
{


/**
  \class EphemerisModel
  \brief The highest level class in the avSky library allowing for
         the setting of parameters which control the placement of heavenly
         bodies, namely the Sun, Moon, Stars and Planets, in the virtual scene.

         EphemersModel is the programming interface to set the parameters
         for controlling the placement of heavenly bodies, namely the 
         Sun, the Moon, Stars and Planets in the virtual scene.  

         EphemerisModel initializes a SkyDome which controls the 
         sky color and provides a surface for projection of the texture of
         the Sun.  It also retains a pointer to EphemerisData, which
         contains the actual parameters for computing the positions of
         the heavenly bodies, namely latitude, longitude, altitude, date
         and time.

         EphemerisModel by default uses an EphemerisEngine to process
         the information about heavenly body position.  

         EphemerisModel also defines a light, for which it controls the
         position and colors, representing the light from the Sun.  

         EphermerisModel is intended to be a ground bound or near ground
         bound view presentation of heavenly bodies and not intended for
         space-based observation of heavenly bodies.

         EphemerisModel derives from osg::Group and contains an internal 
         traverse() method where it does internal updates at run-time.
         */
  
class EphemerisModel : public osg::Group, public EphemerisData
{
public:

    /** The default constructor */
    EphemerisModel( osg::Group* scene );

    /** The copy constructor */
    EphemerisModel( const EphemerisModel & copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

    //META_Node(svSkyModel, EphemerisModel);



    /**
      Set the current eyepoint latitude in radians.
      */
    void setLatitude( double latitude );
    /**
      Get the current eyepoint latitude in radians.
      */
    double getLatitude() const;

    /**
      Set the current eyepoint longitude in radians.
      */
    void setLongitude( double longitude );
    /**
      Get the current eyepoint longitude in radians.
      */
    double getLongitude() const;

    /**
      Set both the latitude and longitude of the current eyepoint
      */
    void setLatitudeLongitude( double latitude, double longitude );
    /**
      Get both the latitude and longitude of the current eyepoint
      */
    void getLatitudeLongitude( double & latitude, double & longitude ) const;

    /**
      Set the latitude, longitude, and altitude of the current eyepoint
      */
    void setLatitudeLongitudeAltitude( double latitude, double longitude, double altitude );
    /**
      Get the latitude, longitude, and altitude of the current eyepoint
      */
    void getLatitudeLongitudeAltitude( double &latitude, double &longitude, double &altitude ) const;

    /** 
      Set the current date and time by passing a DateTime struct
      */
    void setDateTime( const DateTime & dateTime );
    /**
      Get the current date and time setting in a DateTime struct
      */
    DateTime getDateTime() const;

    /**
        Convenience function to get a pointer to the Sun Light Source.
        */
    osg::LightSource* getSunLightSource(){ return _sunLightSource.get(); }

    /**
      * Effect an update.  Used internally by the internal UpdateCallback
      */
    void update( osg::NodeVisitor * nv );
    


    // fog control
    void setFogDensity(float fFogDensity);

    // manual illumination control
    void setManualMode(bool bManual, float fIllumDesired);

    // force update underwater color
    void updateUnderWaterColor( const osg::Vec3f & cWaterColor );

    bool FrameCall();

private:

    void _setDefault();
    bool _initialize();

    void _updateSun();
    void _updateStars();
    void _updateMoon();
    void _calculateIllumFactors();

    osg::Group * _scene;

    osg::ref_ptr<osg::LightSource> _sunLightSource;

    osg::ref_ptr<SkyDome>    _skyDome;
    osg::ref_ptr<avSky::FogLayer>   _skyFogLayer;
    osg::ref_ptr<StarField>  _skyStarField;
    osg::ref_ptr<MoonSphere> _skyMoon;

    float _fogDensity;
    osg::Vec3f _cFogColor;

    bool _manualSunElevationMode;
    float _desiredIllumination;

    osg::Vec3d _sunVec;

    osg::ref_ptr<EphemerisEngine> _ephemerisEngine;

    float _illumination;
    osg::ref_ptr<osg::Uniform> _uniformSunIntensity;

    int _nCloudsType;
    float _fCloudsDensity;

public:

    bool    _needsToUpdate;

    void setHumidityTemperature( float humidity, float skytemperature );

    float getIllumination() const { return _illumination; } //-- [0.0 ... 1.0]

    const osg::Vec3f & getFogColor() const { return _cFogColor; }

    void SetClouds( int nType, float fCloudIntensity = 1.0f );
};


} // namespace
#endif
