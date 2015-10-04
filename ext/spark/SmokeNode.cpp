#include "stdafx.h"
#include "SmokeNode.h"

#ifndef Q_MOC_RUN
#include <SPK.h>
#include <SPK_GL.h>
#endif

#include "SparkDrawable.h"


/* SmokeNode */

SmokeNode::SmokeNode()
	: gravity_(boost::none)
{
}
    
SmokeNode::SmokeNode( const SmokeNode& copy,const osg::CopyOp& copyop )
	: osg::Geode(copy, copyop)

{}

SmokeNode::~SmokeNode()
{
}


void SmokeNode::setGravity (const osg::Vec3f& g)
{
	
	auto d = getNumDrawables()>0?dynamic_cast<SparkDrawable*>(getDrawable(0)):nullptr;
	if(d)
	{
		unsigned int  sys_num =   d->getNumParticleSystems();
    	SPK::System*  sys		  = d->getProtoSystem();
		if(sys && sys_num>0 && sys->getGroups().size()>0  )
		{
			SPK::System*  sys     = d->getParticleSystem( 0 ) ;
			SPK::Group*   group   = sys->getGroup(0);

			group->setGravity( SPK::Vector3D(g.x(), g.y(), g.z()) );
			if(gravity_)
				gravity_.reset();
		}
	}
	else
	{
		gravity_  = g;
	}


}

void  SmokeNode::traverse(osg::NodeVisitor& nv)
{
	 if(gravity_)
		 setGravity (*gravity_);

     osg::Geode::traverse(nv);
} 