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
                if(_intensivity > 1.0)
                    _intensivity = 1.0;
                else
                if ( _f_fog_changer )
                { 
                    _f_fog_changer(osg::Vec4f(_color.x(),_color.y(),_color.z(),_intensivity));
                }
                return true;
            } else
                if (ea.getKey()== osgGA::GUIEventAdapter::KEY_Leftbracket)
                { 
                    _intensivity -= 0.1;
                    if(_intensivity < 0.0) 
                        _intensivity = 0.0;
                    else
                    if ( _f_fog_changer )
                    { 
                        _f_fog_changer(osg::Vec4f(_color.x(),_color.y(),_color.z(),_intensivity));
                    }
                    return true;
                }

        }

        return false;
    }

    void setFogColor(osg::Vec3f color)
    {
       _color =  color;
       if ( _f_fog_changer )  _f_fog_changer(osg::Vec4f(_color.x(),_color.y(),_color.z(),_intensivity));
    }
    

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("]",       "+ fog");
        usage.addKeyboardMouseBinding("[",       "- fog");

    }

private:

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

    // get fog density
    float getFogDensity() const { return m_fogDensity; }

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
    float m_fogDensity;

    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();
};

