#include "stdafx.h"
#include "creators.h"
#include "CommonFunctions"
#include "find_node_visitor.h"  
#include "animation_handler.h"

static const std::string animationName("Default");

static const char* vertSource = STRINGIFY ( 
    attribute vec3 tangent;
    attribute vec3 binormal;
    varying   vec3 lightDir;
  
    out block
    {
        vec2 texcoord;
        vec3 normal;
        vec3 tangent;
        vec3 binormal;
        vec3 viewpos;
        vec4 shadow_view;
        vec4 lightmap_coord;
    } v_out;

    void main()
    {
        vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
        mat3 rotation = mat3(tangent, binormal, normal);
        vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
        lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
        lightDir = normalize(rotation * normalize(lightDir));
        gl_Position = ftransform();
        gl_TexCoord[0] = gl_MultiTexCoord1;

        v_out.tangent   = tangent;
        v_out.binormal  = binormal;
        v_out.normal    = normal;
        v_out.viewpos   = vertexInEye.xyz;
    }
    );

static const char* fragSource = STRINGIFY ( 
    uniform sampler2D colorTex;
    uniform sampler2D normalTex;
    varying vec3 lightDir;
    void main (void)
    {
        vec4 base = texture2D(colorTex, gl_TexCoord[0].xy);
        vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
        bump = normalize(bump * 2.0 - 1.0);
    
        float lambert = max(dot(bump, lightDir), 0.0);
        if (lambert > 0.0)
        {
            gl_FragColor = base * gl_LightSource[0].diffuse * lambert;
            gl_FragColor += gl_LightSource[0].specular * pow(lambert, 2.0);
        }
        gl_FragColor += gl_LightSource[0].ambient;
    }
    );

static const char* fragSource2 = { "#extension GL_ARB_gpu_shader5 : enable \n "
    STRINGIFY ( 
    
    uniform sampler2D           ViewLightMap;
    uniform sampler2D           Detail;
    uniform samplerCube         Env;
    uniform sampler2DShadow     ShadowSplit0;
    uniform sampler2DShadow     ShadowSplit1;
    uniform sampler2DShadow     ShadowSplit2;
    uniform sampler2D           ViewDecalMap;    
    
    // saturation helper
    float saturate( const in float x )
    {
        return clamp(x, 0.0, 1.0);
    }   

    )

    STRINGIFY ( 

    uniform sampler2D colorTex;
    uniform sampler2D normalTex;
    varying vec3 lightDir;
    
    in block
    {
        vec2 texcoord;
        vec3 normal;
        vec3 tangent;
        vec3 binormal;
        vec3 viewpos;
        vec4 shadow_view;
        vec4 lightmap_coord;
    } f_in;

    mat4 viewworld_matrix;

    void main (void)
    {
        // GET_SHADOW(f_in.viewpos, f_in);
        //#define GET_SHADOW(viewpos, in_frag) 
        float shadow = 1.0; 
        //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
        //if (split_test.x) 
        //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
        //else if (split_test.y) 
        //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
        //else if (split_test.z) 
        //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

        vec4  specular       = gl_LightSource[0].specular;     // FIXME 
        vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
        vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 
        vec4  light_vec_view = vec4(lightDir,1);

        viewworld_matrix = gl_ModelViewMatrixInverse;
        vec4 base = texture2D(colorTex, gl_TexCoord[0].xy);
        vec3 bump = fma(texture2D(normalTex, gl_TexCoord[0].xy).xyz, vec3(2.0), vec3(-1.0));
        //vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
        //bump = normalize(bump * 2.0 - 1.0);
        vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
        vec4  dif_tex_col  = texture2D(colorTex,gl_TexCoord[0].xy, -1.0);
        float glass_factor = 1.0 - dif_tex_col.a;

        // get dist to point and normalized to-eye vector
        float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
        float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
        float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
        vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

        vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
        float normal_world_space_z = dot(view_up_vec, normal);


        float incidence_dot  = dot(to_eye, normal);
        float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
        vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
        vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
        float refl_min       = fma(glass_factor, 0.275, 0.125);
        float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
        float fresnel        = mix(refl_min, 0.97, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.25)); 

        float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
        float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
        vec3  pure_spec_color = specular.rgb * specular_val;
        float spec_compose_fraction = 0.35;


        // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
        vec3 cube_color = textureCube(Env, refl_vec_world).rgb + pure_spec_color;
        //vec3 cube_color = vec3(0.5f,0.5f,0.5f);


        vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
        // GET_LIGHTMAP(f_in.viewpos, f_in);
        // #define GET_LIGHTMAP(viewpos, in_frag) 
        // float height_world_lm      = in_frag.lightmap_coord.z; 
        // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
        // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
        // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

        vec3 lightmap_color = vec3(0.1f,0.1f,0.1f); // FIXME dummy staff

        float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
        non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

        float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
        vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
        vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
        float night_factor = step(ambient.a, 0.35);
        vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
        //ALFA-TEST// gl_FragColor = vec4( glass_factor,0,0,1.0f);
        gl_FragColor = vec4( result,1.0);
       
    }
    )
};

class ComputeTangentVisitor : public osg::NodeVisitor
{
public:
    void apply( osg::Node& node ) { traverse(node); }
    
    void apply( osg::Geode& node )
    {
        for ( unsigned int i=0; i<node.getNumDrawables(); ++i )
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>( node.getDrawable(i) );
            if ( geom ) generateTangentArray( geom );
        }
        traverse( node );
    }
    
    void generateTangentArray( osg::Geometry* geom )
    {
        osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
        tsg->generate( geom, 1 );
        geom->setVertexAttribArray( 6, static_cast<osg::Array*>(tsg->getTangentArray()) );
        geom->setVertexAttribBinding( 6, osg::Geometry::BIND_PER_VERTEX );
        geom->setVertexAttribArray( 7, static_cast<osg::Array*>(tsg->getBinormalArray()) );
        geom->setVertexAttribBinding( 7, osg::Geometry::BIND_PER_VERTEX );
    }
};

osg::Geometry* createOctahedron( const osg::Vec3& center )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(6);
    (*vertices)[0].set( 0.0f, 0.0f, 1.0f); (*vertices)[0] += center;
    (*vertices)[1].set(-0.5f,-0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[2].set( 0.5f,-0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[3].set( 0.5f, 0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[4].set(-0.5f, 0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[5].set( 0.0f, 0.0f,-1.0f); (*vertices)[0] += center;

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES, 24);
    (*indices)[0] = 0; (*indices)[1] = 1; (*indices)[2] = 2;
    (*indices)[3] = 0; (*indices)[4] = 2; (*indices)[5] = 3;
    (*indices)[6] = 0; (*indices)[7] = 3; (*indices)[8] = 4;
    (*indices)[9] = 0; (*indices)[10]= 4; (*indices)[11]= 1;
    (*indices)[12]= 5; (*indices)[13]= 2; (*indices)[14]= 1;
    (*indices)[15]= 5; (*indices)[16]= 3; (*indices)[17]= 2;
    (*indices)[18]= 5; (*indices)[19]= 4; (*indices)[20]= 3;
    (*indices)[21]= 5; (*indices)[22]= 1; (*indices)[23]= 4;
    
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    texcoords->push_back( osg::Vec2(0.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.5f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.5f) );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->addPrimitiveSet( indices.get() );
    geom->setTexCoordArray( 0, texcoords.get() );

    // osgUtil::SmoothingVisitor::smooth( *geom );
    return geom.release();
}

osg::Node* CreateEarth()
{
    const double r_earth = 63.137;

    osg::ShapeDrawable *earth_sd = new osg::ShapeDrawable;
    osg::Sphere* earth_sphere = new osg::Sphere;
    earth_sphere->setName("EarthSphere");
    earth_sphere->setRadius(r_earth);
    earth_sd->setShape(earth_sphere);
    earth_sd->setColor(osg::Vec4(0, 0, 1.0, 1.0));

    osg::Geode* earth_geode = new osg::Geode;
    earth_geode->setName("EarthGeode");
    earth_geode->addDrawable(earth_sd);
     
    osg::ref_ptr<osg::Geode> oct = new osg::Geode;
    oct->addDrawable( createOctahedron(osg::Vec3()) );

    osg::Group* root = new osg::Group;
    //root->addChild(earth_geode);
    //root->addChild(osgDB::readNodeFile("cessna2.osgt")); 
    root->addChild(oct);
    return root ;
}

class LightChangeHandler : public osgGA::GUIEventHandler
{
public:
    LightChangeHandler(osg::LightSource *ls) : ls_(ls) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add)
            {
                auto a_ = ls_->getLight()->getAmbient();
                auto d_ = ls_->getLight()->getDiffuse();
                auto s_ = ls_->getLight()->getSpecular();

                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                {}
                else                                                                    // Decrement by one minute
                {
                    a_ += osg::Vec4f(0.1f,0.1f,0.1f,0.1f);  d_ += osg::Vec4f(0.1f,0.1f,0.1f,0.1f); s_ += osg::Vec4f(0.1f,0.1f,0.1f,0.1f); 
                }
                ls_->getLight()->setAmbient(a_);
                ls_->getLight()->setDiffuse(d_);
                ls_->getLight()->setSpecular(s_);

                return true;
            }

            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract)
            {
                const osg::Vec4f zero(0.0f,0.0f,0.0f,0.0f);
                auto a_ = ls_->getLight()->getAmbient();
                auto d_ = ls_->getLight()->getDiffuse();
                auto s_ = ls_->getLight()->getSpecular();

                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                {}
                else                                                                    // Decrement by one minute
                {
                  a_ -= osg::Vec4f(0.1f,0.1f,0.1f,0.1f);  d_ -= osg::Vec4f(0.1f,0.1f,0.1f,0.1f); s_ -= osg::Vec4f(0.1f,0.1f,0.1f,0.1f);
                  if(a_ < zero) a_ = zero; if(d_ < zero) d_ = zero; if(s_ < zero) s_ = zero;
                }
                ls_->getLight()->setAmbient(a_);
                ls_->getLight()->setDiffuse(d_);
                ls_->getLight()->setSpecular(s_);

                return true;
            }

        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Increment time by one minute");
        usage.addKeyboardMouseBinding("Shift Keypad +", "Increment time by one hour"  );
        usage.addKeyboardMouseBinding("Alt Keypad +",   "Increment time by one day"   );
        usage.addKeyboardMouseBinding("Ctrl Keypad +",  "Increment time by one month" );
        usage.addKeyboardMouseBinding("Keypad -",       "Decrement time by one minute");
        usage.addKeyboardMouseBinding("Shift Keypad -", "Decrement time by one hour"  );
        usage.addKeyboardMouseBinding("Alt Keypad -",   "Decrement time by one day"   );
        usage.addKeyboardMouseBinding("Ctrl Keypad -",  "Decrement time by one month" );
    }

    osg::ref_ptr<osg::LightSource> ls_;
};

int main_bump_map( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::DisplaySettings::instance()->setNumMultiSamples( 8 );
    
    osg::ref_ptr<osg::Group> root (new osg::Group);

    osg::ref_ptr<osg::StateSet> lightSS (root->getOrCreateStateSet());
    //osg::ref_ptr<osg::LightSource> lightSource1 = new osg::LightSource;
    osg::ref_ptr<osg::LightSource> lightSource2 = new osg::LightSource;
    // create a local light.
    osg::Vec4f lightPosition2 (osg::Vec4f(-200.0,-100.0,-300.0,0.0f));
    osg::ref_ptr<osg::Light> myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(lightPosition2);

#if 1
    myLight2->setAmbient(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    myLight2->setDiffuse(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    myLight2->setSpecular(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
#else
    myLight2->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.9f,0.9f,0.85f,1.0f));
    myLight2->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    myLight2->setConstantAttenuation(1.0f);
#endif

    lightSource2->setLight(myLight2.get());
    lightSource2->setLocalStateSetModes(osg::StateAttribute::ON); 
    lightSource2->setStateSetModes(*lightSS,osg::StateAttribute::ON);


/////////////////////////////////////////////////////////////////////////    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) 
        scene = creators::loadAirplane();// CreateEarth(); //osgDB::readNodeFile("skydome.osgt");//  //osgDB::readNodeFile("spaceship.osgt"); // 
    
    ComputeTangentVisitor ctv;
    ctv.setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    scene->accept( ctv );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, vertSource) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragSource2) );
    program->addBindAttribLocation( "tangent", 6 );
    program->addBindAttribLocation( "binormal", 7 );

#if 0
    osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    colorTex->setImage( osgDB::readImageFile("Images/whitemetal_diffuse.jpg") );
    
    osg::ref_ptr<osg::Texture2D> normalTex = new osg::Texture2D;
    normalTex->setImage( osgDB::readImageFile("Images/whitemetal_normal.jpg") );
#else
    osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    colorTex->setImage( osgDB::readImageFile("a_319_airfrance.dds"/*,new osgDB::Options("dds_flip")*/) );//
    
    osg::ref_ptr<osg::Texture2D> normalTex = new osg::Texture2D;
    normalTex->setImage( osgDB::readImageFile("a_319_n.png") );  
#endif

    osg::StateSet* stateset = scene->getOrCreateStateSet();
    stateset->addUniform( new osg::Uniform("colorTex", 0) );
    stateset->addUniform( new osg::Uniform("normalTex", 1) );
    stateset->setAttributeAndModes( program.get() );

    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
    stateset->setTextureAttributeAndModes( 0, colorTex.get(), value );
    stateset->setTextureAttributeAndModes( 1, normalTex.get(), value );


#if 0  // Очень интересно и прозрачно, но работает только в отсутствии шейдера
    stateset->setMode( GL_BLEND,osg::StateAttribute::ON );

    //stateset->setAttributeAndModes( new osg::CullFace() );
    //stateset->setAttributeAndModes( new osg::Depth( osg::Depth::LESS, 0.f, 1.f, true ) );

    stateset->setAttributeAndModes( new osg::AlphaFunc( osg::AlphaFunc::GREATER, 0.0f ) );
#endif

    osgDB::writeNodeFile(*scene,"bump_mapping_test.osgt");
    
    root->addChild(scene.get());
	root->addChild(lightSource2.get());

    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( root.get() );

    // Use a default camera manipulator
    osgGA::TrackballManipulator* manip = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(manip);
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.addEventHandler(new LightChangeHandler(lightSource2.get()));

    viewer.addEventHandler(new AnimationHandler(scene,animationName
            ,[&](){/*effects::insertParticle(model->asGroup(),model_parts[2],osg::Vec3(00.f,00.f,00.f),0.f);*/}
            ,[&](bool off){/*model_parts[2]->setUpdateCallback(off?nullptr:new circleAimlessly());*/}
            ,[&](bool low){/*model_parts[4]->setNodeMask(low?0:0xffffffff);model_parts[5]->setNodeMask(low?0xffffffff:0);*/}
    ));


    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );


    return viewer.run();
}
