#pragma once

namespace avSky
{

// Sky Dome is overall sky around us
class SkyDome : public osg::Geode
{
public:

    SkyDome();

public:

    void setSunPos( float azimuth, float altitude );
    void setTurbidity( float t );
    void setTemperature( float t );

    void setCloudsTexture(osg::Texture2D * pCloudsTex);
    void setCloudsIntensity(float intensity);

public:

    void updateSkyTexture();

public:

    const osg::Vec3f & getAmbient()  const { return _globalAmbient; }
    const osg::Vec3f & getDiffuse()  const { return _globalDiffuse; }
    const osg::Vec3f & getSpecular() const { return _globalSpecular; }

    const osg::Vec3f & getSunDir()   const { return _sunDirection; }

    const float & getIllumination() const { return _globalIllumination; }

private:

    static const unsigned _skyDomeTextureWidth;
    static const unsigned _skyDomeTextureHeight;

    float _sunAzimuth;
    float _sunAltitude;

    //
    // illumination data
    //

    osg::Vec3f _sunDirection;

    osg::Vec3f _globalAmbient;
    osg::Vec3f _globalDiffuse;
    osg::Vec3f _globalSpecular;

    float _globalIllumination;

    //
    // rendering related info
    //

    static const int _skyTextureUnit = 0;
    static const int _cloudsTextureUnit = 1;
    static const int _sunRaysTextureUnit = 2;
    static const int _sunDiscTextureUnit = 3;

    // create state set
    void _buildStateSet();

    // sky color texture
    osg::ref_ptr<osg::Texture2D> _skyTexture;

    // sun related 
    osg::ref_ptr<osg::Uniform> _sunColorUniform;
    osg::ref_ptr<osg::Uniform> _sunDiscSizeUniform;
    osg::ref_ptr<osg::Uniform> _sunRaysUniform;
    osg::ref_ptr<osg::Uniform> _sunMatrixUniform;

    // clouds related
    osg::ref_ptr<osg::Uniform> _cloudsDensityUniform;

    // create geometry
    void _createGeometry();

    //-- support for the sky temperature
    float _temperature;

    //
    // Some sky luminance model used. This model is castrated a bit and some functionality added.
    //

    // Turbitity
    // The fraction of scattering due to haze rather than molecules
    // 2 = quite a clear day
    // 20 = hazy day
    // Reasonable values are 1 - 60
    float _turbidity;

    // How overall sky brightness varies with altitude
    float _light_due_to_alt;

    // Terry's RGB coefficients
    // A = horizon intensity
    // B = horizon falloff
    // C = circumsolar intensity
    // D = circumsolar falloff
    // E = overall (linear) component
    float _Drgb;
    float _Ar, _Br, _Cr, _Er;
    float _Ag, _Bg, _Cg, _Eg;
    float _Ab, _Bb, _Cb, _Eb;
    float _horiz_atten_g, _solar_atten_g;
    float _horiz_atten_b, _solar_atten_b;

    // recalculate these above coefficients
    void _updateDistributionCoefficients();
};

}


