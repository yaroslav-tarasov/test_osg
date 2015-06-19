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

#ifndef EPHEMERIS_DATA_DEF
#define EPHEMERIS_DATA_DEF

#include "DateTime.h"

namespace avSky {

/** \class EphemerisData
    \brief A container for the parameters used by the EphemerisEngine to compute
           heavenly body positions and to store those positions.

          EphemerisData is a container for the parameters used by EphemerisEngine
          to compute heavenly body positions and to store those positions after
          computation.  
  */


struct EphemerisData
{
    // world spherical viewer position
    // lat/lon are in radians
    double latitude, longitude, altitude;

    // atmospheric parameters
    float turbidity, temperature;

    // current date-time
    DateTime dateTime;

    // Julian Date Time
    double modifiedJulianDate;
    // Local Sidereal Time
    double localSiderealTime;

    // sun related data (radians)
    double sunAzimuth, sunAltitude;
    // moon related data (radians)
    double moonAzimuth, moonAltitude;

    // default constructor
    EphemerisData()
        : latitude(0.0)
        , longitude(0.0)
        , altitude(0.0)
        , turbidity(0.0f)
        , temperature(0.0f)
        , sunAzimuth(0.0f)
        , sunAltitude(0.0f)
        , moonAzimuth(0.0f)
        , moonAltitude(0.0f)
    {
        return;
    }
};

}

#endif
