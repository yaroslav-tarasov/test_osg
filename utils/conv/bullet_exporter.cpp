#include "stdafx.h"

#include "LinearMath/btSerializer.h"
#include <btBulletDynamicsCommon.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>

#include "osg_helpers.h"
//#include "phys/bullet_helpers.h"
#include "visitors/find_node_visitor.h"

#ifndef _DEBUG
#pragma comment(lib, "osgbCollision.lib")
#else 
#pragma comment(lib, "osgbCollisiond.lib")
#endif

inline btVector3 to_bullet_vector3( osg::Vec3 const& v  )
{
    return btVector3(btScalar(v.x()), btScalar(v.y()), btScalar(v.z()));
}

//
// http://bulletphysics.org/mediawiki-1.5.8/index.php/Bullet_binary_serialization
//

btCompoundShape*  fill_cs(osg::Node* node/*, wheels_info_t& wi, compound_sensor_impl& cs*/ )
{          
    osg::Node* lod3 =  findFirstNode(node,"Lod3");

    osg::ComputeBoundsVisitor cbv;
    (lod3?lod3:node)->accept( cbv );
    const osg::BoundingBox& bb = cbv.getBoundingBox();

    float xm = bb.xMax() - bb.xMin();
    float ym = bb.yMax() - bb.yMin();
    float zm = bb.zMax() - bb.zMin();

    btVector3 offset_ = btVector3(0,/*lod3?-zm/2:*/0,0);

    auto body   = findFirstNode(lod3?lod3:node,"Body",findNodeVisitor::not_exact);

    auto wheels = findAllNodes(node,"wheel",findNodeVisitor::not_exact);

    for (auto it = wheels.begin();it != wheels.end();++it)
    {   
        if((*it)->asTransform()) // А потом они взяли и поименовали геометрию как трансформы, убил бы
        {
            FIXME(Как передать радиус?)
            //wheel_info wii((*it)->getBound().radius(),/*is_front*/false);
            //wii.trans_f_body = get_relative_transform(node,(*it)/*,body*/);
            //wi.push_back(wii);
            (*it)->setNodeMask(0);
        }
    }

    btCompoundShape*  s = new btCompoundShape;

    btCollisionShape* cs_body   = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );

    for (auto it = wheels.begin();it != wheels.end();++it)
        (*it)->setNodeMask(0xffffffff);


    s->addChildShape(btTransform(btQuaternion(0,0,0),/*to_bullet_vector3(*/offset_/*)*/),cs_body);
    
    return s;
}

bool generateBulletFile(std::string name, osg::Node* node)
{
	btDefaultSerializer*	serializer = new btDefaultSerializer();
	
	/*Convex*/btTriangleMeshShape* trimeshShape = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( node );
    
    btCompoundShape * cs =  fill_cs(node);

    static const char* nodeName = node->getName().c_str();
    serializer->registerNameForPointer(cs, nodeName);   
	
    serializer->startSerialization();
	cs->serializeSingleShape(serializer);
	serializer->finishSerialization();

	FILE* file = fopen(name.c_str(),"wb");
	fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1, file);
	fclose(file);
    
    delete cs;

	return true;
};