#include "stdafx.h"
#include "find_node_visitor.h" 
#include "info_visitor.h"
#include "find_tex_visitor.h"
#include "creators.h"
#include "shaders.h"

namespace {
osg::Image* createSpotLight( const osg::Vec4& centerColor, const osg::Vec4& bgColor, unsigned int size, float power )
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( size, size, 1, GL_RGBA, GL_UNSIGNED_BYTE );

    float mid = (float(size)-1) * 0.5f;
    float div = 2.0f / float(size);
    for( unsigned int r=0; r<size; ++r )
    {
        unsigned char* ptr = image->data(0, r, 0);
        for( unsigned int c=0; c<size; ++c )
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float r = powf(1.0f - sqrtf(dx*dx+dy*dy), power);
            if ( r<0.0f ) r = 0.0f;

            osg::Vec4 color = centerColor*r + bgColor*(1.0f - r);
            *ptr++ = (unsigned char)((color[0]) * 255.0f);
            *ptr++ = (unsigned char)((color[1]) * 255.0f);
            *ptr++ = (unsigned char)((color[2]) * 255.0f);
            *ptr++ = (unsigned char)((color[3]) * 255.0f);
        }
    }
    return image.release();
}


class KeyHandler : public osgGA::GUIEventHandler
{

public:
    typedef std::function<void()> do_smthng_f;
public:
    KeyHandler(osg::Node * root,osg::Texture *texture,do_smthng_f dsf) 
      :_texture    (texture)
      ,_root       (root)
      ,_dsf        (dsf) 
      {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {            
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add || ea.getKey()== osgGA::GUIEventAdapter::KEY_KP_Subtract)
            { 
                if ( _root )
                { 
                    ReplaceTextureVisitor ftv( _texture.get() );
                    _root->accept( ftv );
                    _texture =  ftv.getOldTexture();
                }
                return true;
            }
            else
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F5 )
            { 
                if ( _dsf )
                { 
                   _dsf();
                   _dsf = nullptr;
                }
                return true;
            }
        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Change texture");
        usage.addKeyboardMouseBinding("Keypad -",       "Change texture");

    }

private:
    osg::ref_ptr<osg::Texture> _texture;
    osg::ref_ptr<osg::Node>    _root;
    do_smthng_f                _dsf;
};

class SineAnimation: public osg::Uniform::Callback
{
public:
    SineAnimation( float rate = 1.0f, float scale = 1.0f, float offset = 0.0f ) :
      _rate(rate), _scale(scale), _offset(offset)
      {}

      void operator()( osg::Uniform* uniform, osg::NodeVisitor* nv )
      {
          float angle = _rate * nv->getFrameStamp()->getSimulationTime();
          float value = sinf( angle ) * _scale + _offset;
          uniform->set( value );
      }

private:
    const float _rate;
    const float _scale;
    const float _offset;
};

}

namespace exp_mat 
{
    const char* vs = {  
        "#version 330 compatibility \n"
        "uniform mat4 osg_ModelViewMatrix; \n"
        "uniform mat4 osg_ModelViewProjectionMatrix; \n"
        "in vec4 osg_Vertex; \n"
        STRINGIFY (

        void main( )
        {   

            gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
        }      
        )
    };

    const char* gs = {  
        "#version 330 compatibility \n"
        "#extension GL_EXT_geometry_shader4: enable \n"
        "#extension GL_EXT_gpu_shader4: enable \n"
        "uniform mat4 osg_ProjectionMatrix; \n"    
        "uniform mat4 osg_ModelViewProjectionMatrix; \n"
        
        "uniform float u_anim1;\n"

        STRINGIFY ( 

        layout( triangles )  in;
        layout( points, max_vertices=200 )  out;

        uniform int   uLevel;
        uniform float uGravity;
        uniform float uTime;
        uniform float uVelScale;

        out float gLightIntensity;

        const vec3 LIGHTPOS = vec3( 0., 0., 10. );

        vec3	V0;
        vec3    V01;
        vec3    V02;
        vec3	CG;
        vec3	Normal;

        void ProduceVertex( float s, float t )
        {
            vec3 v = V0 + s*V01 + t*V02;
            gLightIntensity  = abs( dot( normalize(LIGHTPOS - v), Normal ) );
            vec3 vel = uVelScale * ( v - CG );
            v = v + vel*uTime + 0.5*vec3(0.,uGravity,0.)*uTime*uTime;
            gl_Position = /*gl_ProjectionMatrix * */ vec4( v, 1. );
            EmitVertex( );
        }

        void  main( )
        {
            V01 = ( gl_PositionIn[1] - gl_PositionIn[0] ).xyz;
            V02 = ( gl_PositionIn[2] - gl_PositionIn[0] ).xyz;

            Normal = normalize( cross( V01, V02 ) );

            V0  =   gl_PositionIn[0].xyz;
            CG = ( gl_PositionIn[0].xyz + gl_PositionIn[1].xyz + gl_PositionIn[2].xyz ) / 3.;

            int numLayers = 1 << uLevel;

            float dt = 1. / float( numLayers );
            float t = 1.;

            for( int it = 0; it <= numLayers; it++ )
            {
                float smax = 1. - t;
                int nums = it + 1;
                float ds = smax / float( nums - 1 );
                float s = 0.;

                for( int is = 0; is < nums; is++ )
                {
                    ProduceVertex( s, t );
                    s += ds;
                }

                t -= dt;
            } 
                //vec4 v = gl_PositionIn[0];
               
                //gl_Position = v + vec4(u_anim1,0.,0.,0.);  EmitVertex();
                //gl_Position = v - vec4(u_anim1,0.,0.,0.);  EmitVertex();
                //EndPrimitive();
                //
                //gl_Position = v + vec4(0.,1.0-u_anim1,0.,0.);  EmitVertex();
                //gl_Position = v - vec4(0.,1.0-u_anim1,0.,0.);  EmitVertex();
                //EndPrimitive();
        }      
        )

    };

    static const char* geomSource = {
        "#version 430 \n"
        "#extension GL_EXT_geometry_shader4 : enable\n"
        "#extension GL_EXT_gpu_shader4: enable \n"
        "layout( triangles )  in;  \n"
        "layout( points, max_vertices=200 )  out;  \n"
        "uniform float u_anim1;\n"
        "varying in vec4 v_color[];\n"
        "varying out vec4 v_color_out;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 v = gl_PositionIn[0];\n"
        "    v_color_out = v + v_color[0];\n"
        "\n"
        "    gl_Position = v + vec4(u_anim1,0.,0.,0.);  EmitVertex();\n"
        "    gl_Position = v - vec4(u_anim1,0.,0.,0.);  EmitVertex();\n"
        "    EndPrimitive();\n"
        "\n"
        "    gl_Position = v + vec4(0.,1.0-u_anim1,0.,0.);  EmitVertex();\n"
        "    gl_Position = v - vec4(0.,1.0-u_anim1,0.,0.);  EmitVertex();\n"
        "    EndPrimitive();\n"
        "}\n"
    };

    const char* fs = {
        "#version 330 compatibility  \n"

        STRINGIFY ( 

        in float gLightIntensity;

        layout(location=0) out vec4 fFragColor;

        const vec3 COLOR = vec3( 0., 1., 0. );

        void  main( )
        {
            fFragColor = vec4( /*gLightIntensity* */COLOR, 1.  );
        }

        )

    };   

    const char* get_shader(shaders::shader_t t)
    {
        if(t==shaders::VS)
            return vs;
        else if(t==shaders::FS)
            return fs;
        else if(t==shaders::GS)
            return gs;
        else 
            return nullptr;
    }

}  // ns exp_mat

int main_exp_test( int argc, char** argv )
{

    osgViewer::Viewer viewer;

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(osgDB::readImageFile("a_319_aeroflot.png"/*"Fieldstone.jpg"*/));
    
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("dino3.obj"); 

#if 0
    creators::nodes_array_t plane = creators::loadAirplaneParts();
    auto model = plane[1];
    
    InfoVisitor infoVisitor;
    model->accept( infoVisitor );

    auto a_node =  findFirstNode(model,"Body_",findNodeVisitor::not_exact);
    
    findNodeByType<osg::Geode> findGeode; 
    a_node->accept(findGeode);
    auto g_node =  findGeode.getFirst();
#endif

    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum( 0 );
    light->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    light->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    light->setPosition( osg::Vec4(100.0f, 10.0f, 10.0f, 1.0f) );

    osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
    source->setLight( light ); 
 
#if 0
    osg::ref_ptr<osgFX::BumpMapping> effet = new osgFX::BumpMapping();
    effet->setLightNumber(0);
    effet->setOverrideDiffuseTexture(texture.get());
    effet->setOverrideNormalMapTexture(new osg::Texture2D(osgDB::readImageFile("a_319_n.png")));
    effet->addChild(airplane);
    // effet->prepareChildren();
    effet->setEnabled(false);
#endif
    
    osg::StateSet * pCommonStateSet = model->getOrCreateStateSet();
    //pCommonStateSet->setNestRenderBins(false);
    //pCommonStateSet->setRenderBinDetails(/*RENDER_BIN_SOLID_MODELS*/100, "RenderBin");

    // effet->setUpDemo();
    // effet->setEnabled(false);
    root->addChild(source);
    root->addChild(/*effet*/model);
    
    pCommonStateSet->addUniform( new osg::Uniform("uLevel", 1));                        // uniform int uLevel;
    pCommonStateSet->addUniform( new osg::Uniform("uGravity", -0.5 ));                     // uniform float uGravity;
    pCommonStateSet->addUniform( new osg::Uniform("uTime", 1.));                        // uniform float uTime;
    pCommonStateSet->addUniform( new osg::Uniform("uVelScale", 1. ));                    // uniform float uVelScale;
    //effects::createProgram(pCommonStateSet,exp_mat::vs,exp_mat::fs,exp_mat::gs);
    
    // a generic cyclic animation value
    osg::Uniform* u_anim1( new osg::Uniform( "u_anim1", 0.0f ) );
    u_anim1->setUpdateCallback( new SineAnimation( 4, 0.5, 0.5 ) );
    pCommonStateSet->addUniform( u_anim1 );
    
    // osgDB::writeNodeFile(*root,"tex_test_blank.osgt");
	
	// Set the clear color to black
    //viewer.getCamera()->setClearColor(osg::Vec4(1.0,0,0,1));

    viewer.addEventHandler( new KeyHandler( root.get(), texture.get(),[&](){effects::createProgram(pCommonStateSet,exp_mat::vs,exp_mat::fs,exp_mat::gs);} ) );
    
    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}