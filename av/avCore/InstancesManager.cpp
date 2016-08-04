#pragma once

#include "InstancesManager.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "InstancedAnimationManager.h"
#include "InstancesManagerImpl.h"


namespace avCore
{
    InstancesManager* CreateInstancesManager(InstancesManagerType type, osg::Node* prototype, const std::string& anim_file)
	{
		if(type == HW_ANIMATED)
			return new InstancedAnimationManager(prototype,anim_file);
		else
		    return new InstancesManagerImpl(prototype,anim_file);
	}


}