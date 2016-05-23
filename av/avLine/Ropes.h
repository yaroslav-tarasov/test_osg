#pragma once

//
// Local includes
//

#include "avRopes.h"

//
// Local namespaces
//

namespace avRopes
{

    //
    // RopesNode class
    // Implements arresting gear ropes node
    //

    class RopesNode : public osg::Geode
				    , public IRopesNode
    {

    public:

        // constructor
        RopesNode();


        //
        // Base interface
        //
	private: // IRopesNode

		void  updateRopes( const arresting_gear::ropes_state_t& ) override;

	private:

		void cull( osg::NodeVisitor * pNV );

		void setRadius(float r){ m_fRadius = r;}
		float getRadius(float r) const { return m_fRadius; }

	private:
		void						 _createArrays();
		void                         _clearArrays();
        void                         _createGeometry();

	private:

		// data
        osg::ref_ptr<osg::Vec3Array>  _coords;
	    osg::ref_ptr<osg::Vec2Array>  _uv;
	    osg::ref_ptr<osg::Vec3Array>  _normals;
		
		float						  m_fRadius;
        float                         m_fPixelSize;
    private:
		osg::ref_ptr<osg::Uniform>   m_uniformSettings;
		osg::Geometry *              _geom;
     

    };

}