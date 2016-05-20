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


    public:

        // special culling method
		void cull( osg::NodeVisitor * pNV );

	private:
		void						 _createArrays();
		void                         _clearArrays();
        void                         _createGeometry();


	private: // IRopesNode
		
       void  updateRopes( const arresting_gear::ropes_state_t& ) override;

	private:

		// data
        osg::ref_ptr<osg::Vec3Array>  _coords;

    private:

        // AABB data for clipping
        osg::BoundingBox	         _aabbEllipsoid;
		osg::Geometry *              _geom;
    };

}