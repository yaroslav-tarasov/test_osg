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
#ifndef OSGEPHEMERIS_EPHEMERIS_ENGINE_DEF
#define OSGEPHEMERIS_EPHEMERIS_ENGINE_DEF

#include <osg/ref_ptr>
#include "EphemerisData.h"
#include "CelestialBodies.h"
#include "DateTime.h"

namespace avSky {

    /**\class EphemerisEngine
       \brief A class containing computational routines for processing heavely body position
              from latitude, longitude, altitude, date and time.
      */
      
class EphemerisModel;

class EphemerisEngine: public osg::Referenced
{
    public:

        /**
          Constructor
          */
        EphemerisEngine( EphemerisData * ephemerisData );

        /**
          Set the latitude in degrees
          */
        void setLatitude( double latitude);
        /**
          Set the longitude in degrees.
          */
        void setLongitude( double longitude );
        /**
          Set both the latitude and longitude in degrees.
          */
        bool setLatitudeLongitude( double latitude, double longitude );
        /**
          Set the latitude, longitude in degrees and the altitude in meters.
          */
        bool setLatitudeLongitudeAltitude( double latitude, double longitude, double altitude );
        /**
          Set the date and time with a DateTime structure.  The parameter passed 
          is copied to the internal DateTime structure.
          */
        bool setDateTime( const DateTime &dateTime );

        // set some additional atmospheric parameters
        bool setTurbidityTemperature( float turbidity, float temperature );

        /**
          Update heavenly body positions.
         */
        void update();


    protected:
        ~EphemerisEngine() {}

    private:

        EphemerisData           * _ephemerisData;

        osg::ref_ptr<avSky::Sun>  _sun;
        osg::ref_ptr<avSky::Moon> _moon;

        void _updateData( const EphemerisData &, const CelestialBody &, double, double &, double &);

        static void _getLsnRsn( double mjd, double &lsn, double &rsn);
        static void _getAnomaly( double ma, double s, double &nu, double &ea);
        static double getLocalSiderealTimePrecise( double mjd, double longitude );
        static void _RADecElevToAzimAlt( 
                double rightAscension,
                double declination,
                double latitude,
                double localSiderealTime,
                double elevation, // In meters above sea level
                double rsn,
                double &azim,
                double &alt );
        static void _calcParallax ( 
                    double tha, double tdec,    // True right ascension and declination
                    double phi, double ht,      // geographical latitude, height abouve sealevel
                    double ehp,                 // Equatorial horizontal parallax
                    double &aha, double &adec); // output: aparent right ascencion and declination
        static void _aaha_aux (double lat, double x, double y, double *p, double *q);
};


}
#endif
