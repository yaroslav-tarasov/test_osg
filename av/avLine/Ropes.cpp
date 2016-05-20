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

//
// Module namespaces
//

using namespace avRopes;


//
// Ropes node constructor
//

// constructor
RopesNode::RopesNode()
{
    setName("RopesNode");

	_createGeometry();

    //
    // create state set
    //

    osg::StateSet * pCurStateSet = getOrCreateStateSet();

    osg::StateSet* stateset = _geom->getOrCreateStateSet();
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(4.0f);
    pCurStateSet->setAttributeAndModes(linewidth,osg::StateAttribute::ON);
    pCurStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    _geom->setStateSet(stateset);

	
	setCullCallback(Utils::makeNodeCallback(this, &RopesNode::cull));
    // exit
    return;
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

// update
void RopesNode::cull( osg::NodeVisitor * pNV )
{
	osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
	avAssert(pCV);
    
	const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();
	const osg::Matrixd mProjection  = *pCV->getProjectionMatrix();

	const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
	const avCore::Environment::EnvironmentParameters  & cEnvironmentParameters  = avCore::GetEnvironment()->GetEnvironmentParameters();

	_clearArrays();
	
	// feed it to gpu
#if 0
	pos_time_unit_->resize(cpu_queue.size()); 
    prev_pos_->resize(cpu_queue.size());
    randoms_->resize(cpu_queue.size());

	
	
	size_t i =0;
	for (auto part = cpu_queue.begin(), it_end = cpu_queue.end(); part != it_end; ++part, ++i)
	{
		// result_aabb |= part->cur_pos();
		auto cpu_p = *part;
		pos_time_unit_->at(i).set(cpu_p.cur_pos().x,cpu_p.cur_pos().y,cpu_p.cur_pos().z,cpu_p.t());
		prev_pos_->at(i).set(cpu_p.prev_pos.x, cpu_p.prev_pos.y, cpu_p.prev_pos.z);
	}

    if(i>0)
    {
        pos_time_unit_->dirty();
        prev_pos_->dirty();
        randoms_->dirty();

        _drawArrays->setCount(cpu_queue.size());
    }
#endif

}


void  RopesNode::updateRopes( const arresting_gear::ropes_state_t& rss )
{
    using namespace arresting_gear;
    
    _coords->clear();
    _coords->resize(rss.size() * rss[0].size());
    
    osg::Vec3Array::iterator it_vert( _coords->begin() );

    int i=0;
    for (auto it = rss.begin(); it != rss.end(); ++it)
    {
         auto  & rs = *it;
         std::vector<int>   Indices;
         auto before_end = std::prev(rs.end());
         for (auto it_r = rs.begin(); it_r != rs.end(); ++it_r)
         {
             *it_vert++ = to_osg_vector3(it_r->coord);
             
             if( it_r != before_end)
             {
                 Indices.push_back(i);
                 Indices.push_back(i + 1);
             }

             i++;
         }

         const auto cur_rope =std::distance(rss.begin(),it);
         if (cur_rope < _geom->getNumPrimitiveSets() )
            _geom->setPrimitiveSet(cur_rope, new osg::DrawElementsUInt ( osg::PrimitiveSet::LINES, Indices.size(), (GLuint*)&Indices [ 0 ] ));
         else
            _geom->addPrimitiveSet(new osg::DrawElementsUInt ( osg::PrimitiveSet::LINES, Indices.size(), (GLuint*)&Indices [ 0 ] ));
    }

    _coords->dirty();
    _geom->dirtyBound();
}