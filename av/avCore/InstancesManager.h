#pragma once

#include "InstancedData.h"

namespace avCore
{
	enum InstancesManagerType
	{
		HW_ANIMATED,
		HW_DEFAULT
	};
	
	
	struct  InstancesManager : public osg::Object    
	{
		struct InstancedNodeType
		{
			InstancedNodeType()
				: parented(false)
			{}

			InstancedNodeType(osg::Node* first, osg::Node* second )
				: parented(false)
				, first   (first)
				, second  (second)
			{}

			osg::ref_ptr<osg::Node> first;
		    osg::ref_ptr<osg::Node> second;
			bool                    parented;
		};
		
		InstancesManager::InstancesManager()
		{}

		InstancesManager::InstancesManager(const InstancesManager& object,const osg::CopyOp& copyop)
			: osg::Object(object,copyop)
		{}

		typedef std::vector< InstancedNodeType > InstancedNodesVectorType;
		typedef std::vector<osg::Matrixf>               InstancedDataType;
        
		virtual void                 addMatrix(const osg::Matrixf& matrix) = 0;
		virtual osg::Matrixf         getMatrix(size_t index) const         = 0;
		virtual void                 clearMatrices()					   = 0;
        virtual osg::Node*           getMainNode()                         = 0;
        virtual osg::Node *          getObjectInstance()                   = 0;                   
        virtual void                 commitInstancesPositions()            = 0;
	};

	InstancesManager* CreateInstancesManager(InstancesManagerType type, osg::Node* prototype, const std::string& anim_file);
}