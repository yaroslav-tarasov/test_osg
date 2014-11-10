#pragma once


class FogHandler : public osgGA::GUIEventHandler
{
public:
    typedef std::function<void(osg::Vec4f)> on_fog_change_f;
public:
    FogHandler(const on_fog_change_f& f_fog_changer,osg::Vec3f color) 
        :_f_fog_changer    (f_fog_changer)
        , _intensivity(0.1)
        , _color(color)
    {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {            
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Rightbracket )
            { 
                _intensivity += 0.1;
                if ( _f_fog_changer )
                { 
                    _f_fog_changer(osg::Vec4f(1.0,1.0,1.0,_intensivity));
                }
                return true;
            } else
                if (ea.getKey()== osgGA::GUIEventAdapter::KEY_Leftbracket)
                { 
                    _intensivity -= 0.1;
                    if ( _f_fog_changer )
                    { 
                        _f_fog_changer(osg::Vec4f(1.0,1.0,1.0,_intensivity));
                    }
                    return true;
                }

        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("]",       "+ fog");
        usage.addKeyboardMouseBinding("[",       "- fog");

    }

    on_fog_change_f _f_fog_changer;
    float           _intensivity;
    osg::Vec3f      _color;


};

//
// Post-applied fog sky dome layer
//

class FogLayer : public osg::Geode
{
public:

    // constructor
    FogLayer(osg::Group * sceneRoot);

    // set fog parameters
    void setFogParams( const osg::Vec3f & vFogColor, float fFogDensity );

    // get real evaluated visible distance
    float getVisDistance() const { return m_fRealVisDist; }

    // get fog coefficient
    float getFogExp2Coef() const { return m_realExp2Density; }

private:

    // scene ptr
    osg::Group * _pScenePtr;


    // fog related global uniforms
    osg::ref_ptr<osg::Uniform> _sceneFogUniform;

    // current uniforms
    osg::ref_ptr<osg::Uniform> _skyFogUniform;

    // real vis distance
    float m_fRealVisDist;
    float m_realExp2Density;

    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();
};

