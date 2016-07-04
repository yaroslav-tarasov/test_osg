//
// Module includes
//

#include <stdafx.h>

#include <osg/LineWidth>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avLine/Ropes.h"

#include "utils/materials.h"
#include "objects/arresting_gear.h"

namespace {
char vertexShaderSource[] = 
	"#version 430 compatibility \n"
	"#extension GL_ARB_gpu_shader5 : enable \n"
    "vec3 light_dir = normalize(vec3(-0.79f, 0.39f, 0.47f)); \n"
    "uniform vec2 Settings; \n"
    "out block\n"
    "{        \n"
	"centroid out vec2 texcoord; \n"
    "centroid out float fade; \n"
	"out float light; \n"
    "} v_out;       \n"
	"void main(void) \n"
	"{ \n"
	"    float w            = dot(transpose(gl_ModelViewProjectionMatrix)[3], vec4(gl_Vertex.xyz, 1.0f));  \n"
	"    float pixel_radius = w * Settings.x;  \n"
	"    float radius       = max(Settings.y, pixel_radius);  \n"
	"    v_out.fade         = Settings.y / radius;  \n"
	"    vec3 position      = gl_Vertex.xyz + radius * gl_Normal.xyz;    \n"
    "    v_out.texcoord     = vec2( gl_MultiTexCoord0.x * (0.02f / radius), gl_MultiTexCoord0.y)   ;  \n"
	"    v_out.light        = dot(light_dir,gl_Normal.xyz) * 0.5f + 0.5f;  \n"
	"    gl_Position        = gl_ModelViewProjectionMatrix * vec4(position, 1.0f) ;\n"
	"}\n";


char fragmentShaderSource[] =  
	"#version 430 compatibility \n"
	"#extension GL_ARB_gpu_shader5 : enable \n"
    "in block\n"
    "{        \n"
    "centroid in vec2 texcoord; \n"
    "centroid in float fade; \n"
    "in float light; \n"
    "} v_in;       \n"
	"uniform sampler2D colorTex; \n"
	"out vec4 FragColor;   \n"
	"\n"
	"void main(void) \n"
	"{ \n"
	"\n"
	"    FragColor = vec4(texture2D(colorTex, v_in.texcoord.xy).xyz * v_in.light, v_in.fade);"
	"}\n";

osg::Program* createProgram( const std::string& name, const std::string& vertexSource, const std::string& fragmentSource  )
{
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->setName( name );

	osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);
	vertexShader->setName( name + "_vertex" );
	program->addShader(vertexShader.get());

	osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);
	fragmentShader->setName( name + "_fragment" );
	program->addShader(fragmentShader.get());

	return program.release();
}


}

//
// Module namespaces
//

using namespace avRopes;


//
// Ropes node constructor
//

// constructor
RopesNode::RopesNode()
	: m_fRadius(0.1f)
{
    setName("RopesNode");

	_createGeometry();

    //
    // create state set
    //

    osg::StateSet * pCurStateSet = getOrCreateStateSet();


    //osg::LineWidth* linewidth = new osg::LineWidth();
    //linewidth->setWidth(4.0f);
    //pCurStateSet->setAttributeAndModes(linewidth,osg::StateAttribute::ON);

    pCurStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Program> program;
	program = createProgram("Arresting gear",vertexShaderSource,fragmentShaderSource);
	pCurStateSet->setAttributeAndModes(program);	
	
	pCurStateSet->addUniform(new osg::Uniform("colorTex", BASE_COLOR_TEXTURE_UNIT));
	osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
	pCurStateSet->setTextureAttributeAndModes( BASE_COLOR_TEXTURE_UNIT, avCore::GetDatabase()->LoadTexture("images/wire.dds", osg::Texture::REPEAT), value );

	m_uniformSettings = new osg::Uniform("Settings", osg::Vec2f());
	pCurStateSet->addUniform(m_uniformSettings.get());

	// setup blending
	osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	pCurStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

	osg::BlendEquation* pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
	pCurStateSet->setAttributeAndModes(pBlendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
	
	setCullCallback(Utils::makeNodeCallback(this, &RopesNode::cull));
}


void RopesNode::_createArrays()
{
	_coords =  new osg::Vec3Array();
    _geom->setVertexArray(_coords);
    _coords->setDataVariance(osg::Object::DYNAMIC);

    osg::Vec4Array* color = new osg::Vec4Array(1);
    (*color)[0].set(0.95f,0.0f,0.0f,1.0f) ;

    _geom->setColorArray(color, osg::Array::BIND_OVERALL);

	_coords->setPreserveDataType(true);

	 _uv = new osg::Vec2Array;
	 _uv->setPreserveDataType(true);
	 _geom->setTexCoordArray(BASE_COLOR_TEXTURE_UNIT, _uv.get(), osg::Array::BIND_PER_VERTEX);
	 _uv->setNormalize(true);

	 _normals = new osg::Vec3Array;
	 _normals->setPreserveDataType(true);
	 _geom->setNormalArray( _normals.get(), osg::Array::BIND_PER_VERTEX);


}

void RopesNode::_clearArrays()
{
}

void RopesNode::_createGeometry()
{
	// dummy bounding box callback
	osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

	// create OSG geode with 1 drawable node
	setCullingActive(false);
	setDataVariance(osg::Object::DYNAMIC);

	_geom = new osg::Geometry;
	_geom->setComputeBoundingBoxCallback(pDummyBBCompute);

	_geom->setUseDisplayList(false);
	_geom->setUseVertexBufferObjects(true);
	_geom->setDataVariance(osg::Object::DYNAMIC);
	
	addDrawable(_geom);

	_createArrays();

}

const static unsigned sides = 8;

void  RopesNode::updateRopes( const arresting_gear::ropes_state_t& rss )
{
    using namespace arresting_gear;
    
    if (rss.size()==0)
        return;

    _coords->clear();
    _coords->resize(rss.size() * rss[0].size()  * (sides + 1));
    _uv->resize(rss.size() * rss[0].size() * (sides + 1));
    _normals->resize(rss.size() * rss[0].size() * (sides + 1));

    osg::Vec3Array::iterator it_vert( _coords->begin() );
    osg::Vec2Array::iterator it_uv  ( _uv->begin() );
    osg::Vec3Array::iterator it_normals( _normals->begin() );

    int i=0;
    for (auto it = rss.begin(); it != rss.end(); ++it)
    {
         auto  & rs = *it;
         std::vector<int>   Indices;

         auto before_end = std::prev(rs.end());
		 
		 int r = 0; auto SEGMENT_COUNT = rs.size() - 1;
		 Indices.reserve(SEGMENT_COUNT * (sides + 1) * 2);

         for (auto it_r = rs.begin(); it_r != rs.end(); ++it_r, ++r, ++i)
         {
			 int prev = (r > 0)? r - 1 : 0;
			 int next = (r < SEGMENT_COUNT)? r + 1 : SEGMENT_COUNT;

			 osg::Vec3 dir  = to_osg_vector3(cg::normalized_safe(rs[next].coord - rs[prev].coord));
			 osg::Vec3 dir0 = (dir ^ osg::Vec3(0, 0, 1)); dir0.normalize();
			 osg::Vec3 dir1 = (dir0 ^ dir);  dir1.normalize();
			 
             cg::point_3 coord (it_r->coord.x,it_r->coord.y,rs.on?it_r->coord.z:0);

			 auto it_uv0 = std::next(it_uv); 
			 auto it_normals0 = std::next(it_normals);
		     for (int a = 0; a < sides; a++)
			 {
				 float angle = a * (2 * cg::pif / sides);
				 float x = cosf(angle);
				 float y = sinf(angle);

				 osg::Vec3f normal = dir0 * x  + dir1 * y;

				 *it_vert++ = to_osg_vector3(coord);
             
				 if( it_r != before_end)
				 {
					 Indices.push_back(i * (sides + 1) + a);
					 Indices.push_back((i + 1) * (sides + 1) + a);
				 }

				 auto c = std::distance(rs.begin(),it_r);
				 (*it_uv++).set(c * 2.f, a * 1.f / sides);
				 (*it_normals++).set(normal);
			 }

			 (*it_vert++).set(to_osg_vector3(coord));
			 (*it_uv++).set((*it_uv0).x(), 1.0);
			 (*it_normals++).set(*it_normals0);


         }

         const auto cur_rope = std::distance(rss.begin(),it);
         if (cur_rope < _geom->getNumPrimitiveSets() )
            _geom->setPrimitiveSet(cur_rope, new osg::DrawElementsUInt ( osg::PrimitiveSet::TRIANGLE_STRIP, Indices.size() , (GLuint*)&Indices [ 0 ] ));
         else
            _geom->addPrimitiveSet(new osg::DrawElementsUInt ( osg::PrimitiveSet::TRIANGLE_STRIP, Indices.size() , (GLuint*)&Indices [ 0 ] ));
    }
    
    auto const prims = _geom->getNumPrimitiveSets() - rss.size();
    
    if(prims>0)
    {
       _geom->removePrimitiveSet(rss.size(), prims); 
    }

    _coords->dirty();
	_uv->dirty();
    _geom->dirtyBound();
}


void RopesNode::cull( osg::NodeVisitor * pNV )
{
	osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
	avAssert(pCV);
	
	const osg::Matrix & P = *pCV->getProjectionMatrix();
	const osg::Viewport & W = *pCV->getViewport();

#if 0
	m_fPixelSize = 1.f / 4.5/ 1200.f/*1024.f*/;
#else 
    if(W.height()>2.0 && pNV->getNodePath().size()< 5) // dirty hack with node paths
        m_fPixelSize= 1.f / P(1,1) / W.height();
#endif

	m_uniformSettings->set(osg::Vec2f(m_fPixelSize , m_fRadius));

	float fClampedPixelSize;
	osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(pNV);
	if (cullStack)
	{
		fClampedPixelSize = cullStack->clampedPixelSize(getBound()) ;
	}

}