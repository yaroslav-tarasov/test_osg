#include "av/precompiled.h"

#include "geometry/lispsm_matrix.h"
#include "geometry/custom_matrix.h"

#include "av/avUtils/materials.h"
#include "av/avScene/Scene.h"
#include "Lights.h"
#include "LightMaps.h"

#if 0
#ifdef SCREEN_TEXTURE
#include "av/avScene/ScreenTextureManager.h"
#endif
#endif

namespace {

char vertexShaderSource[] = 
    "#extension GL_ARB_gpu_shader5 : enable \n"
    "uniform mat4 mvp_matrix; \n"
    "attribute vec3 from_l ; \n"
    "attribute vec3 l_dir ; \n"
    "attribute vec3 l_color ; \n"
    "attribute vec2 dist_falloff ; \n"
    "attribute vec2 cone_falloff ; \n"
    "out block  \n"
    "{          \n"
    "    vec3 from_l;              \n"
    "    flat vec3 l_dir;          \n"
    "    flat vec3 l_color;        \n"
    "    flat vec2 dist_falloff;   \n"
    "    flat vec2 cone_falloff;   \n"
    "} v_out;                      \n"
    "varying vec2 MCPosition; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    v_out.from_l = from_l; \n"
    "    v_out.l_dir = l_dir; \n"
    "    v_out.l_color = l_color; \n"
    "    v_out.dist_falloff = dist_falloff; \n"
    "    v_out.cone_falloff = cone_falloff; \n"
    "    gl_Position =   gl_ModelViewProjectionMatrix *gl_Vertex;\n"
    "}\n";


char fragmentShaderSource[] =  {
    "#version 430 compatibility \n"
    "#extension GL_ARB_gpu_shader5 : enable \n"


    STRINGIFY(
    
\n    subroutine vec4 LightFunc(vec3);
\n
\n    subroutine (LightFunc) vec4 ambient_aa(vec3 n) 
\n    {
\n        return vec4(1.0,1.0,1.0,1.0);
\n    }
\n
    in block                                                                                                       \n
    {                                                                                                              \n
        vec3 from_l;                                                                                               \n
        flat vec3 l_dir;                                                                                           \n
        flat vec3 l_color;                                                                                         \n
        flat vec2 dist_falloff;                                                                                    \n
        flat vec2 cone_falloff;                                                                                    \n
    } f_in;                                                                                                        \n
                                                                                                                   \n
    out vec4 FragColor;                                                                                            \n
                                                                                                                   \n
    void main()                                                                                                    \n
    {                                                                                                              \n
        // get dist falloff                                                                                        \n
        const float dist_rcp = inversesqrt(dot(f_in.from_l, f_in.from_l));                                         \n
        const vec3 from_l_nrm = dist_rcp * f_in.from_l;                                                            \n
        const float dist_atten = clamp(fma(dist_rcp, f_in.dist_falloff.x, f_in.dist_falloff.y), 0.0, 1.0);         \n
        // get conical falloff                                                                                     \n
        const float angle_dot = dot(from_l_nrm, f_in.l_dir);                                                       \n
        const float angle_atten = clamp(fma(angle_dot, f_in.cone_falloff.x, f_in.cone_falloff.y), 0.0, 1.0);       \n
        // diffuse-like term for planar surfaces                                                                   \n
        //const float ndotl = clamp(fma(-from_l_nrm.z, 0.35, 0.65), 0.0, 1.0);                                     \n
        // write color                                                                                             \n
        const float height_packed = -f_in.from_l.z;                                                                \n
        const float angledist_atten = angle_atten * dist_atten;                                                    \n
        const float angledist_atten_ramped = angledist_atten * (2.0 - angledist_atten);                            \n
        FragColor = vec4(f_in.l_color * (angledist_atten/* * ndotl*/), height_packed * angledist_atten_ramped);    \n
        /*FragColor = vec4(f_in.l_color , 0.5); */
    }                                                                                                             
    )                                                                                                             
};

}

class CameraCullCallback : public osg::NodeCallback
{
    bool &_NeedToReset;
public:

    CameraCullCallback  ( bool &NeedToReset ) :
      _NeedToReset ( NeedToReset )
      {}

      virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
      {
          if ( _NeedToReset )
          { osgUtil::CullVisitor *cv = dynamic_cast < osgUtil::CullVisitor * > ( nv );
          if ( cv )
          { _NeedToReset = false;
          osgUtil::RenderStage* cur_stage = cv->getCurrentRenderBin()->getStage();
          cur_stage->setCameraRequiresSetUp(true);
          cur_stage->setFrameBufferObject(0);
          }
          }
          nv->traverse(*node);
      }

};

class LightMapCamera : public osg::Camera
{
public:

    LightMapCamera()
    {
        init();
    };

    osg::Texture2D        * getTexture() const { return _texture.get(); }

protected:


    void init()
    {
        _texture = creators::getTextureHolder().getLightMapTexture();

        setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        setClearDepth(1.0);

        setClearColor(osg::Vec4(0.0,0.0,0.0,0.0));
        setColorMask(true, true, true, true);

        // just inherit the main cameras view
        setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        setProjectionMatrix(osg::Matrixd::identity());
        setViewMatrix(osg::Matrixd::identity());

        // set viewport
        setViewport(0,0,_texture->getTextureWidth(),_texture->getTextureHeight());

        // set the camera to render before the main camera.
        setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        setRenderOrder(osg::Camera::PRE_RENDER);

        // attach the texture and use it as the color buffer.
        attach(osg::Camera::COLOR_BUFFER, _texture.get(), 0, 0, true, 0, 0);

        osg::StateSet * pSS = /*_camera->*/getOrCreateStateSet();
        pSS->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        pSS->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        pSS->setMode(GL_DEPTH_CLAMP_NV, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        pSS->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        
        FIXME( invalid operation after BlendEquation)
        osg::BlendEquation * pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD,osg::BlendEquation::ALPHA_MAX);
        //pSS->setAttributeAndModes(pBlendEquation, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        pSS->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE,osg::BlendFunc::ONE, osg::BlendFunc::ONE), osg::StateAttribute::ON);

        pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

#if 0
#ifdef SCREEN_TEXTURE
		// depth texture
		avScene::ScreenTextureManager * pScrTexManager = avScene::Scene::GetInstance()->getScreenTextureManager();
		_texNormals = pScrTexManager->request(RENDER_BIN_AFTER_MODELS, avScene::ScreenTexture::DEPTH_TEXTURE);
		static const int g_nDepthTexUnit = 0;
		pSS->setTextureAttribute(g_nDepthTexUnit, _texNormals->getTexture(avScene::ScreenTexture::NORMAL_TEXTURE));
		pSS->addUniform(new osg::Uniform("DepthTexture", g_nDepthTexUnit));
		pSS->addUniform(_texNormals->getSettings());
#endif
#endif

    }

    void traverse(osg::NodeVisitor& nv)
    {
        if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
            osg::Group::traverse(nv);
    }

    osg::ref_ptr<osg::Texture2D>         _texture;

#if 0
	osg::ref_ptr<avScene::ScreenTexture> _texNormals;
#endif
};

// intersection points
typedef std::vector<cg::point_2f>  inter_points_vector;
typedef std::array<cg::point_2f,4> inter_points_array_4;

class Private : public LightMapRenderer
{
 
    friend struct LightMapCullCallback;
    friend ILightMapRendererPtr createLightMapRenderer(osg::Group * sceneRoot);

public: // ILightMapRenderer

    // set camera frustum and calculate all transformations
    // also clear collected lights
    virtual void                SetupProjection( cg::frustum_f const & view_frustum, float dist_max, bool night_mode );

    // get matrix
    virtual cg::matrix_4f const & GetViewTextureMatrix() const;
    // get night mode
    virtual bool                GetNightMode() const;

    // add spot on cull pass
    virtual void                AddSpotLight( SpotData const & spot );

    // update uniforms
    void                        UpdateTextureMatrix( bool enabled );

    virtual void                traverse(osg::NodeVisitor& nv);    
    
    struct SpotRenderVertex
    {
        cg::point_3f                    v;
        cg::point_3f                    from_l;
        cg::point_3f                    ldir;
        cg::colorf                      lcolor;
        cg::point_2f                    dist_falloff;
        cg::point_2f                    cone_falloff;
    };

    void                                setMVP(const osg::Matrixf& mvp);
    osg::Matrix                         getProjectionMatrix() {return to_osg_matrix(proj_matr_);};

protected:

    Private(osg::Group * sceneRoot);

    void							    _createArrays();
    osg::Geometry *                     _createGeometry();
    void                                _clearArrays();
    void                                _commitLights();
    
    template <typename T >
    void add_light( T const & light_contour, cg::point_3f const & world_lightpos, cg::point_3f const & world_lightdir, SpotData const & spot );
    
    void cull( osg::NodeVisitor * nv );

protected:
    unsigned                             tex_dim_;
    bool                                 we_see_smth_;

    cg::transform_4f                     mv_;

    cg::matrix_4f                        proj_matr_;
    cg::matrix_4f                        tex_matr_;

    cg::frustum_clipper                  lightmap_clipper_;

    ITexturePtr                          color_buf_;
    unsigned                             _tex_dim;
    
    std::vector<SpotRenderVertex>        vertices_;

    osg::ref_ptr<LightMapCamera>         _camera;
    
    osg::ref_ptr<osg::Uniform>           _lightmapMatrix;

    osg::ref_ptr<osg::Geometry>          _geom         ;
    osg::Vec3Array  *                    geom_array_   ;
    osg::ref_ptr<osg::Vec3Array>         from_l_       ;
    osg::ref_ptr<osg::Vec3Array>         ldir_         ;
    osg::ref_ptr<osg::Vec3Array>         lcolor_       ;
    osg::ref_ptr<osg::Vec2Array>         dist_falloff_ ;
    osg::ref_ptr<osg::Vec2Array>         cone_falloff_ ;

private:
    bool                                            _NeedToUpdateFBO;
	boost::optional<std::pair<matrix_4f,matrix_4f>> clpt_mat_;
};

// create
ILightMapRendererPtr createLightMapRenderer(osg::Group * sceneRoot)
{
    return new Private(sceneRoot);
}



// cone
static inter_points_vector make_intersection( const cg::point_3f & world_light_pos, const cg::point_3f & world_light_dir, float half_angle_max, float dist_max )
{
    inter_points_vector res;
    res.reserve(15);

    //high_res_timer _hr_timer;

    // alpha-cut
    if (fabs(world_light_pos.z) < dist_max)
    {
        static const unsigned subd_cone = 4;
        typedef std::array<cg::point_3f, subd_cone + 1> cone_frustum;
        static cone_frustum world_pts;

        typedef std::pair<unsigned, unsigned> frustum_edge;
        typedef std::array<frustum_edge, 2 * subd_cone> frustum_edges;
        static frustum_edges edges = {
            frustum_edge(0, 1), frustum_edge(0, 2), frustum_edge(0, 3), frustum_edge(0, 4),
            frustum_edge(1, 2), frustum_edge(2, 3), frustum_edge(3, 4), frustum_edge(4, 1)
        };

        // distance bound
        //cg::rectangle_2f bound_rect(world_light_pos);
        //bound_rect.inflate(sqrt(dist_max * dist_max - world_light_pos.z * world_light_pos.z));

        // side vectors
        const cg::point_3f side_vec = cg::normalized(world_light_dir ^ (cg::eq(fabs(world_light_dir.z), 1.0f, 0.001f) ? cg::point_3f(0.f, 1.f, 0.f) : cg::point_3f(0.f, 0.f, 1.f)));
        const cg::point_3f up_vec = side_vec ^ world_light_dir;

        const float hatan = tan(half_angle_max);

        // make frustum points
        world_pts[0] = world_light_pos;
        world_pts[1] = world_light_pos + (world_light_dir + (-side_vec - up_vec) * hatan) * dist_max;
        world_pts[2] = world_light_pos + (world_light_dir + ( side_vec - up_vec) * hatan) * dist_max;
        world_pts[3] = world_light_pos + (world_light_dir + ( side_vec + up_vec) * hatan) * dist_max;
        world_pts[4] = world_light_pos + (world_light_dir + (-side_vec + up_vec) * hatan) * dist_max;

        // test all 8 lines
        std::vector<cg::point_2> int_pts;
        int_pts.reserve(20);
        for (unsigned i = 0; i < edges.size(); ++i)
        {
            const float & z1 = world_pts[edges[i].first].z;
            const float & z2 = world_pts[edges[i].second].z;
            if (z1 * z2 < 0.f)
            {
                const float k = (0.f - z1) / (z2 - z1);
                const auto int_pnt = cg::lerp01(world_pts[edges[i].first], world_pts[edges[i].second], k);
                //int_pts.push_back(bound_rect.closest_point(int_pnt));
                int_pts.push_back(int_pnt);
            }
        }

        if (int_pts.size() >= 3)
            cg::point_set_convex_hull_envelope(&int_pts.front(), int_pts.size(), std::back_inserter(res));
    }

    //double dt = _hr_timer.get_delta();
    //LOG_ODS_MSG("make_intersection" << dt << "\n");
    
    return res;
}
// sphere
__forceinline static inter_points_vector make_intersection( const cg::point_3f & world_light_pos, float dist_max )
{
    inter_points_vector res;
    res.reserve(4);

    // alpha-cut
    if (fabs(world_light_pos.z) < dist_max)
    {
        // intersect sphere
        const cg::point_2f& sph_center = world_light_pos;
        const float sph_rad = sqrt(cg::sqr(dist_max) - cg::sqr(world_light_pos.z));
        res.emplace_back(sph_center + cg::point_2f(-sph_rad, -sph_rad));
        res.emplace_back(sph_center + cg::point_2f(+sph_rad, -sph_rad));
        res.emplace_back(sph_center + cg::point_2f(+sph_rad, +sph_rad));
        res.emplace_back(sph_center + cg::point_2f(-sph_rad, +sph_rad));
    }

    return res;
}

__forceinline static void make_intersection(inter_points_vector& ipa, const cg::point_3f & world_light_pos, float dist_max )
{
    ipa.reserve(4);

    // alpha-cut
    if (fabs(world_light_pos.z) < dist_max)
    {
        // intersect sphere
        const cg::point_2f& sph_center = world_light_pos;
        const float sph_rad = sqrt(cg::sqr(dist_max) - cg::sqr(world_light_pos.z));
        ipa.emplace_back(sph_center + cg::point_2f(-sph_rad, -sph_rad));
        ipa.emplace_back(sph_center + cg::point_2f(+sph_rad, -sph_rad));
        ipa.emplace_back(sph_center + cg::point_2f(+sph_rad, +sph_rad));
        ipa.emplace_back(sph_center + cg::point_2f(-sph_rad, +sph_rad));
    }
}

__forceinline static void make_intersection(inter_points_array_4& ipa, const cg::point_3f & world_light_pos, float dist_max )
{
    // alpha-cut
    if (fabs(world_light_pos.z) < dist_max)
    {
        // intersect sphere
        const cg::point_2f& sph_center = world_light_pos;
        const float sph_rad = sqrt(cg::sqr(dist_max) - cg::sqr(world_light_pos.z));

        ipa[0] = (sph_center + cg::point_2f(-sph_rad, -sph_rad));
        ipa[1] = (sph_center + cg::point_2f(+sph_rad, -sph_rad));
        ipa[2] = (sph_center + cg::point_2f(+sph_rad, +sph_rad));
        ipa[3] = (sph_center + cg::point_2f(-sph_rad, +sph_rad));


    }
}


struct LightMapCullCallback : public osg::NodeCallback
{
    LightMapCullCallback(Private * p)
        : p_(p)
    {

    }

    virtual void operator()( osg::Node * pNode, osg::NodeVisitor * pNV )
    {

        osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
        osg::Group * pNodeAsGroup = static_cast<osg::Group *>(pNode);
        avAssert(pCV && pNodeAsGroup);

        const  osg::Camera* cam = avScene::GetScene()->getCamera();

        double  fLeft(-1.0f)
              , fRight(+1.0f)
              , fBottom(-1.0f)
              , fTop(+1.0f)
              , fNear(1.0f)
              , fFar(1000.0f);

        cam->getProjectionMatrixAsFrustum(fLeft, fRight, fBottom, fTop, fNear, fFar);

        osg::Vec3f eye, center, up;
        cam->getViewMatrixAsLookAt(eye, center, up);

        const cg::point_3f  vPosition(from_osg_vector3(pCV->getViewPoint()));
        auto dir = cg::polar_point_3f(from_osg_vector3(center - eye));
        const cg::cprf      rOrientation = cg::cprf(dir.course,dir.pitch,0);

        cg::frustum_perspective_f frustum_(cg::camera_f(), cg::range_2f(fNear, fFar), 2.0f * cg::rad2grad(atan(fRight / fNear)), 2.0f * cg::rad2grad(atan(fTop / fNear)));
        frustum_.camera() = cg::camera_f(vPosition, rOrientation);

        p_->SetupProjection(frustum_, 3300.f, true/*night_mode*/);

        p_->cull( pCV );
        p_->UpdateTextureMatrix(true);

        // go down
        pNV->traverse(*pNode);
    }
private:

    Private*           p_;

};


void Private::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
    {
        Group::traverse(nv);
        return;
    }


    osgUtil::CullVisitor * cv = static_cast<osgUtil::CullVisitor*>(&nv);
#if 1    
    const  osg::Camera* cam = avScene::GetScene()->getCamera();

    double  m_fLeft(-1.0f)
        , m_fRight(+1.0f)
        , m_fBottom(-1.0f)
        , m_fTop(+1.0f)
        , m_fNear(1.0f)
        , m_fFar(1000.0f);

    cam->getProjectionMatrixAsFrustum(m_fLeft, m_fRight, m_fBottom, m_fTop, m_fNear, m_fFar);

    osg::Vec3f eye, center, up;
    cam->getViewMatrixAsLookAt(eye, center, up);

    const cg::point_3f  vPosition(from_osg_vector3(nv.getViewPoint()));
    auto dir = cg::polar_point_3f(from_osg_vector3(center - eye));
    const cg::cprf      rOrientation = cg::cprf(dir.course,dir.pitch,0);

    cg::frustum_perspective_f frustum_(cg::camera_f(), cg::range_2f(m_fNear, m_fFar), 2.0f * cg::rad2grad(atan(m_fRight / m_fNear)), 2.0f * cg::rad2grad(atan(m_fTop / m_fNear)));
    frustum_.camera() = cg::camera_f(vPosition, rOrientation);
    
    const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
    SetupProjection(frustum_, 3300.f, cIlluminationParameters.Illumination < cIlluminationParameters.NavigationLightsThreshold/*night_mode*/);

    cull( cv );
    
    UpdateTextureMatrix(true);
#endif

    // set matrices from frustum calculator to camera
    if(clpt_mat_)
    {
        _camera->setViewMatrix(to_osg_matrix(clpt_mat_->first.transpose()));
        _camera->setProjectionMatrix(to_osg_matrix(clpt_mat_->second.transpose()));
    }


    // go farther
    _camera->accept(*cv);

}

osg::Geometry * Private::_createGeometry()
{
    // dummy bounding box callback
    osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

    // create OSG geode with 1 drawable node
    setCullingActive(false);
    setDataVariance(osg::Object::DYNAMIC);

    osg::Geometry * geom = new osg::Geometry;
    geom->setComputeBoundingBoxCallback(pDummyBBCompute);
    
    geom_array_ = new osg::Vec3Array;
    geom_array_->setDataVariance(osg::Object::DYNAMIC);
    
    // set vertex array

    geom->setVertexArray(geom_array_);

    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);
    geom->setDataVariance(osg::Object::DYNAMIC);

    return geom;
}

// ctor
Private::Private(osg::Group * sceneRoot)
    : tex_dim_(0)
    , we_see_smth_(false)
    , lightmap_clipper_(cg::matrix_4f())
    , _NeedToUpdateFBO(false)
{

    _lightmapMatrix = new osg::Uniform("lightmap_matrix",osg::Matrixf());
    sceneRoot->getOrCreateStateSet()->addUniform(_lightmapMatrix.get());

    osg::Geode* geode = new osg::Geode;

    _geom = _createGeometry();


    geode->addDrawable(_geom.get());

    osg::StateSet* stateset = _geom->getOrCreateStateSet();

    osg::Program* program = new osg::Program;

    osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
    program->addShader(vertex_shader);

    
    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
    program->addShader(fragment_shader);


    program->setName("LightMap");
    stateset->setAttribute(program);

    _createArrays();

    
    _camera = new LightMapCamera();
    _camera->setCullCallback ( new CameraCullCallback ( _NeedToUpdateFBO ) );


    _camera->addChild(geode);
    color_buf_ = _camera->getTexture();

    //setCullCallback(new LightMapCullCallback(this));
}
 

// add spot on cull pass
void Private::AddSpotLight( SpotData const & spot )
{
    // check visibility
    const cg::point_3f world_light_pos = mv_.treat_point(spot.view_pos, false);
    if (fabs(world_light_pos.z) < spot.dist_falloff.hi())
    {
        const cg::point_3f world_light_dir = mv_.treat_vector(spot.view_dir, false);
       
        // angle corrected
        auto corrected_spot = spot;
        if (!corrected_spot.angle_falloff.empty())
            corrected_spot.angle_falloff |= cg::lerp01(spot.angle_falloff.hi(), 65.f, cg::bound(-world_light_dir.z, 0.f, 1.f));

        // get light intersection
        inter_points_vector spot_plane;
        
        if (!corrected_spot.angle_falloff.empty())
        {
            spot_plane = std::move(make_intersection(world_light_pos, world_light_dir, cg::grad2rad(corrected_spot.angle_falloff.hi()), corrected_spot.dist_falloff.hi()));
            
            // add it

            if (spot_plane.size() > 2)
                add_light(spot_plane, world_light_pos, world_light_dir, corrected_spot);
        }
        else
        {
            //spot_plane = std::move(make_intersec  tion(world_light_pos, corrected_spot.dist_falloff.hi()));
            inter_points_array_4  spot_plane;
            make_intersection(spot_plane,world_light_pos, corrected_spot.dist_falloff.hi());
            
            // add it
            if (spot_plane.size() > 2)
                add_light(spot_plane, world_light_pos, world_light_dir, corrected_spot);
            

        }

    }
}

// set camera frustum and calculate all transformations
void Private::SetupProjection( cg::frustum_f const & view_frustum, float dist_max, bool night_mode )
{
    // clear vertex buffer
    vertices_.resize(0);
    clpt_mat_.reset();
    we_see_smth_ = false;
    if (!night_mode)
        return;

    // save data
    mv_ = view_frustum.camera().world_view_transform();

    // prepare frustum
    std::unique_ptr<cg::frustum_f> const frustum_split(view_frustum.clone());
    frustum_split->set_clipping(cg::range_2f(view_frustum.clipping_near(), dist_max));

    // get projection matrix
    clpt_mat_ = cg::clpt_matrix(*frustum_split, dist_max * 0.75f, cg::range_2f(0.f, 40.f));
	
    we_see_smth_ = clpt_mat_;
    if (!we_see_smth_)
        return;

    // ok, we see smth
    we_see_smth_ = true;
    proj_matr_ = (*clpt_mat_).second * (*clpt_mat_).first;
    // frustum
    lightmap_clipper_ = cg::frustum_clipper(proj_matr_);
    // texture matrix
    tex_matr_ = cg::xyzw_to_strq_remap_matrix(1.0f, 1.0f) * (proj_matr_ * mv_.inverse_matrix());
}

// update uniform
void Private::UpdateTextureMatrix( bool enabled )
{
    if (enabled && we_see_smth_ && !vertices_.empty())
    {
        _lightmapMatrix->set(to_osg_matrix(tex_matr_.transpose()));
    }
}

// texture matrix for specific split
cg::matrix_4f const & Private::GetViewTextureMatrix() const
{
    return tex_matr_;
}

// get night mode
bool Private::GetNightMode() const
{
    return we_see_smth_;
}

void Private::_createArrays()
{
    from_l_ = new osg::Vec3Array();
    _geom->setVertexAttribArray(1, from_l_.get(),osg::Array::BIND_PER_VERTEX);
    _geom->setVertexAttribNormalize(1, false);
    ldir_ = new osg::Vec3Array();
    _geom->setVertexAttribArray(2, ldir_.get(),osg::Array::BIND_PER_VERTEX);
    _geom->setVertexAttribNormalize(2, false);
    lcolor_ = new osg::Vec3Array();
    _geom->setVertexAttribArray(3, lcolor_.get(),osg::Array::BIND_PER_VERTEX);
    _geom->setVertexAttribNormalize(3, false);
    dist_falloff_ = new osg::Vec2Array();
    _geom->setVertexAttribArray(4, dist_falloff_.get(),osg::Array::BIND_PER_VERTEX);
    _geom->setVertexAttribNormalize(4, false);
    cone_falloff_ = new osg::Vec2Array();
    _geom->setVertexAttribArray(5, cone_falloff_.get(),osg::Array::BIND_PER_VERTEX);
    _geom->setVertexAttribNormalize(5, false);

    from_l_->setPreserveDataType(true);
    ldir_->setPreserveDataType(true);
    dist_falloff_->setPreserveDataType(true);
    cone_falloff_->setPreserveDataType(true);
    lcolor_->setPreserveDataType(true);

    _geom->addPrimitiveSet(new osg::DrawArrays() );
}

void Private::_clearArrays()
{
    from_l_->clear();
    ldir_->clear();
    dist_falloff_->clear();
    cone_falloff_->clear();
    lcolor_->clear();
}

//
// Impl
//

template <typename T >
void Private::add_light( T const & light_contour, cg::point_3f const & world_lightpos, cg::point_3f const & world_lightdir, SpotData const & spot )
{
    // convert angular falloffs
    cg::point_2f dot_falloff(0, 1);
    if (!spot.angle_falloff.empty())
    {
        float const dot_start = cos(cg::grad2rad(spot.angle_falloff.lo()));
        float const dot_fade = cos(cg::grad2rad(spot.angle_falloff.hi()));
        dot_falloff = cg::point_2f(-1.0f / (dot_fade - dot_start), dot_fade / (dot_fade - dot_start));
    }
    // convert dist falloffs
    float const rcp_start = 1.0f / spot.dist_falloff.lo();
    float const rcp_fade = 1.0f / spot.dist_falloff.hi();
    cg::point_2f const rcp_falloff(-1.0f / (rcp_fade - rcp_start), rcp_fade / (rcp_fade - rcp_start));

    // basic light data
    SpotRenderVertex new_v;
    new_v.ldir = world_lightdir;
    new_v.lcolor = spot.spot_color;
    new_v.dist_falloff = rcp_falloff;
    new_v.cone_falloff = dot_falloff;


    // add points (3 for each triangle)
    std::vector<SpotRenderVertex> new_spot(3 * (light_contour.size() - 2), new_v);
    for (unsigned i = 0; i < light_contour.size() - 2; ++i)
    {
        // 0
        SpotRenderVertex & cur_v0 = new_spot[i * 3 + 0];
        cur_v0.v = cg::point_3f(light_contour[0]);
        cur_v0.from_l = cur_v0.v - world_lightpos;
        // 1
        SpotRenderVertex & cur_v1 = new_spot[i * 3 + 1];
        cur_v1.v = cg::point_3f(light_contour[i + 1]);
        cur_v1.from_l = cur_v1.v - world_lightpos;
        // 2
        SpotRenderVertex & cur_v2 = new_spot[i * 3 + 2];
        cur_v2.v = cg::point_3f(light_contour[i + 2]);
        cur_v2.from_l = cur_v2.v - world_lightpos;
    }

    // append them
    vertices_.insert(vertices_.end(), new_spot.begin(), new_spot.end());
}


void Private::_commitLights()
{
    geom_array_ = static_cast<osg::Vec3Array *>(_geom->getVertexArray());
    geom_array_->resize(0);
    
    _clearArrays();

    if(vertices_.empty())
    {
        _geom->setPrimitiveSet(0, new osg::DrawArrays() );
        return;
    }

    geom_array_->resize(vertices_.size());

    from_l_->resize(vertices_.size());
    ldir_->resize(vertices_.size());
    dist_falloff_->resize(vertices_.size());
    cone_falloff_->resize(vertices_.size());
    lcolor_->resize(vertices_.size());


    for (unsigned i = 0; i < vertices_.size(); ++i)
    {
        SpotRenderVertex & cur_v = vertices_[i];
        geom_array_->at(i).set(cur_v.v.x, cur_v.v.y, cur_v.v.z);
        from_l_->at(i).set( cur_v.from_l.x, cur_v.from_l.y,cur_v.from_l.z );
        ldir_->at(i).set( cur_v.ldir.x, cur_v.ldir.y, cur_v.ldir.z );
        dist_falloff_->at(i).set( cur_v.dist_falloff.x, cur_v.dist_falloff.y  );
        cone_falloff_->at(i).set( cur_v.cone_falloff.x, cur_v.cone_falloff.y );
        lcolor_->at(i).set(  cur_v.lcolor.r, cur_v.lcolor.g, cur_v.lcolor.b  );
    }


    _geom->setPrimitiveSet(0, new osg::DrawArrays( GL_TRIANGLES, 0, vertices_.size() ) );

    from_l_->dirty();
    ldir_->dirty();
    dist_falloff_->dirty();
    cone_falloff_->dirty();
    lcolor_->dirty();
    geom_array_->dirty();
}



void Private::cull( osg::NodeVisitor * nv )
{
    using namespace avScene;

    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();

    avScene::Scene * scene = GetScene();

    if (scene == NULL)
        return;

    avScene::Lights * lights = scene->getLights();
    
    if (lights == NULL)
        return;

    const std::vector<LightProcessedInfo>& lpi = lights->GetProcessedLights();
    const std::vector<LightExternalInfo>&  lei = lights->GetVisibleLights();

    if(clpt_mat_)
    {
        auto it_p = lpi.begin();
        auto const it_end = lei.end();
        for (auto it = lei.begin();it!=it_end;++it,++it_p)
        {
            SpotData spot;
            cg::transform_4f tr = from_osg_matrix(mWorldToView);
        
            spot.view_dir =cg::point_3f(it_p->lightVSDirSpecRatio.x(),it_p->lightVSDirSpecRatio.y(),it_p->lightVSDirSpecRatio.z());
            // spot.view_dir =  tr.treat_vector(it->vDirWorld,false);
 
            spot.view_pos =  cg::point_3f(it_p->lightVSPosAmbRatio.x(),it_p->lightVSPosAmbRatio.y(),it_p->lightVSPosAmbRatio.z());
            spot.spot_color    = it->cDiffuse;
            spot.dist_falloff  = it->rDistAtt;
            spot.angle_falloff = (!it->rConeAtt.empty())?cg::rad2grad()*it->rConeAtt:it->rConeAtt ;

            AddSpotLight( spot );
        }
    }

    //force_log fl;
    // LOG_ODS_MSG( "_private::cull  " << _hr_timer.get_delta() << "\n");

    _commitLights();

}
