#include "stdafx.h"

#include "light_maps.h"

#include "geometry/lispsm_matrix.h"

#include "av/Prerender.h"
#include "av/Scene.h"

//"uniform vec4 coeff; \n"
//"attribute vec3 from_l : ATTR2; \n"
//"attribute vec3 l_dir : ATTR3; \n"
//"attribute vec3 l_color : ATTR4; \n"
//"attribute vec2 dist_falloff : ATTR5; \n"
//"attribute vec2 cone_falloff : ATTR6; \n"

#define GL_RGBA16F 0x881A
char vertexShaderSource_simple[] = 
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
    "    gl_Position =    mvp_matrix*gl_Vertex;\n"
    "}\n";


char fragmentShaderSource[] =  {
    "#version 430 compatibility \n"
    "#extension GL_ARB_gpu_shader5 : enable \n"


    STRINGIFY(

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
\n      const float dist_rcp = inversesqrt(dot(f_in.from_l, f_in.from_l));                                         \n
        const vec3 from_l_nrm = dist_rcp * f_in.from_l;                                                            \n
        const float dist_atten = clamp(fma(dist_rcp, f_in.dist_falloff.x, f_in.dist_falloff.y), 0.0, 1.0);         \n
        // get conical falloff                                                                                     \n
\n      const float angle_dot = dot(from_l_nrm, f_in.l_dir);                                                       \n
        const float angle_atten = clamp(fma(angle_dot, f_in.cone_falloff.x, f_in.cone_falloff.y), 0.0, 1.0);       \n
        // diffuse-like term for planar surfaces                                                                   \n
        //const float ndotl = clamp(fma(-from_l_nrm.z, 0.35, 0.65), 0.0, 1.0);                                     \n
        // write color                                                                                             \n
\n      const float height_packed = -f_in.from_l.z;                                                                \n
        const float angledist_atten = angle_atten * dist_atten;                                                    \n
        const float angledist_atten_ramped = angledist_atten * (2.0 - angledist_atten);                            \n
        FragColor = vec4(f_in.l_color * (angledist_atten/* * ndotl*/), height_packed * angledist_atten_ramped);    \n
        
    }                                                                                                             
    )                                                                                                             
};

char fragmentShaderSource_test[] = 
    "#extension GL_ARB_gpu_shader5 : enable \n"
    "uniform sampler2D baseTexture; \n"
    "in block  \n"
    "{          \n"
    "    vec3 from_l;              \n"
    "    flat vec3 l_dir;          \n"
    "    flat vec3 l_color;        \n"
    "    flat vec2 dist_falloff;   \n"
    "    flat vec2 cone_falloff;   \n"
    "} v_in;                      \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = vec4(vec3(v_in.cone_falloff,0.5),1.0); \n"
    "}\n";

char geometryShaderSource[] = {
    "#version 430 compatibility \n"

    "#extension GL_EXT_geometry_shader4 : enable\n"
    "#extension GL_ARB_gpu_shader5 : enable\n"

    STRINGIFY(
    layout(points) in;                                                                              \n
    layout(triangle_strip, max_vertices = 4) out;                                                   \n
                                                                                                    \n
    in mat4 projection_matrix;                                                                      \n
                                                                                                    \n
    in block                                                                                        \n
    {                                                                                               \n
        vec4 color;                                                                                 \n
        vec3 size_pixel_vert;                                                                       \n
    } g_in[1];                                                                                      \n
                                                                                                    \n
    out block                                                                                       \n
    {                                                                                               \n
        centroid vec2 tex;                                                                          \n
        vec4 color;                                                                                 \n
    } g_out;                                                                                        \n
                                                                                                    \n
    const vec2 g_ParticleCoords[4] = vec2[]                                                         \n
    (                                                                                               \n
        vec2(-1.0, -g_in[0].size_pixel_vert.z),                                                     \n
        vec2(+1.0, -g_in[0].size_pixel_vert.z),                                                     \n
        vec2(-1.0, +1.0),                                                                           \n
        vec2(+1.0, +1.0)                                                                            \n
        );                                                                                          \n
                                                                                                    \n
    const vec2 g_ParticleTexCoords[4] = vec2[]                                                      \n
    (                                                                                               \n
        vec2(-1.0, -1.0),                                                                           \n
        vec2(+1.0, -1.0),                                                                           \n
        vec2(-1.0, +1.0),                                                                           \n
        vec2(+1.0, +1.0)                                                                            \n
        );                                                                                          \n
                                                                                                    \n
    void main(void)                                                                                 \n
    {                                                                                               \n
        const vec3 vViewPosition = gl_in[0].gl_Position.xyz;                                        \n
        const float fSize = g_in[0].size_pixel_vert.x + abs(vViewPosition.z) * g_in[0].size_pixel_vert.y; \n
                                                                                                    \n
        vec2 vOffset;                                                                               \n
        vec4 vViewPos;                                                                              \n
                                                                                                    \n
        g_out.color = g_in[0].color;                                                                \n
        if (g_out.color.a >= 0.002)                                                                 \n
        {                                                                                           \n
            // lower-left                                                                           
\n          vOffset = fSize * g_ParticleCoords[0];                                                  \n
            vViewPos = vec4(gl_in[0].gl_Position.xyz + vec3(vOffset, 0.0), 1.0);                    \n
            gl_Position = projection_matrix * vViewPos;                                             \n
            g_out.tex = g_ParticleTexCoords[0];                                                     \n
            EmitVertex();                                                                           \n
                                                                                                    \n
            // lower-right                                                                          
\n          vOffset = fSize * g_ParticleCoords[1];                                                  \n
            vViewPos = vec4(gl_in[0].gl_Position.xyz + vec3(vOffset, 0.0), 1.0);                    \n
            gl_Position = projection_matrix * vViewPos;                                             \n
            g_out.tex = g_ParticleTexCoords[1];                                                     \n
            EmitVertex();                                                                           \n
                                                                                                    \n
            // upper-left                                                                           \n
\n          vOffset = fSize * g_ParticleCoords[2];                                                  \n
            vViewPos = vec4(gl_in[0].gl_Position.xyz + vec3(vOffset, 0.0), 1.0);                    \n
            gl_Position = projection_matrix * vViewPos;                                             \n
            g_out.tex = g_ParticleTexCoords[2];                                                     \n
            EmitVertex();                                                                           \n
                                                                                                    \n
            // upper-right                                                                          \n
\n          vOffset = fSize * g_ParticleCoords[3];                                                  \n
            vViewPos = vec4(gl_in[0].gl_Position.xyz + vec3(vOffset, 0.0), 1.0);                    \n
            gl_Position = projection_matrix * vViewPos;                                             \n
            g_out.tex = g_ParticleTexCoords[3];                                                     \n
            EmitVertex();                                                                           \n
                                                                                                    \n
            //EndPrimitive();                                                                       \n
        }
    }

    )
};

class _private : public LightMapRenderer
{
 
    friend ILightMapRendererPtr createLightMapRenderer();

public: // ILightMapRenderer
    // reserves textures, setups FBO, etc
    virtual void                InitializeTexture( unsigned tex_dim );

    // set camera frustum and calculate all transformations
    // also clear collected lights
    virtual void                SetupProjection( cg::frustum_f const & view_frustum, float dist_max, bool night_mode );

    // get matrix
    virtual cg::matrix_4f const & GetViewTextureMatrix() const;
    // get night mode
    virtual bool                GetNightMode() const;

    // add spot on cull pass
    virtual void                AddSpotLight( SpotData const & spot );

    // render lightmap
    virtual ITexture*           UpdateTexture( bool enabled );
    
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

    _private();
    
    osg::Geometry *                     _createGeometry();


protected:
    unsigned                             tex_dim_;
    bool                                 we_see_smth_;
    ITexturePtr                          empty_tex_;

    cg::transform_4f                     mv_;

    cg::matrix_4f                        proj_matr_;
    cg::matrix_4f                        tex_matr_;

    cg::frustum_clipper                  lightmap_clipper_;

    ITexturePtr                          color_buf_;
    // osg::ref_ptr<osg::FrameBufferObject> fbo_;
    unsigned                             _tex_dim;
    
    std::vector<SpotRenderVertex>        vertices_;

    void add_light( std::vector<cg::point_2f> const & light_contour, cg::point_3f const & world_lightpos, cg::point_3f const & world_lightdir, SpotData const & spot );
    
    osg::ref_ptr<osg::Uniform>      uni_mvp_      ;
    osg::ref_ptr<osg::Geometry>     geom_         ;
    osg::ref_ptr<osg::Vec3Array>    from_l_       ;
    osg::ref_ptr<osg::Vec3Array>    ldir_         ;
    osg::ref_ptr<osg::Vec3Array>    lcolor_       ;
    osg::ref_ptr<osg::Vec2Array>    dist_falloff_ ;
    osg::ref_ptr<osg::Vec2Array>    cone_falloff_ ;
};

// create
ILightMapRendererPtr createLightMapRenderer()
{
    return new _private();
}

// intersection points
typedef std::vector<cg::point_2f> inter_points_array;

// cone
static inter_points_array make_intersection( const cg::point_3f & world_light_pos, const cg::point_3f & world_light_dir, float half_angle_max, float dist_max )
{
    inter_points_array res;

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

    return res;
}
// sphere
static inter_points_array make_intersection( const cg::point_3f & world_light_pos, float dist_max )
{
    inter_points_array res;

    // alpha-cut
    if (fabs(world_light_pos.z) < dist_max)
    {
        // intersect sphere
        const cg::point_2f sph_center = world_light_pos;
        const float sph_rad = sqrt(cg::sqr(dist_max) - cg::sqr(world_light_pos.z));
        res.emplace_back(sph_center + cg::point_2f(-sph_rad, -sph_rad));
        res.emplace_back(sph_center + cg::point_2f(+sph_rad, -sph_rad));
        res.emplace_back(sph_center + cg::point_2f(+sph_rad, +sph_rad));
        res.emplace_back(sph_center + cg::point_2f(-sph_rad, +sph_rad));
    }

    return res;
}

struct LightMapCullCallback : public osg::NodeCallback
{
    LightMapCullCallback(_private * p)
        : p_(p)
    {

    }

    virtual void operator()( osg::Node * pNode, osg::NodeVisitor * pNV )
    {

        osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
        osg::Group * pNodeAsGroup = static_cast<osg::Group *>(pNode);
        avAssert(pCV && pNodeAsGroup);

        Prerender * pPrerenderNode = static_cast<Prerender *>(pNodeAsGroup->getChild(0));
        avAssert(pPrerenderNode);
        
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
        
        const cg::point_3f  vPosition(from_osg_vector3(pNV->getViewPoint()));
        auto dir = cg::polar_point_3f(/*target_pos.pos - begin_pos.pos*/from_osg_vector3(center - eye));
        const cg::cprf      rOrientation = cg::cprf(dir.course,dir.pitch,0);

        cg::frustum_perspective_f frustum_(cg::camera_f(), cg::range_2f(m_fNear, m_fFar), /*2.0f **/ 2.0f * cg::rad2grad(atan(m_fRight / m_fNear)), /*2.0f **/ 2.0f * cg::rad2grad(atan(m_fTop / m_fNear)));
        frustum_.camera() = cg::camera_f(vPosition, rOrientation);

        p_->SetupProjection(frustum_, 3300.f, true/*night_mode*/);

        const osg::Matrixd & mProjection = *pCV->getProjectionMatrix();
        osg::Matrixd mModelView = *pCV->getModelViewMatrix();

        p_->UpdateTexture(true);
#if 0
        pPrerenderNode->setProjectionMatrix(mProjection/*p_->getProjectionMatrix()*/);
        pPrerenderNode->setViewMatrix(mModelView/*osg::Matrix()*/);
#endif

        // go down
        pNV->traverse(*pNode);
    }
private:

    _private*           p_;

};

void            _private::setMVP(const osg::Matrixf& mvp)
{
     uni_mvp_->set(mvp);
}

osg::Geometry * _private::_createGeometry()
{
    // dummy bounding box callback
    osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

    // create OSG geode with 1 drawable node
    setCullingActive(false);
    setDataVariance(osg::Object::STATIC);

    // create tetrahedron around viewer (just to fill the whole volume)
    osg::Geometry * box_geometry = new osg::Geometry;
    box_geometry->setUseDisplayList(true);
    box_geometry->setDataVariance(osg::Object::STATIC);
    box_geometry->setComputeBoundingBoxCallback(pDummyBBCompute);

    const float fSqrt3 = sqrtf(3.0f);

    // create its' vertex
    osg::Vec3Array * paBoxPointsPos = new osg::Vec3Array;
    paBoxPointsPos->resize(4);
    //paBoxPointsPos->at(0).set(0.f, 0.f, +2.f);
    //paBoxPointsPos->at(1).set(-2.0f * fSqrt3, 0.f, -1.0f);
    //paBoxPointsPos->at(2).set(fSqrt3, -3.0f, -1.0f);
    //paBoxPointsPos->at(3).set(fSqrt3, +3.0f, -1.0f);

    paBoxPointsPos->at(0).set(0.f, 0.f, +1.f);
    paBoxPointsPos->at(1).set(-1, 0.f, -1.0f);
    paBoxPointsPos->at(2).set(1, -1.0f, -1.0f);
    paBoxPointsPos->at(3).set(1, +1.0f, -1.0f);
    // set vertex array
    paBoxPointsPos->setDataVariance(osg::Object::DYNAMIC);
#if 0
    box_geometry->setVertexArray(paBoxPointsPos);
#endif

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

    paBoxDrawElem->setDataVariance(osg::Object::DYNAMIC);
#if 0    
    box_geometry->addPrimitiveSet(paBoxDrawElem);
#endif

    return box_geometry;
}

// ctor
_private::_private()
    : tex_dim_(0)
    , we_see_smth_(false)
    , lightmap_clipper_(cg::matrix_4f())
{
    osg::Geode* geode = new osg::Geode;

    geom_ = _createGeometry();
    geom_->setUseDisplayList(false);
    geom_->setUseVertexBufferObjects(true);
    geom_->setDataVariance(osg::Object::DYNAMIC);

    osg::BlendEquation * pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD,osg::BlendEquation::ALPHA_MAX);
    geom_->getOrCreateStateSet()->setAttributeAndModes(pBlendEquation, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    geom_->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE,osg::BlendFunc::ONE, osg::BlendFunc::ONE), osg::StateAttribute::ON);
    
    FIXME(Do not have clamp)
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, true /*false*/);
    geom_->getOrCreateStateSet()->setAttribute(pDepth,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    geom_->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    geode->addDrawable(geom_.get());

    osg::StateSet* stateset = geom_->getOrCreateStateSet();

    osg::Program* program = new osg::Program;

    {
        osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_simple);
        program->addShader(vertex_shader);

        uni_mvp_ = new osg::Uniform("mvp_matrix",osg::Matrixf());

        stateset->addUniform(uni_mvp_);

        //if (true)
        //{
        //    coeff->setUpdateCallback(new UniformVarying);
        //    coeff->setDataVariance(osg::Object::DYNAMIC);
        //    stateset->setDataVariance(osg::Object::DYNAMIC);
        //}

    }
    
    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
    program->addShader(fragment_shader);


    program->setName("LightMap");
    stateset->setAttribute(program);

    //osg::Texture2D* texture = new osg::Texture2D(osgDB::readImageFile(textureFileName));
    //stateset->setTextureAttributeAndModes(0,texture);

    //osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
    //stateset->addUniform(baseTextureSampler);

    // create its' vertices
    //osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
    //v->setDataVariance(osg::Object::DYNAMIC);
    //geom->setVertexArray(v);

    //osg::ref_ptr<osg::Vec2Array> pMoonTexCoords = new osg::Vec2Array();
    //pMoonTexCoords->setDataVariance(osg::Object::STATIC);
    //vao_->setTexCoordArray(0, pMoonTexCoords);

    from_l_ = new osg::Vec3Array();
    //vao_->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
    geom_->setVertexAttribArray(1, from_l_.get(),osg::Array::BIND_PER_VERTEX);
    geom_->setVertexAttribNormalize(1, false);
    ldir_ = new osg::Vec3Array();
    //vao_->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);
    geom_->setVertexAttribArray(2, ldir_.get(),osg::Array::BIND_PER_VERTEX);
    geom_->setVertexAttribNormalize(2, false);
    lcolor_ = new osg::Vec3Array();
    //vao_->setVertexAttribBinding(3, osg::Geometry::BIND_PER_VERTEX);
    geom_->setVertexAttribArray(3, lcolor_.get(),osg::Array::BIND_PER_VERTEX);
    geom_->setVertexAttribNormalize(3, false);
    dist_falloff_ = new osg::Vec2Array();
    //vao_->setVertexAttribBinding(4, osg::Geometry::BIND_PER_VERTEX);
    geom_->setVertexAttribArray(4, dist_falloff_.get(),osg::Array::BIND_PER_VERTEX);
    geom_->setVertexAttribNormalize(4, false);
    cone_falloff_ = new osg::Vec2Array();
    //vao_->setVertexAttribBinding(5, osg::Geometry::BIND_PER_VERTEX);
    geom_->setVertexAttribArray(5, cone_falloff_.get(),osg::Array::BIND_PER_VERTEX);
    geom_->setVertexAttribNormalize(5, false);
    
    from_l_->setPreserveDataType(true);
    ldir_->setPreserveDataType(true);
    dist_falloff_->setPreserveDataType(true);
    cone_falloff_->setPreserveDataType(true);
    lcolor_->setPreserveDataType(true);


    if(geom_->getVertexArray())
    {
        size_t s = geom_->getVertexArray()->getNumElements();
        from_l_->assign( s, osg::Vec3(0.1,0.1,0.1) );
        ldir_->assign( s, osg::Vec3(0.2,0.2,0.2) );
        dist_falloff_->assign( s, osg::Vec2(0.3,0.3) );
        cone_falloff_->assign( s, osg::Vec2(0.4,0.4) );
        lcolor_->assign( s, osg::Vec3(0.5,0.5,0.5) );
    }

    //// fake texture
    static const unsigned fake_tex_size_ = 2;
    empty_tex_ = new osg::Texture2D;
    empty_tex_->setTextureSize( fake_tex_size_, fake_tex_size_ );
    empty_tex_->setInternalFormat( /*GL_RGBA*/GL_RGBA16F );
    empty_tex_->setNumMipmapLevels(4);
    empty_tex_->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR );
    empty_tex_->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR ); 
    empty_tex_->setWrap(osg::Texture::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    empty_tex_->setWrap(osg::Texture::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    empty_tex_->setBorderColor(osg::Vec4d(0,0,0,0));
    
    static const int
        g_nWidth = 2048/*512*/,
        g_nHeight = 2048/*512*/;
    osg::ref_ptr<Prerender> pFBOGroup = new Prerender(g_nWidth, g_nHeight);
    addChild(pFBOGroup); 
    pFBOGroup->addChild(geode);
    color_buf_ = pFBOGroup->getTexture();

    setCullCallback(new LightMapCullCallback(this));
}

// reserves textures, setups FBO, etc
void _private::InitializeTexture( unsigned tex_dim )
{
    if (tex_dim_ != tex_dim)
    {
        //IGraphicDevice * pGD = vtGlobal::pGD;
        tex_dim_ = tex_dim;
         
        color_buf_ = new osg::Texture2D;
        color_buf_->setTextureSize( tex_dim_, tex_dim_ );
        color_buf_->setInternalFormat( /*GL_RGBA*/GL_RGBA16F );
        color_buf_->setNumMipmapLevels(4);
        color_buf_->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR );
        color_buf_->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR ); 
        color_buf_->setWrap(osg::Texture::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
        color_buf_->setWrap(osg::Texture::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
        color_buf_->setBorderColor(osg::Vec4d(0,0,0,0));
        
        //Assert(fbo_->IsComplete());
    }
}

// add spot on cull pass
void _private::AddSpotLight( SpotData const & spot )
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
        inter_points_array spot_plane;
        if (!corrected_spot.angle_falloff.empty())
            spot_plane = std::move(make_intersection(world_light_pos, world_light_dir, cg::grad2rad(corrected_spot.angle_falloff.hi()), corrected_spot.dist_falloff.hi()));
        else
            spot_plane = std::move(make_intersection(world_light_pos, corrected_spot.dist_falloff.hi()));
        // add it
        if (spot_plane.size() > 2)
            add_light(spot_plane, world_light_pos, world_light_dir, corrected_spot);
    }
}

// set camera frustum and calculate all transformations
void _private::SetupProjection( cg::frustum_f const & view_frustum, float dist_max, bool night_mode )
{
    // clear vertex buffer
    // vertices_.resize(0);
    we_see_smth_ = false;
    if (!night_mode)
        return;

    // save data
    mv_ = view_frustum.camera().world_view_transform();

    // prepare frustum
    std::unique_ptr<cg::frustum_f> const frustum_split(view_frustum.clone());
    frustum_split->set_clipping(cg::range_2f(view_frustum.clipping_near(), dist_max));

    // get projection matrix
    boost::optional<matrix_4f> jlpt_mat = cg::jlpt_matrix(*frustum_split, dist_max * 0.75f, cg::range_2f(0.f, 40.f));
    we_see_smth_ = jlpt_mat;
    if (!we_see_smth_)
        return;

    // ok, we see smth
    we_see_smth_ = true;
    proj_matr_ = *jlpt_mat;
    // frustum
    lightmap_clipper_ = cg::frustum_clipper(proj_matr_);
    // texture matrix
    tex_matr_ = cg::xyzw_to_strq_remap_matrix(1.0f, 1.0f) * (proj_matr_ * mv_.inverse_matrix());
}

// render lightmap
ITexture * _private::UpdateTexture( bool enabled )
{
    FIXME( vertices_ )                        
    if (enabled && we_see_smth_ /*&& !vertices_.empty()*/)
    {

#if 0
        IGraphicDevice * pGD = vtGlobal::pGD;

        // bind and clear fbo
        pGD->BindFrameBuffer(fbo_.get());
        pGD->Viewport(0, 0, tex_dim_, tex_dim_);
        pGD->ClearColor(cg::coloraf(0, 0, 0, 0));
        pGD->Clear(GL_COLOR_BUFFER_BIT);

        // setup states
        static const vtDepthState g_depthState(false, false, GL_LEQUAL, true);
        pGD->SetDepthState(g_depthState);
        static const vtPolygonState g_polyState(true, false, 0.0f, 0.0f, false);
        pGD->SetPolygonState(g_polyState);
        //static const vtBlendState blendState(true, GL_FUNC_ADD, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        static const vtBlendState blendState(true, GL_FUNC_ADD, GL_MAX, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
        pGD->SetBlendState(blendState);

        // program
        light_prog_->SetUniform(0, &proj_matr_);
        pGD->BindProgram(light_prog_.get());

        // update vbo
        vbo_->Data(sizeof(SpotRenderVertex) * vertices_.size(), &vertices_.front());

        // bind and draw
        pGD->BindVAO(vao_.get());
        pGD->DrawArrays(GL_TRIANGLES, 0, vertices_.size());

        // build mips
        color_buf_->GenerateMipmaps();

#else

		for (unsigned i = 0; i < vertices_.size(); ++i)
		{
			SpotRenderVertex & cur_v = vertices_[i];
			point_4f vv = proj_matr_ * point_4f (cur_v.v , 0.0); 

			// force_log fl;
			
			LOG_ODS_MSG ( "SpotRenderVertex:  "  << vv.x << "  " << vv.y << " " << vv.z << " " << vv.w << "\n");
		}

        uni_mvp_->set(to_osg_matrix(proj_matr_));
#endif
        return color_buf_.get();
    }


    // empty tex
    return empty_tex_.get();
}

// texture matrix for specific split
cg::matrix_4f const & _private::GetViewTextureMatrix() const
{
    return tex_matr_;
}

// get night mode
bool _private::GetNightMode() const
{
    return we_see_smth_;
}

//
// Impl
//

void _private::add_light( std::vector<cg::point_2f> const & light_contour, cg::point_3f const & world_lightpos, cg::point_3f const & world_lightdir, SpotData const & spot )
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

    // create its' vertex
    osg::Vec3Array * paBoxPointsPos = new osg::Vec3Array;
    paBoxPointsPos->resize(vertices_.size());
    
    from_l_->clear();
    ldir_->clear();
    dist_falloff_->clear();
    cone_falloff_->clear();
    lcolor_->clear();

    from_l_->resize(vertices_.size());
    ldir_->resize(vertices_.size());
    dist_falloff_->resize(vertices_.size());
    cone_falloff_->resize(vertices_.size());
    lcolor_->resize(vertices_.size());

    if(int s = geom_->getNumPrimitiveSets()>0)
        geom_->removePrimitiveSet(0,s);

    for (unsigned i = 0; i < vertices_.size(); ++i)
    {
        SpotRenderVertex & cur_v = vertices_[i];
        paBoxPointsPos->at(i).set(cur_v.v.x, cur_v.v.y, cur_v.v.z);
        from_l_->at(i).set( cur_v.from_l.x, cur_v.from_l.y,cur_v.from_l.z );
        ldir_->at(i).set( cur_v.ldir.x, cur_v.ldir.y, cur_v.ldir.z );
        dist_falloff_->at(i).set( cur_v.dist_falloff.x, cur_v.dist_falloff.y  );
        cone_falloff_->at(i).set( cur_v.cone_falloff.x, cur_v.cone_falloff.y );
        lcolor_->at(i).set(  cur_v.lcolor.r, cur_v.lcolor.g, cur_v.lcolor.b  );
    }

    // set vertex array
    paBoxPointsPos->setDataVariance(osg::Object::DYNAMIC);
    geom_->setVertexArray(paBoxPointsPos);

	geom_->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, vertices_.size() ) );

#if 0
    const float fSqrt3 = sqrtf(3.0f);

    // create its' vertex
    // osg::Vec3Array * paBoxPointsPos = new osg::Vec3Array;
    paBoxPointsPos->resize(4);
    paBoxPointsPos->at(0).set(0.f, 0.f, +1.f);
    paBoxPointsPos->at(1).set(-1, 0.f, -1.0f);
    paBoxPointsPos->at(2).set(1, -1.0f, -1.0f);
    paBoxPointsPos->at(3).set(1, +1.0f, -1.0f);
    // set vertex array
    paBoxPointsPos->setDataVariance(osg::Object::DYNAMIC);

    geom_->setVertexArray(paBoxPointsPos);


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

    paBoxDrawElem->setDataVariance(osg::Object::DYNAMIC);
 
    geom_->addPrimitiveSet(paBoxDrawElem);

    if(geom_->getVertexArray())
    {
        size_t s = geom_->getVertexArray()->getNumElements();
        from_l_->assign( s, osg::Vec3(0.1,0.1,0.1) );
        ldir_->assign( s, osg::Vec3(0.2,0.2,0.2) );
        dist_falloff_->assign( s, osg::Vec2(0.3,0.3) );
        cone_falloff_->assign( s, osg::Vec2(0.4,0.4) );
        lcolor_->assign( s, osg::Vec3(0.5,0.5,0.5) );
    }
#endif
}