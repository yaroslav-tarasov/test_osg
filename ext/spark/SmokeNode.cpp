#include "stdafx.h"
#include "SmokeNode.h"

#ifndef Q_MOC_RUN
#include <SPK.h>
#include <SPK_GL.h>
#endif

#include "SparkDrawable.h"
#include "av/Environment.h"

struct SmokeNode::SafeEnvironment
{
	avCore::Environment::EnvironmentParameters env;
};

/* SmokeNode */

SmokeNode::SmokeNode()
	: gravity_(boost::none)
	, env_    (new SafeEnvironment)
{
}
    
SmokeNode::SmokeNode( const SmokeNode& copy,const osg::CopyOp& copyop )
	: osg::Geode(copy, copyop)
    , gravity_  (copy.gravity_)
    , env_      (new SafeEnvironment)
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
	if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
	{
		const avCore::Environment::EnvironmentParameters & cEnvironmentParameters = avCore::GetEnvironment()->GetEnvironmentParameters(); 
	    if (env_->env.WindDirection != cEnvironmentParameters.WindDirection )
			gravity_ = to_osg_vector3(env_->env.WindSpeed * env_->env.WindDirection);
		
	    env_->env = cEnvironmentParameters;

		if(gravity_)
			 setGravity (*gravity_);
	}

     osg::Geode::traverse(nv);
} 