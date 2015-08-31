#include "stdafx.h"

// #include <math/sincos.h>

#include "utils/lut.h"

#include "SkyDome.h"

#include "av/Utils.h"
#include "av/Environment.h"
//
// Global namespaces
//

using namespace avSky;

//
// Module constants
//

// numbers below are chosen to be as minimal as possible
const unsigned SkyDome::_skyDomeTextureWidth = 64;
const unsigned SkyDome::_skyDomeTextureHeight = 32;

//
// Clouds density uniform update
//
class CloudsDensityUniform : public osg::Uniform::Callback
{
public:
    CloudsDensityUniform( SkyDome* pSkyDome ) : m_pSkyDome( pSkyDome ) {}

private:
    SkyDome* m_pSkyDome;

    virtual void operator()( osg::Uniform* uniform, osg::NodeVisitor* /*nv*/ )
    {
      
        float fDensity = avCore::GetEnvironment()->GetWeatherParameters().CloudDensity; 
        uniform->set( osg::Vec3f( cg::bound( fDensity * 2.0f, 0.0f, 1.0f ),                         // Clouds opacity
                                  cg::lerp01( 1.0f, 0.9f,fDensity ) * m_pSkyDome->getIllumination(), // Clouds color darkness
                                  cg::lerp01( 1.0f, 0.9f,fDensity ) ) );                             // Sky texture color darkness
    }
};

//
// Sky Dome object class
//

// constructor
SkyDome::SkyDome()
    : _turbidity(2.0f)
    , _temperature(0.0)
    , _sunAzimuth(-1.0)
    , _sunAltitude(-1.0)
{
     setNodeMask( DO_NOT_PICK_NODE_MASK );
    
    _updateDistributionCoefficients();

    _buildStateSet();

    _createGeometry();
}

// set sun position
void SkyDome::setSunPos( float azimuth, float altitude )
{
    if (!cg::eq(_sunAzimuth, azimuth) || !cg::eq(_sunAltitude, altitude))
    {
        _sunAzimuth = azimuth;
        _sunAltitude = altitude;

        // level when sky becomes totally black
        static const float dark_alt = osg::DegreesToRadians(-15.0f);

        // darkness level value
        _light_due_to_alt = osg::DegreesToRadians(altitude);
        if (_light_due_to_alt < dark_alt)
            _light_due_to_alt = 0.0f;
        else
        {
            // remap {dark_alt, pi/3} to {-pi/2, pi/2}
            // because we assume pi/3 (60 degrees) as maximum sun elevation point with maximum intensity
            _light_due_to_alt = cg::lerp_clamp<float, float>(_light_due_to_alt, dark_alt, 0.333f * osg::PI, -osg::PI_2, osg::PI_2);
            // sine of this new angle remapped to {0, 1}
            _light_due_to_alt = sinf(_light_due_to_alt) * 0.5f + 0.5f;
            // brighten it
            static const float day_exp = 0.35f;
            _light_due_to_alt = powf(_light_due_to_alt, day_exp);
            // and make max as 0.65
            _light_due_to_alt *= 0.65f;
        }
    }
}

void SkyDome::setTurbidity( float t )
{
    _turbidity = cg::bound(t, 1.0f, 40.0f);
    _updateDistributionCoefficients();
}

void SkyDome::setTemperature( float t )
{
    _temperature = t;
}

void SkyDome::_buildStateSet()
{
    osg::StateSet * sset = getOrCreateStateSet();

    // Set render bin
    sset->setRenderBinDetails(RENDER_BIN_SKYDOME, "RenderBin");
    sset->setNestRenderBins(false);

    // Sky Texture unit
    {
        const unsigned dataSize = _skyDomeTextureWidth * _skyDomeTextureHeight * 3 * sizeof(float);
        unsigned char * data = new unsigned char[dataSize];
        memset(data, 0, dataSize);

        osg::Image * skyImage = new osg::Image();
        skyImage->setDataVariance(osg::Image::DYNAMIC);
        skyImage->setImage(_skyDomeTextureWidth, _skyDomeTextureHeight, 0,
            GL_RGB32F_ARB, GL_RGB,
            GL_FLOAT, data, osg::Image::USE_NEW_DELETE);

        _skyTexture = new osg::Texture2D();
        _skyTexture->setImage(skyImage);
        _skyTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        _skyTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
        _skyTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
        _skyTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

        sset->setTextureAttribute(_skyTextureUnit, _skyTexture.get());
    }

    // Create sky dome shader
    osg::ref_ptr<osg::Program> cSkydomeProg = new osg::Program();
    cSkydomeProg->setName("SkyDomeShader");
    cSkydomeProg->addShader(/*utils::GetDatabase()->LoadShader("sky.vs")*/
        osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("sky.vs")));
    cSkydomeProg->addShader(/*utils::GetDatabase()->LoadShader("sky.fs")*/
        osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("sky.fs")));
    sset->setAttribute(cSkydomeProg.get());

    // add textures uniforms
    sset->addUniform(new osg::Uniform("SkyTexture", _skyTextureUnit));
    sset->addUniform(new osg::Uniform("SkyCloudsTexture", _cloudsTextureUnit));

    // sun disc uniforms
    _sunColorUniform = new osg::Uniform("SunColor", osg::Vec3f());
    sset->addUniform(_sunColorUniform.get());
    _sunDiscSizeUniform = new osg::Uniform("SunRadii", osg::Vec3f());
    sset->addUniform(_sunDiscSizeUniform.get());
    _sunRaysUniform = new osg::Uniform("RaysRadiiFade", osg::Vec4f());
    sset->addUniform(_sunRaysUniform.get());
    // sun matrix uniform
    _sunMatrixUniform = new osg::Uniform("SunMatrix", osg::Matrix3());
    sset->addUniform(_sunMatrixUniform.get());

    // sun rays
    _sunRaysUniform = new osg::Uniform("RaysRadiiFade", osg::Vec4f());
    sset->addUniform(_sunRaysUniform.get());

    // load sun textures
    utils::SetTextureUniform("Sky/Sun_a.dds", "SunDiscTexture", _sunDiscTextureUnit, sset, osg::Texture::CLAMP_TO_EDGE);
    utils::SetTextureUniform("Sky/SunRays_a.dds", "SunRaysTexture", _sunRaysTextureUnit, sset, osg::Texture::CLAMP_TO_EDGE);

    // clouds intensity uniform
    _cloudsDensityUniform = new osg::Uniform("CloudsDensity", osg::Vec3f());
    _cloudsDensityUniform->setUpdateCallback( new CloudsDensityUniform( this ) );
    sset->addUniform(_cloudsDensityUniform.get());
}

void SkyDome::_createGeometry()
{
    // dummy bounding box callback
    osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

    // create OSG geode with 1 drawable node
    setCullingActive(false);
    setDataVariance(osg::Object::DYNAMIC);

    // create tetrahedron around viewer (just ti fill the whole volume)
    osg::Geometry * box_geometry = new osg::Geometry;
    box_geometry->setUseDisplayList(true);
    box_geometry->setDataVariance(osg::Object::STATIC);
    box_geometry->setComputeBoundingBoxCallback(pDummyBBCompute);

    const float fSqrt3 = sqrtf(3.0f);

    // create its' vertex
    osg::Vec3Array * paBoxPointsPos = new osg::Vec3Array;
    paBoxPointsPos->resize(4);
    paBoxPointsPos->at(0).set(0.f, 0.f, +2.f);
    paBoxPointsPos->at(1).set(-2.0f * fSqrt3, 0.f, -1.0f);
    paBoxPointsPos->at(2).set(fSqrt3, -3.0f, -1.0f);
    paBoxPointsPos->at(3).set(fSqrt3, +3.0f, -1.0f);
    // set vertex array
    paBoxPointsPos->setDataVariance(osg::Object::STATIC);
    box_geometry->setVertexArray(paBoxPointsPos);

    // draw elements command, that would be executed
    // volume is made looking inside
    osg::DrawElementsUShort * paBoxDrawElem = new osg::DrawElementsUShort(GL_TRIANGLES, 12);
    paBoxDrawElem->at(0)  = 0;
    paBoxDrawElem->at(1)  = 2;
    paBoxDrawElem->at(2)  = 1;
    paBoxDrawElem->at(3)  = 0;
    paBoxDrawElem->at(4)  = 3;
    paBoxDrawElem->at(5)  = 2;
    paBoxDrawElem->at(6)  = 0;
    paBoxDrawElem->at(7)  = 1;
    paBoxDrawElem->at(8)  = 3;
    paBoxDrawElem->at(9)  = 1;
    paBoxDrawElem->at(10) = 2;
    paBoxDrawElem->at(11) = 3;

    // add DIP
    paBoxDrawElem->setDataVariance(osg::Object::STATIC);
    box_geometry->addPrimitiveSet(paBoxDrawElem);

    // all is done, so add box to this geode
    addDrawable(box_geometry);
}

void SkyDome::_updateDistributionCoefficients()
{
    // for all components
    _Drgb = _turbidity * -0.63333f + 25.0f;

    // red component
    _Ar = _turbidity * 0.00367f + 0.09f;
    _Br = _turbidity * -0.08f + 6.0f;
    _Cr = 0.53f;
    _Er = 0.15f;

    // green component
    _Ag = _turbidity * 0.00367f + 0.12f;
    _Bg = _turbidity * -0.08f + 6.0f;
    _Cg = 0.47f;
    _Eg = 0.25f;
    _horiz_atten_g = powf(0.25f, 1.0f + 0.07f * _turbidity);
    _solar_atten_g = powf(0.50f, 1.0f + 0.02f * _turbidity);

    // blue component
    _Ab = 0.1f;
    _Bb = _turbidity * -0.11f + 8.0f;
    _Cb = 0.5f;
    _Eb = 0.42f;
    _horiz_atten_b = powf(0.35f, 1.0f + 0.07f * _turbidity);
    _solar_atten_b = powf(0.85f, 1.0f + 0.02f * _turbidity);
}

static inline float scale_luminance_func( float luminance )
{
    static const float exposure = 5.0f;
    const float brightness = 1.0f - expf(-exposure * luminance);
    return brightness / (luminance + 0.001f);
}

static inline float acosf_to_1_0( float angle_cos )
{
    static const float piInv = 1.0f / 3.14159265f;
    return 1.0f - piInv * acosf(angle_cos);
}

void SkyDome::updateSkyTexture()
{
    static const float 
        pi = 3.14159265f,
        twoPi = 2.0f * pi,
        halfPi = 0.5f * pi, 
        halfPiInv = 1.0f / halfPi;

    struct PrecomputedInfo
    {
        struct TexelData
        {
            osg::Vec3f dir;
        } texelData[_skyDomeTextureHeight][_skyDomeTextureWidth];

        struct RowData
        {
            float theta, theta_sin, theta_cos, theta_0_1;
        } rowData[_skyDomeTextureHeight];

        struct ColumnData
        {
            float phi, phi_sin, phi_cos;
        } columnData[_skyDomeTextureWidth];

        PrecomputedInfo()
        {
            for (unsigned row = 0; row < _skyDomeTextureHeight; row++)
            {
                RowData & data = rowData[row];
                data.theta = halfPi - ((float(row) + 0.5f) * (halfPi / float(_skyDomeTextureHeight)));
                cg::fsincos(data.theta, &data.theta_sin, &data.theta_cos);
                data.theta_0_1 = (halfPi - data.theta) * halfPiInv;
            }

            for (unsigned column = 0; column < _skyDomeTextureWidth; column++)
            {
                ColumnData & data = columnData[column];
                data.phi = -halfPi - ((float(column) + 0.5f) * (twoPi / float(_skyDomeTextureWidth)));
                cg::fsincos(data.phi, &data.phi_sin, &data.phi_cos);
            }

            for (unsigned row = 0; row < _skyDomeTextureHeight; row++)
            {
                for (unsigned column = 0; column < _skyDomeTextureWidth; column++)
                {
                    texelData[row][column].dir = osg::Vec3(
                        columnData[column].phi_sin * rowData[row].theta_cos, 
                        columnData[column].phi_cos * rowData[row].theta_cos, 
                        rowData[row].theta_sin);
                }
            }
        }
    };

    static PrecomputedInfo precomputedInfo;
    static utils::LookupTable<float, float> acosf_to_1_0_lut(-1.0f, 1.0f, 256, acosf_to_1_0);
    static utils::LookupTable<float, float> scale_luminance_lut(0.0f, 1.0f, 512, scale_luminance_func);

    // desaturation vector
    static const osg::Vec3f vDesaturationColor(0.299f, 0.587f, 0.114f);

    // ambient calculation based on sky-dome luminance integration (for me seems to be the most correct)
    float fAmbientWheightedSum = 0.f, fAmbientWheightTotal = 0.f;
    // diffuse calculation based on sky-dome luminance integration near sun position
    osg::Vec3f vDiffuseWheightedSum;
    float fDiffuseWheightTotal = 0.f;

    osg::Image * image = _skyTexture->getImage();
    if (image != NULL)
    {
        const float phiSun  = static_cast<float>(osg::DegreesToRadians(_sunAzimuth));
        const float thetaSun = static_cast<float>(osg::DegreesToRadians(_sunAltitude));

        cg::point_3f vSunDir = utils::celestialUnitVector(phiSun, thetaSun);
        _sunDirection.set(vSunDir.x, vSunDir.y, vSunDir.z);

        // sunset attenuation factor
        const float sunsetAttenuation = powf(sinf(halfPi - thetaSun), 16.0f);

        // dusk/dawn factor, based on east/west sun position
        // 0 - pure dawn, 1 - pure dusk
        const float duskdawnFactor = sinf(phiSun) * 0.5f + 0.5f;

        // making sky a little bit more "nice&blue" depending on temperature
        // also lessen this hand-made effect when dusk-dawn because we want cool coloring but not gray pictures
        const float temperature_coef = cg::lerp01( _temperature, __max(_temperature, 30.0f),sunsetAttenuation * 0.5f);
        const osg::Vec3f
            vTemperatureMul(1.000f + 0.0032f * temperature_coef,
                            1.000f + 0.0020f * temperature_coef,
                            1.000f + 0.0005f * temperature_coef);

        // get color scale (color delta between sunlight and other sky-dome)
        const osg::Vec3f vGammaScale =
            osg::Vec3f(_Cr,
                       _Cg * (1.0f - _solar_atten_g * sunsetAttenuation * cg::lerp01( 1.f, 0.5f,duskdawnFactor)),
                       _Cb * (1.0f - _solar_atten_b * sunsetAttenuation)) * _light_due_to_alt;

        // go through every texel, updating it's data
        osg::Vec3f * ptr = (osg::Vec3f *)image->data();
        for (unsigned row = 0; row < _skyDomeTextureHeight; row++)
        {
            const float theta_0_1 = precomputedInfo.rowData[row].theta_0_1;
            const float gammaPow = (1.0f - theta_0_1 * 0.9f) * _Drgb;

            // get sky mode vertical layer base color
            const osg::Vec3f vConstColor =
                osg::Vec3f(_Ar * powf(theta_0_1, _Br) + _Er,
                           _Ag * powf(theta_0_1, _Bg) * (1.0f - _horiz_atten_g * sunsetAttenuation) + _Eg,
                           _Ab * powf(theta_0_1, _Bb) * (1.0f - _horiz_atten_b * sunsetAttenuation) + _Eb) * _light_due_to_alt;

            for (unsigned col = 0; col < _skyDomeTextureWidth; col++)
            {
                // gamma = angle between sun and texel
                const float cos_gamma = _sunDirection * precomputedInfo.texelData[row][col].dir;
                // gamma remapped from {0, pi} to {1, 0}
                // gamma_1_0 = 1.0f - piInv * acosf(cos_gamma);
                const float gamma_1_0 = acosf_to_1_0_lut(cos_gamma);
                // gamma_1_0 weighted such that it is larger for texels that are lower in the sky
                // This is used for a more realistic--less circular-circumsolar glow
                const float weighted_gamma_1_0_pow = powf(gamma_1_0, gammaPow);

                // Our home grown sky color math model
                osg::Vec3f & vTexColor = *ptr;
                vTexColor = vConstColor + vGammaScale * weighted_gamma_1_0_pow;

                // tone mapping
                //static const float exposure = 5.0f;
                const float luminance = vTexColor * vDesaturationColor;
                //const float brightness = 1.0f - expf(-luminance * exposure);
                //const float scale = 255.0f * brightness / (luminance + 0.001f);
                const float scale = scale_luminance_lut(luminance);

                // sum luminance for ambient calculation
                const float & fCurLumWeight = precomputedInfo.rowData[row].theta_sin;
                fAmbientWheightedSum += luminance * fCurLumWeight;
                fAmbientWheightTotal += fCurLumWeight;

                // finally apply all the magic coefs to get resulting RGB value
                vTexColor[0] *= scale * vTemperatureMul[0];
                vTexColor[1] *= scale * vTemperatureMul[1];
                vTexColor[2] *= scale * vTemperatureMul[2];

                // shift texel
                ++ptr;

                // save color for diffuse calculation
                if (weighted_gamma_1_0_pow > 0.2f)
                {
                    const float fCurDifWeight = weighted_gamma_1_0_pow * weighted_gamma_1_0_pow;
                    vDiffuseWheightedSum += vTexColor * fCurDifWeight;
                    fDiffuseWheightTotal += fCurDifWeight;
                }
            }
        }

        // texture is updated, notify OSG on it
        image->dirty();

        //
        // Illumination factors evaluation
        //

        // try to calculate some ambient term (base luminance level for ambient term is [0, 0.6])
        float fResultAmbScale = cg::bound(10.0f * fAmbientWheightedSum / fAmbientWheightTotal,0.f,1.f);
        _globalAmbient = vTemperatureMul * (fResultAmbScale * 0.6f);

        // get magic sun color
        osg::Vec3f vSunColor;
        // average it with some empirically found normalization constant
        if (fDiffuseWheightTotal)
            vSunColor = vDiffuseWheightedSum * (1.25f / fDiffuseWheightTotal);
        // make it desaturated a bit
        const float fDesatSun = vSunColor * vDesaturationColor;
        vSunColor = cg::lerp01( osg::Vec3f(fDesatSun, fDesatSun, fDesatSun), vSunColor, sunsetAttenuation);
        // XXX: lots of magic here to make sun look appropriate
        const float fSunRaise = powf(sunsetAttenuation, 2.0f);
        // scale sun color to make it over brighten
        vSunColor *= 1.8f;
        // then lessen down green and blue components depending on sunrise/sunset
        vSunColor[0] = cg::bound(vSunColor[0],0.f,1.f);
        vSunColor[1] = powf(cg::lerp01( 1.0f, cg::lerp01( 0.92f, 0.97f, duskdawnFactor),fSunRaise) * cg::bound(vSunColor[1],0.f,1.f), 2.0f);
        vSunColor[2] = powf(cg::lerp01( 1.0f, 0.87f, fSunRaise) * cg::bound(vSunColor[2],0.f,1.f), 2.5f);

        // try to calculate some diffuse term (base luminance level for diffuse term is [0, 0.8])
        _globalDiffuse = vSunColor;
        // let's additionally fade diffuse when sun is absent
        const float
            fBelowHorizonFactorR = cg::lerp_clamp(osg::RadiansToDegrees(thetaSun), -5.0f, 5.0f, 0.f, 1.f),
            fBelowHorizonFactorG = powf(fBelowHorizonFactorR, 1.2f),
            fBelowHorizonFactorB = powf(fBelowHorizonFactorR, 1.5f);
        _globalDiffuse[0] *= fBelowHorizonFactorR;
        _globalDiffuse[1] *= fBelowHorizonFactorG;
        _globalDiffuse[2] *= fBelowHorizonFactorB;
        // diffuse must be in terms of [0, 0.8]
        _globalDiffuse *= 0.8f;

        // illumination is calculated as saturate(ambient + diffuse * 0.75) to match almost everything 
        const float fDesatIllum = (_globalAmbient + _globalDiffuse * 0.75f) * vDesaturationColor;
        _globalIllumination = cg::bound(fDesatIllum,0.f,1.f);

        // specular is simply desaturated diffuse
        const float fDesatDiffuse = _globalDiffuse * vDesaturationColor;
        _globalSpecular = cg::lerp01( osg::Vec3f(fDesatDiffuse, fDesatDiffuse, fDesatDiffuse), _globalDiffuse, sunsetAttenuation) * 0.8f;

        //
        // sun rendering related calculations
        //

        float phiSunSin, phiSunCos, thetaSunSin, thetaSunCos;
        cg::fsincos(phiSun, &phiSunSin, &phiSunCos);
        cg::fsincos(thetaSun, &thetaSunSin, &thetaSunCos);

        // get direction vectors
        const osg::Vec3f
            d(_sunDirection),
            r(phiSunCos, -phiSunSin, 0.f),
            u(-thetaSunSin * phiSunSin, -thetaSunSin * phiSunCos, thetaSunCos);

        // set direction matrix, which will turn sky vector into sun-oriented vector
        // update uniform
        _sunMatrixUniform->set(osg::Matrix3(
            r[0], u[0], d[0],
            r[1], u[1], d[1],
            r[2], u[2], d[2]));

        // update sun color
        _sunColorUniform->set(vSunColor);

        // update sun radii
        const float fCurSunSize = cg::lerp01(0.007f, 0.009f, powf(thetaSunCos, 4.0f));
        const osg::Vec3f vSunRadii(fCurSunSize, 0.5f / fCurSunSize, 0.5f);
        _sunDiscSizeUniform->set(vSunRadii);

        // update sun rays size and opaqueness
        const float fCurRaysSize = fCurSunSize * 10.f;
        const float fRaysOpaqueness = 0.27f * (1.0f - powf(thetaSunCos, 30.0f));
        const osg::Vec4f vRaysRadii(fCurRaysSize, 0.5f / fCurRaysSize, 0.5f, fRaysOpaqueness);
        _sunRaysUniform->set(vRaysRadii);
    }
}

void SkyDome::setCloudsTexture(osg::Texture2D * pCloudsTex)
{
    if (pCloudsTex)
    {
        pCloudsTex->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        pCloudsTex->setWrap(osg::Texture2D::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        pCloudsTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
        pCloudsTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

        getOrCreateStateSet()->setTextureAttribute(_cloudsTextureUnit, pCloudsTex);
    }
}
