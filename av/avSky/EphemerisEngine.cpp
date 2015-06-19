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

#include "stdafx.h"
#include "EphemerisEngine.h"


using namespace avSky;

EphemerisEngine::EphemerisEngine( EphemerisData *ephemerisData )
    : _ephemerisData(ephemerisData)
    , _sun(new avSky::Sun())
    , _moon(new avSky::Moon())
{
}

void EphemerisEngine::setLatitude( double latitude )
{
    _ephemerisData->latitude = latitude;
}

void EphemerisEngine::setLongitude( double longitude )
{
    _ephemerisData->longitude = longitude;
}


bool EphemerisEngine::setLatitudeLongitude( double latitude, double longitude )
{
	return setLatitudeLongitudeAltitude( latitude, longitude, 0.0 );
}

bool EphemerisEngine::setLatitudeLongitudeAltitude( double latitude, double longitude, double altitude )
{
    if ( _ephemerisData->latitude != latitude ||
         _ephemerisData->longitude != longitude ||
         _ephemerisData->altitude  != altitude )
    {
        _ephemerisData->latitude  = latitude;
        _ephemerisData->longitude = longitude;
        _ephemerisData->altitude  = altitude;
        return true;
    }

    return false;
}

bool EphemerisEngine::setTurbidityTemperature( float turbidity, float temperature )
{
    if ( _ephemerisData->turbidity   != turbidity ||
         _ephemerisData->temperature != temperature )
    {
        _ephemerisData->turbidity   = turbidity;
        _ephemerisData->temperature = temperature;
        return true;
    }

    return false;
}


bool EphemerisEngine::setDateTime( const DateTime &dateTime )
{
    if ( _ephemerisData->dateTime.getYear() != dateTime.getYear() ||
         _ephemerisData->dateTime.getMonth() != dateTime.getMonth() ||
         _ephemerisData->dateTime.getDayOfMonth() != dateTime.getDayOfMonth() ||
         _ephemerisData->dateTime.getHour() != dateTime.getHour() ||
         _ephemerisData->dateTime.getMinute() != dateTime.getMinute() )
    {
        _ephemerisData->dateTime = dateTime;
        return true;
    }

	return false;
}

void  EphemerisEngine::_updateData( 
        const EphemerisData &ephemData, 
        const CelestialBody &cb, 
        double rsn,
        double & cbAzimuth, double & cbAlt )
{
    _RADecElevToAzimAlt(
            cb.getRightAscension(),
            cb.getDeclination(),
            ephemData.latitude,
            ephemData.localSiderealTime,
            ephemData.altitude,
            rsn,
            cbAzimuth,
            cbAlt );
}

void EphemerisEngine::update( )
{
    double dbOldJulianDate = _ephemerisData->modifiedJulianDate;
    double dbOldSiderealTime = _ephemerisData->localSiderealTime;

    _ephemerisData->modifiedJulianDate = _ephemerisData->dateTime.getModifiedJulianDate();

    double rsn, lsn;
    _getLsnRsn( _ephemerisData->modifiedJulianDate, lsn, rsn );

    _ephemerisData->localSiderealTime = getLocalSiderealTimePrecise( _ephemerisData->modifiedJulianDate, osg::RadiansToDegrees(_ephemerisData->longitude) );

    if ( dbOldJulianDate != _ephemerisData->modifiedJulianDate ||
         dbOldSiderealTime != _ephemerisData->localSiderealTime )
    {
        _sun->updatePosition( _ephemerisData->modifiedJulianDate );
        _updateData( *_ephemerisData, *_sun.get(), rsn, _ephemerisData->sunAzimuth, _ephemerisData->sunAltitude );

        _moon->updatePosition( _ephemerisData->modifiedJulianDate, _ephemerisData->localSiderealTime, _ephemerisData->latitude, _sun.get() );
        _updateData( *_ephemerisData, *_moon.get(), rsn, _ephemerisData->moonAzimuth, _ephemerisData->moonAltitude );
    }
}

/* given the modified JD, mjd, return the true geocentric ecliptic longitude
*   of the sun for the mean equinox of the date, *lsn, in radians, and the
*   sun-earth distance, *rsn, in AU. (the true ecliptic latitude is never more
*   than 1.2 arc seconds and so may be taken to be a constant 0.)
* if the APPARENT ecliptic longitude is required, correct the longitude for
*   nutation to the true equinox of date and for aberration (light travel time,
*   approximately  -9.27e7/186000/(3600*24*365)*2*pi = -9.93e-5 radians).
*/
void EphemerisEngine::_getLsnRsn( double mjd, double &lsn, double &rsn)
{
   double t, t2;
   double ls, ms;    /* mean longitude and mean anomaly */
   double s, nu, ea; /* eccentricity, true anomaly, eccentric anomaly */
   double a, b, a1, b1, c1, d1, e1, h1, dl, dr;

#define range(x, r)     ((x) -= (r)*floor((x)/(r)))

   t = mjd/36525.;
   t2 = t*t;
   a = 100.0021359 * t;
   b = 360. * ( a - (int)a );
   ls = 279.69668f + (.0003025 * t2) + b;
   a = 99.99736042000039 * t;
   b = 360. * ( a - (int)a);
   ms = 358.47583 - (.00015+.0000033 * t * t2) +b;
   s = .016751 - (.0000418*t) - (1.26e-07 * t2);

   _getAnomaly(osg::DegreesToRadians(ms), s, nu, ea);

   a = 62.55209472000015 * t;
   b = 360. * (a - (int)a);
   a1 = osg::DegreesToRadians(153.23 + b);

   a = 125.1041894 * t;
   b = 360*(a - (int)a);
   b1 = osg::DegreesToRadians(216.57 + b);

   a = 91.56766028 * t;
   b = 360 * (a - (int)a);
   c1 = osg::DegreesToRadians(312.69 + b);

   a = 1236.853095*t;
   b = 360*(a-(int)a);
   d1 = osg::DegreesToRadians(350.74 - (.00144 * t2) + b);
   e1 = osg::DegreesToRadians(231.19 + (20.2*t));

   a = 183.1353208 * t;
   b = 360 * (a - (int)a);
   h1 = osg::DegreesToRadians(353.4 + b);
   dl = .00134 * (cos(a1) + .00154) * (cos(b1) + .002) * cos(c1) + (.00179 * sin(d1)) + (.00178 * sin(e1));
   dr = (5.43e-06 * sin(a1)) + (1.575e-05 * sin(b1)) + (1.627e-05 * sin(c1)) + (3.076e-05 * cos(d1)) + (9.27e-06 * sin(h1));
   lsn = nu + osg::DegreesToRadians(ls - ms + dl);
   range(lsn, 2.0*osg::PI);
   rsn = 1.0000002 * (1 - s * cos(ea)) + dr;
}

/* given the mean anomaly, ma, and the eccentricity, s, of elliptical motion,
* find the true anomaly, *nu, and the eccentric anomaly, *ea.
* all angles in radians.
*/
void EphemerisEngine::_getAnomaly( double ma, double s, double &nu, double &ea)
{
    double m, fea;

    m = ma-(2.f*osg::PI)*(int)(ma/(2.f*osg::PI));

    if (m > osg::PI) 
        m -= (2.0*osg::PI);

    if (m < -osg::PI) 
        m += (2.0*osg::PI);

    fea = m;

    if (s < 1.0) 
    {
      /* elliptical */
        double dla;
        for (;;) 
        {
            dla = fea-(s*sin(fea))-m;
            if (fabs(dla)<1e-6)
                break;
            dla /= 1-(s*cos(fea));
            fea -= dla;
        }
        nu = 2*atan((double)(sqrt((1+s)/(1-s))*tan(fea/2)));
    } 
    else 
    {
      /* hyperbolic */
        double corr = 1;
        while (fabs(corr) > 0.000001) 
        {
            corr = (m - s * sinh(fea) + fea) / (s*cosh(fea) - 1);
            fea += corr;
        }
        nu = 2*atan((double)(sqrt((s+1)/(s-1))*tanh(fea/2)));
   }
   ea = fea;
}

double EphemerisEngine::getLocalSiderealTimePrecise( double mjd, double lng )
{
    static const double MJD0    = 2415020.0;
    static const double J2000   = 2451545.0 - MJD0;
    static const double SIDRATE = 0.9972695677;

    double day = floor(mjd - 0.5) + 0.5;
    double hr = (mjd - day) * 24.0;
    double T, x;

    T = ((int)(mjd - 0.5) + 0.5 - J2000)/36525.0;
    x = 24110.54841 + (8640184.812866 + (0.093104 - 6.2e-6 * T) * T) * T;
    x /= 3600.0;
    double gst = (1.0/SIDRATE) * hr + x;

    /// NOTE THE SIGN CHANGE IN THE NEXT LINE
    // 
    double lst = gst + lng/15.0; 
    //               ^
    // here ---------|

    lst -= 24.0 * floor( lst / 24.0 );

    return lst;
}

void EphemerisEngine::_RADecElevToAzimAlt( 
        double rightAscension,
        double declination,
        double latitude,
        double localSiderealTime,
        double elevation, // In meters above sea level
        double rsn,
        double &azim,
        double &alt )
{
    static const double meanRadiusOfEarthInMeters = 6378160.0;
    range(rightAscension, 2*osg::PI );
    double ha = (osg::DegreesToRadians(localSiderealTime) * 15.0) - rightAscension;
    double elev = elevation/meanRadiusOfEarthInMeters;

    static const double ddes = (2.0 * 6378.0 / 146.0e6);
    double ehp = ddes/rsn;

    double aha; // Apparent HA
    double adec;// Apparent declination
    _calcParallax(ha, declination, latitude, elev, ehp, aha, adec);
    _aaha_aux( latitude, aha, adec, &azim, &alt );
}

/* given true ha and dec, tha and tdec, the geographical latitude, phi, the
* height above sea-level (as a fraction of the earths radius, 6378.16km),
* ht, and the equatorial horizontal parallax, ehp, find the apparent
* ha and dec, aha and adec allowing for parallax.
* all angles in radians. ehp is the angle subtended at the body by the
* earth's equator.
*/
void EphemerisEngine::_calcParallax ( 
                    double tha, double tdec,    // True right ascension and declination
                    double phi, double ht,      // geographical latitude, height abouve sealevel
                    double ehp,                 // Equatorial horizontal parallax
                    double &aha, double &adec)  // output: aparent right ascencion and declination
{
    double cphi = cos(phi);
    double sphi = sin(phi);
    double u = atan(9.96647e-1*sphi/cphi);
    double rsp = (9.96647e-1*sin(u))+(ht*sphi);
    double rcp = cos(u)+(ht*cphi);

   double rp = 1/sin(ehp);  /* distance to object in Earth radii */
   double ctha = cos(tha);
   double stdec = sin(tdec);
   double ctdec = cos(tdec);
   double tdtha = (rcp*sin(tha))/((rp*ctdec)-(rcp*ctha));
   double dtha = atan(tdtha);

   aha = tha+dtha;
   double caha = cos(aha);
   range(aha, 2*osg::PI);
   adec = atan(caha*(rp*stdec-rsp)/(rp*ctdec*ctha-rcp));
}

/* the actual formula is the same for both transformation directions so
* do it here once for each way.
* N.B. all arguments are in radians.
//   lat = latitude, x = aparent right ascension, y = Declination, p = azimuth, q = altitude
*/
void EphemerisEngine::_aaha_aux (double lat, double x, double y, double *p, double *q)
{
   double sinlat = sin (lat);
   double coslat = cos (lat);

   double sy = sin (y);
   double cy = cos (y);
   double sx = sin (x);
   double cx = cos (x);

#define EPS (1e-20)
   double sq = (sy*sinlat) + (cy*coslat*cx);
   *q = asin (sq);
   double cq = cos (*q);
   double a = coslat*cq;
   if (a > -EPS && a < EPS)
      a = a < 0 ? -EPS : EPS; /* avoid / 0 */
   double cp = (sy - (sinlat*sq))/a;
   if (cp >= 1.0)   /* the /a can be slightly > 1 */
      *p = 0.0;
   else if (cp <= -1.0)
      *p = osg::PI;
   else
      *p = acos ((sy - (sinlat*sq))/a);
   if (sx>0) *p = 2.0*osg::PI - *p;
}
