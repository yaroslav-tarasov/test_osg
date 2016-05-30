#include "stdafx.h"
#include "av/precompiled.h"

#include "geometry/lispsm_matrix.h"
#include "geometry/custom_matrix.h"

#include "utils/materials.h"
#include "av/avScene/Scene.h"
#include "DecalRender.h"


namespace {

char vertexShaderSource[] = 
    "#extension GL_ARB_gpu_shader5 : enable \n"
    "attribute vec3 color ; \n"
    "out block  \n"
    "{          \n"
    "    vec4 color;               \n"
    "} v_out;                      \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
	"    v_out.color = vec4(color, 1.0); \n"
    "    gl_Position =   gl_ModelViewProjectionMatrix *gl_Vertex;\n"
    "}\n";


char fragmentShaderSource[] =  {
    "#version 430 compatibility \n"
    "#extension GL_ARB_gpu_shader5 : enable \n"


    STRINGIFY(
\n
\n    in block
\n    {
\n        vec4 color;
\n    } f_in;
\n
\n    out vec4 FragColor; 
\n
\n    void main()
\n    {
\n        FragColor = f_in.color;
\n        // FragColor = vec4(1.0f, 0.0, 0.0, 1.0f);
\n    }
\n
    )                                                                                                             
};

}

namespace avCore
{

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

class DecalMapCamera : public osg::Camera
{
public:

    DecalMapCamera()

    {
        init();
    };

    osg::Texture2D        * getTexture() const { return _texture.get(); }

protected:


    void init()
    {
        _texture = creators::getTextureHolder().getDecalTexture();

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
        attach(osg::Camera::COLOR_BUFFER, _texture.get(), 0, 0, true, 8, 8);

        osg::StateSet * pSS = getOrCreateStateSet();
        pSS->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        pSS->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        pSS->setMode(GL_DEPTH_CLAMP_NV, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        pSS->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        
        FIXME( invalid operation after BlendEquation)
        osg::BlendEquation * pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD,osg::BlendEquation::FUNC_ADD);
        pSS->setAttributeAndModes(pBlendEquation, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        pSS->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::ONE,osg::BlendFunc::ZERO, osg::BlendFunc::ONE, osg::BlendFunc::ZERO), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

        pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    }

    void traverse(osg::NodeVisitor& nv)
    {
        if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
            osg::Group::traverse(nv);
    }

    osg::ref_ptr<osg::Texture2D>         _texture;
};

class _private : public DecalRenderer
{
 
    friend IDecalRendererPtr createDecalRenderer(osg::Group * sceneRoot);

public: // IDecalRenderer

    // set camera frustum and calculate all transformations
    // also clear collected lights

    // update uniforms
    void                        UpdateTextureMatrix( bool enabled );
    
    void                        SetupProjection( cg::frustum_f const & view_frustum, cg::range_2f const & z_range );

    virtual void                traverse(osg::NodeVisitor& nv);    

    struct LineVertex
    {
        cg::point_3f v;
        cg::colorab c;
        LineVertex( cg::point_2f const & v, cg::colorab const & c ) : v(v), c(c) {}
        LineVertex() {}
    };

    void                                setMVP(const osg::Matrixf& mvp);
    osg::Matrix                         getProjectionMatrix() {return to_osg_matrix(proj_matr_);};
	
	//
    // DecalRenderer interface
	//
private:
    void SetVisible( bool vis ) override;
    void Clear()                override;
    void AddPolyline( std::vector<cg::point_2f> const & positions, cg::colorf const & col, float w ) override;
	void AddGeometry( osg::Geometry * g );

protected:

    _private(osg::Group * sceneRoot);

    void							    _createArrays();
    osg::Geometry *                     _createGeometry();
    void                                _clearArrays();
    void                                _commitData();
    
    
    void cull( osg::NodeVisitor * nv );

protected:
    unsigned                             tex_dim_;
    bool                                 we_see_smth_;

    cg::transform_4f                     mv_;

    cg::matrix_4f                        proj_matr_;
    cg::matrix_4f                        tex_matr_;

    ITexturePtr                          color_buf_;
    unsigned                             _tex_dim;
    
    std::vector<LineVertex>              vertices_;
    std::vector<unsigned>                lines_indices_;

    osg::ref_ptr<DecalMapCamera>         _camera;
    
    osg::ref_ptr<osg::Uniform>           _decalmapMatrix;

    osg::ref_ptr<osg::Geometry>          _geom ;
    osg::ref_ptr<osg::Geode>             _geode ;
	osg::ref_ptr<osg::Geode>             _geodeExt;
    osg::ref_ptr<osg::StateSet>          _stateset;

    osg::Vec3Array  *                    geom_array_   ;
    osg::ref_ptr<osg::Vec3Array>         lcolor_       ;
    osg::ref_ptr<osg::UIntArray>         indices_;

    bool                                 dirty_;

private:
    bool                                 _NeedToUpdateFBO;
	boost::optional<std::pair<matrix_4f,matrix_4f>> clpt_mat_;
};

// create
IDecalRendererPtr createDecalRenderer(osg::Group * sceneRoot)
{
    return new _private(sceneRoot);
}

void _private::SetVisible( bool vis )
{

}

void _private::Clear()
{

}
    
void _private::AddGeometry( osg::Geometry * g )
{
	std::string ls = boost::to_lower_copy(g->getStateSet()->getName());
	g->setStateSet(_stateset);
	
	auto color = osg::Vec3(0.0f,0.0f,0.0f);

	if(ls.find("red") != std::string::npos)
	{
		color = osg::Vec3(1.0f,0.0f,0.0f);
	}
	else if (ls.find("yellow") != std::string::npos)
	{
		color = osg::Vec3(1.0f,1.0f,0.0f);
	}
	else if (ls.find("white") != std::string::npos)
	{
		color = osg::Vec3(1.0f,1.0f,1.0f);
	}


	osg::Vec3Array*  acolor = new osg::Vec3Array();
	acolor->push_back(color);
	acolor->setPreserveDataType(true);
	g->setVertexAttribArray(1, acolor,osg::Array::BIND_OVERALL);

	_geodeExt->addDrawable(g);
	g->setUseDisplayList(false);
	g->setUseVertexBufferObjects(true);
	g->setDataVariance(osg::Object::DYNAMIC);
}

void _private::AddPolyline( std::vector<cg::point_2f> const & positions, cg::colorf const & col, float w )
{
    if (positions.size() < 2)
        return;

    const cg::colorab cur_color(col);
    static std::vector<LineVertex> new_va;
    static std::vector<unsigned> new_ia;
    new_va.resize(2 * positions.size());
    new_ia.resize(6 * (positions.size() - 1));

    // verts
    for (unsigned i = 0; i < positions.size(); ++i)
    {
        cg::point_2f dir;
        if (i == 0)
            dir = cg::normalized_safe(positions[1] - positions[0]);
        else if (i == positions.size() - 1)
            dir = cg::normalized_safe(positions[positions.size() - 1] - positions[positions.size() - 2]);
        else
        {
            const auto prev = cg::normalized_safe(positions[i] - positions[i - 1]);
            const auto next = cg::normalized_safe(positions[i + 1] - positions[i]);
            dir = prev + next;
            dir /= dir * prev;
        }
        const cg::point_2f right_vec(w * dir.y, -w * dir.x);
        new_va[i * 2 + 0] = LineVertex(positions[i] - right_vec, cur_color);
        new_va[i * 2 + 1] = LineVertex(positions[i] + right_vec, cur_color);
    }
    // idx
    const unsigned vtx_shift = vertices_.size();
    for (unsigned i = 0; i < positions.size() - 1; ++i)
    {
        new_ia[i * 6 + 0] = vtx_shift + 2 * i + 0;
        new_ia[i * 6 + 1] = vtx_shift + 2 * i + 1;
        new_ia[i * 6 + 2] = vtx_shift + 2 * i + 2;
        new_ia[i * 6 + 3] = vtx_shift + 2 * i + 2;
        new_ia[i * 6 + 4] = vtx_shift + 2 * i + 1;
        new_ia[i * 6 + 5] = vtx_shift + 2 * i + 3;
    }

    vertices_.insert(vertices_.end(), new_va.begin(), new_va.end());
    lines_indices_.insert(lines_indices_.end(), new_ia.begin(), new_ia.end());
#if 0
    lines_updated_ = true;
#endif

    dirty_ = true;
}

void _private::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
    {
        Group::traverse(nv);
        return;
    }

    osgUtil::CullVisitor * cv = static_cast<osgUtil::CullVisitor*>(&nv);
    const  osg::Camera* cam = avScene::GetScene()->getCamera();

    double  fLeft(-1.0f), fRight(+1.0f), fBottom(-1.0f), fTop(+1.0f), fNear(1.0f), fFar(1000.0f);

    cam->getProjectionMatrixAsFrustum(fLeft, fRight, fBottom, fTop, fNear, fFar);

    osg::Vec3f eye, center, up;
    cam->getViewMatrixAsLookAt(eye, center, up);

    const cg::point_3f  vPosition(from_osg_vector3(nv.getViewPoint()));
    auto dir = cg::polar_point_3f(from_osg_vector3(center - eye));
    const cg::cprf      rOrientation = cg::cprf(dir.course,dir.pitch,0);

    cg::frustum_perspective_f frustum_(cg::camera_f(), cg::range_2f(fNear, fFar), 2.0f * cg::rad2grad(atan(fRight / fNear)), 2.0f * cg::rad2grad(atan(fTop / fNear)));
    frustum_.camera() = cg::camera_f(vPosition, rOrientation);
    
    const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
    FIXME(Дистанция хардкод)
    SetupProjection(frustum_, cg::range_2f(500.f, 2250.f)); 

    cull( cv );
    
    UpdateTextureMatrix(true);


    // set matrices from frustum calculator to camera
    if(clpt_mat_)
    {
        _camera->setViewMatrix(to_osg_matrix(clpt_mat_->first.transpose()));
        _camera->setProjectionMatrix(to_osg_matrix(clpt_mat_->second.transpose()));
    }

    // go farther
    _camera->accept(*cv);

}

osg::Geometry * _private::_createGeometry()
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
_private::_private(osg::Group * sceneRoot)
    : tex_dim_(0)
    , we_see_smth_(false)
    , _NeedToUpdateFBO(false)
    , dirty_ (false)
{

    _decalmapMatrix = new osg::Uniform("decal_matrix",osg::Matrixf());
    sceneRoot->getOrCreateStateSet()->addUniform(_decalmapMatrix.get());

	_geodeExt = new osg::Geode;
    _geode = new osg::Geode;

    _geom = _createGeometry();

    _geode->addDrawable(_geom.get());


	osg::Group* gr = new osg::Group;
    _stateset = gr->getOrCreateStateSet();

    osg::Program* program = new osg::Program;

    osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
    program->addShader(vertex_shader);
    
    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
    program->addShader(fragment_shader);


    program->setName("DecalMap");
    _stateset->setAttribute(program);

    _createArrays();
    
    _camera = new DecalMapCamera();
    _camera->setCullCallback ( new CameraCullCallback ( _NeedToUpdateFBO ) );
	
	const osg::Quat quat0(osg::inDegrees(0.0f), osg::X_AXIS,                      
		osg::inDegrees(0.f)  , osg::Y_AXIS,
		osg::inDegrees(0.f)  , osg::Z_AXIS ); 
	 
	
	osg::PositionAttitudeTransform* patExt = new osg::PositionAttitudeTransform;
	patExt->addChild(_geodeExt);
	patExt->setAttitude(quat0);

	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	pat->addChild(_geode);
	pat->setAttitude(osg::Quat (osg::inDegrees(180.f)  , osg::Z_AXIS ));
    
	gr->addChild(pat);
	gr->addChild(patExt);
	
	_camera->addChild(gr);
    
	color_buf_ = _camera->getTexture();

}
 


// set camera frustum and calculate all transformations
void _private::SetupProjection( cg::frustum_f const & view_frustum, cg::range_2f const & z_range )
{
    // clear vertex buffer
#if 0
    vertices_.resize(0);
#endif
    clpt_mat_.reset();

    // save data
    mv_ = view_frustum.camera().world_view_transform();

    // prepare frustum
    std::unique_ptr<cg::frustum_f> const frustum_split(view_frustum.clone());
    frustum_split->set_clipping(cg::range_2f(view_frustum.clipping_near(), z_range.hi()));

    // get projection matrix
    clpt_mat_ = cg::clpt_matrix(*frustum_split, z_range.lo(), cg::range_2f(0.f, 0.5f));
	
    we_see_smth_ = clpt_mat_;
    if (!we_see_smth_)
        return;

    // ok, we see smth
    we_see_smth_ = true;
    proj_matr_ = (*clpt_mat_).second * (*clpt_mat_).first;
    // texture matrix
    tex_matr_ = cg::xyzw_to_strq_remap_matrix(1.0f, 1.0f) * (proj_matr_ * mv_.inverse_matrix());
}

// update uniform
void _private::UpdateTextureMatrix( bool enabled )
{
    if (enabled && we_see_smth_ && !vertices_.empty())
    {
        _decalmapMatrix->set(to_osg_matrix(tex_matr_.transpose()));
    }
}




void _private::_createArrays()
{

    lcolor_ = new osg::Vec3Array();
    _geom->setVertexAttribArray(1, lcolor_.get(),osg::Array::BIND_PER_VERTEX);
    _geom->setVertexAttribNormalize(1, false);
    
    indices_ = new osg::UIntArray();
    
    lcolor_->setPreserveDataType(true);
    indices_->setPreserveDataType(true);

    _geom->addPrimitiveSet(new osg::DrawElementsUInt() );
}

void _private::_clearArrays()
{
    lcolor_->clear();
    indices_->clear();
}

//
// Impl
//


void _private::_commitData()
{
    geom_array_ = static_cast<osg::Vec3Array *>(_geom->getVertexArray());
    geom_array_->resize(0);
    
    _clearArrays();

    if(vertices_.size()==0)
    {
        _geom->setPrimitiveSet(0, new osg::DrawArrays() );
        _geom->removePrimitiveSet(0);
        return;
    }

    geom_array_->resize(vertices_.size());

    lcolor_->resize(vertices_.size());
    indices_->resize(lines_indices_.size());

    for (unsigned i = 0; i < vertices_.size(); ++i)
    {
        LineVertex & cur_v = vertices_[i];
        geom_array_->at(i).set(cur_v.v.x, cur_v.v.y, cur_v.v.z);
        lcolor_->at(i).set(  cur_v.c.r, cur_v.c.g, cur_v.c.b  );
    }

    for (unsigned i = 0; i < lines_indices_.size(); ++i)
    {
        indices_->at(i) = lines_indices_[i];
    }


    _geom->setPrimitiveSet(0, new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indices_->size(), &indices_->front()) );
    indices_->dirty();
    lcolor_->dirty();
    geom_array_->dirty();
}



void _private::cull( osg::NodeVisitor * nv )
{
    using namespace avScene;

    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();

    avScene::Scene * scene = GetScene();

    if (scene == NULL)
        return;

    if(dirty_)
        _commitData();

}





} //ns avCore