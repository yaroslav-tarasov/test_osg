#include "stdafx.h"

#include "LinearMath/btSerializer.h"
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>

#ifndef _DEBUG
//#pragma comment(lib, "osgwTools.lib")
//#pragma comment(lib, "osgbDynamics.lib")
//#pragma comment(lib, "osgbInteraction.lib")
#pragma comment(lib, "osgbCollision.lib")
#else 
// #pragma comment(lib, "osgwToolsd.lib")
//#pragma comment(lib, "osgbDynamicsd.lib")
//#pragma comment(lib, "osgbInteractiond.lib")
#pragma comment(lib, "osgbCollisiond.lib")
#endif

//
// http://bulletphysics.org/mediawiki-1.5.8/index.php/Bullet_binary_serialization
//

bool generateBulletFile(std::string name, osg::Node* body)
{
	btDefaultSerializer*	serializer = new btDefaultSerializer();
	
	auto trimeshShape = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( body );

	serializer->startSerialization();
	trimeshShape->serializeSingleShape(serializer);
	serializer->finishSerialization();

	FILE* file = fopen(name.c_str(),"wb");
	fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1, file);
	fclose(file);

	return true;
};