#pragma once


namespace avSky
{

struct  ISky 
{
    virtual float                   GetSunIntensity() const = 0;
    virtual const osg::Vec3f &      GetFogColor    () const = 0;
};

}