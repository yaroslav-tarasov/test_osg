#include "stdafx.h"

#include "light_maps.h"

#include "geometry/lispsm_matrix.h"

#include "av/Prerender.h"

#define GL_RGBA16F 0x881A
char vertexShaderSource_simple[] = 
    "uniform vec4 coeff; \n"
    "attribute vec3 from_l : ATTR2; \n"
    "attribute vec3 l_dir : ATTR3; \n"
    "attribute vec3 l_color : ATTR4; \n"
    "attribute vec2 dist_falloff : ATTR5; \n"
    "attribute vec2 cone_falloff : ATTR6; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_TexCoord[0] = gl_Vertex; \n"
    "    vec4 vert = gl_Vertex; \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vert;\n"
    "}\n";


char fragmentShaderSource[] = 
    "uniform sampler2D baseTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = texture2D( baseTexture, gl_TexCoord[0].xy); \n"
    "}\n";

class LightMapRenderer_priv : public LightMapRenderer
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


protected:

    LightMapRenderer_priv();

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

};

// create
ILightMapRendererPtr createLightMapRenderer()
{
    return new LightMapRenderer_priv();
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

class LightMapCullCallback : public osg::NodeCallback
{
    virtual void operator()( osg::Node * pNode, osg::NodeVisitor * pNV )
    {
        osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
        osg::Group * pNodeAsGroup = static_cast<osg::Group *>(pNode);
        avAssert(pCV && pNodeAsGroup);

        Prerender * pPrerenderNode = static_cast<Prerender *>(pNodeAsGroup->getChild(0));
        avAssert(pPrerenderNode);

        const osg::Matrixd & mProjection = *pCV->getProjectionMatrix();
        osg::Vec3d vEyeLTPPos = pCV->getEyeLocal()/* * utils::GetCoordinateSystem()->GetLCS2LTPMatrix()*/;
        osg::Matrixd mModelView = *pCV->getModelViewMatrix();

        osg::Matrixd InvMVP = osg::Matrixd::inverse(mModelView * mProjection);

        pPrerenderNode->setProjectionMatrix(mProjection);
        pPrerenderNode->setViewMatrix(mModelView);

        // go down
        pNV->traverse(*pNode);
    }
};

// ctor
LightMapRenderer_priv::LightMapRenderer_priv()
    : tex_dim_(0)
    , we_see_smth_(false)
    , lightmap_clipper_(cg::matrix_4f())
{
    osg::Geode* geode = new osg::Geode;

    osg::ref_ptr<osg::Geometry> vao_ = new osg::Geometry();
    vao_->setUseDisplayList(false);
    vao_->setUseVertexBufferObjects(true);
    vao_->setDataVariance(osg::Object::DYNAMIC);
    geode->addDrawable(vao_.get());

    osg::StateSet* stateset = vao_->getOrCreateStateSet();

    //osg::Program* program = new osg::Program;
    //stateset->setAttribute(program);


    //{
    //    osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_simple);
    //    program->addShader(vertex_shader);

    //    //osg::Uniform* coeff = new osg::Uniform("coeff",osg::Vec4(1.0,-1.0f,-1.0f,1.0f));

    //    //stateset->addUniform(coeff);

    //    //if (dynamic)
    //    //{
    //    //    coeff->setUpdateCallback(new UniformVarying);
    //    //    coeff->setDataVariance(osg::Object::DYNAMIC);
    //    //    stateset->setDataVariance(osg::Object::DYNAMIC);
    //    //}

    //}
    //
    //osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
    //program->addShader(fragment_shader);

    //osg::Texture2D* texture = new osg::Texture2D(osgDB::readImageFile(textureFileName));
    //stateset->setTextureAttributeAndModes(0,texture);

    osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
    stateset->addUniform(baseTextureSampler);

    // create its' vertices
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
    v->setDataVariance(osg::Object::DYNAMIC);
    vao_->setVertexArray(v);

    //osg::ref_ptr<osg::Vec2Array> pMoonTexCoords = new osg::Vec2Array();
    //pMoonTexCoords->setDataVariance(osg::Object::STATIC);
    //vao_->setTexCoordArray(0, pMoonTexCoords);

    osg::ref_ptr<osg::Vec3Array> from_l = new osg::Vec3Array();
    //vao_->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);
    vao_->setVertexAttribArray(2, from_l.get(),osg::Array::BIND_PER_VERTEX);
    osg::ref_ptr<osg::Vec3Array> ldir = new osg::Vec3Array();
    //vao_->setVertexAttribBinding(3, osg::Geometry::BIND_PER_VERTEX);
    vao_->setVertexAttribArray(3, ldir.get(),osg::Array::BIND_PER_VERTEX);
    osg::ref_ptr<osg::Vec3Array> lcolor = new osg::Vec3Array();
    //vao_->setVertexAttribBinding(4, osg::Geometry::BIND_PER_VERTEX);
    vao_->setVertexAttribArray(4, lcolor.get(),osg::Array::BIND_PER_VERTEX);
    osg::ref_ptr<osg::Vec2Array> dist_falloff = new osg::Vec2Array();
    //vao_->setVertexAttribBinding(5, osg::Geometry::BIND_PER_VERTEX);
    vao_->setVertexAttribArray(5, dist_falloff.get(),osg::Array::BIND_PER_VERTEX);
    osg::ref_ptr<osg::Vec2Array> cone_falloff = new osg::Vec2Array();
    //vao_->setVertexAttribBinding(6, osg::Geometry::BIND_PER_VERTEX);
    vao_->setVertexAttribArray(6, cone_falloff.get(),osg::Array::BIND_PER_VERTEX);

    addChild(geode);

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
        g_nWidth = 512,
        g_nHeight = 512;
    osg::ref_ptr<Prerender> pReflFBOGroup = new Prerender(g_nWidth, g_nHeight);
    // m_groupMainReflection = pReflFBOGroup.get();
    addChild(pReflFBOGroup); 
    //pReflFBOGroup->addChild(geode);

    //setCullCallback(new LightMapCullCallback());
}

// reserves textures, setups FBO, etc
void LightMapRenderer_priv::InitializeTexture( unsigned tex_dim )
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
void LightMapRenderer_priv::AddSpotLight( SpotData const & spot )
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
void LightMapRenderer_priv::SetupProjection( cg::frustum_f const & view_frustum, float dist_max, bool night_mode )
{
    // clear vertex buffer
    vertices_.resize(0);
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
ITexture * LightMapRenderer_priv::UpdateTexture( bool enabled )
{
#if 0
    if (enabled && we_see_smth_ && !vertices_.empty())
    {
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
        return color_buf_.get();
    }
#endif

    // empty tex
    return empty_tex_.get();
}

// texture matrix for specific split
cg::matrix_4f const & LightMapRenderer_priv::GetViewTextureMatrix() const
{
    return tex_matr_;
}

// get night mode
bool LightMapRenderer_priv::GetNightMode() const
{
    return we_see_smth_;
}

//
// Impl
//

void LightMapRenderer_priv::add_light( std::vector<cg::point_2f> const & light_contour, cg::point_3f const & world_lightpos, cg::point_3f const & world_lightdir, SpotData const & spot )
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