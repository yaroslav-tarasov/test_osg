#pragma once


namespace avSky
{

//
// Stars sphere
//

// Star field class
class StarField : public osg::MatrixTransform
{
public:

    StarField();

    void setIlluminationFog( float fIllum, float fFog );

protected:

    // star loaded data
    struct StarData
    {
        std::string name;

        float right_ascension;
        float declination;

        float magnitude;

        StarData( std::stringstream &ss );
    };
    // all stars array
    std::vector<StarData> m_aStars;

    // stars geometric data
    osg::ref_ptr<osg::Geode> m_geodeStars;
    osg::ref_ptr<osg::Uniform> m_starsUniform;

    // build geode and state-set
    void _buildGeometry();
};

}
