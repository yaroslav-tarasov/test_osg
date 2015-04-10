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

namespace 
{
    template <typename T, size_t N>
    size_t array_size(T (&)[N]) {
        return N;  // size of array
    }

    struct  nodes_hider 
    {   
        
        nodes_hider(osg::Node * base, std::vector< std::string >& a)
           : node_(base)
        {
            std::for_each(a.begin(),a.end(),[=](const std::string name){add_nodes(name);});

            for (auto it = nodes_list_.begin();it != nodes_list_.end();++it)
            {   
                if((*it)->asTransform()) // ј потом они вз€ли и поименовали геометрию как трансформы, убил бы
                {
                    (*it)->setNodeMask(0);
                }
            }
        }

        ~nodes_hider()
        {
            for (auto it = nodes_list_.begin();it != nodes_list_.end();++it)
                (*it)->setNodeMask(0xffffffff);
        }

    protected:
        
        inline void add_nodes(const std::string& name)
        {
            auto ns = findNodes(node_,name,findNodeVisitor::not_exact);
            nodes_list_.insert(nodes_list_.end(), ns.begin(), ns.end());
        }

    private:
        osg::Node* node_;
        findNodeVisitor::nodeListType  nodes_list_;
        std::vector< std::string > a;
    };
    
}

namespace vehicle
{
    btCompoundShape*  fill_cs(osg::Node* node, cg::point_3& offset )
    {          
        osg::Node* lod3 =  findFirstNode(node,"Lod3");

        osg::ComputeBoundsVisitor cbv;
        (lod3?lod3:node)->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();

        btVector3 offset_ = btVector3(0,/*lod3?-zm/2:*/0,0);
        offset = cg::point_3(0,0,0);

        auto body   = findFirstNode(lod3?lod3:node,"Body",findNodeVisitor::not_exact);

        auto wheels = findNodes(node,"wheel",findNodeVisitor::not_exact);

        for (auto it = wheels.begin();it != wheels.end();++it)
        {   
            if((*it)->asTransform()) // ј потом они вз€ли и поименовали геометрию как трансформы, убил бы
            {
                (*it)->setNodeMask(0);
            }
        }

        btCompoundShape*  s = new btCompoundShape;

    #if 0
        btCollisionShape* cs_body   = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
    #else    
        btCollisionShape* cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::Y,3 );
    #endif

        for (auto it = wheels.begin();it != wheels.end();++it)
            (*it)->setNodeMask(0xffffffff);


        s->addChildShape(btTransform(btQuaternion(0,0,0),/*to_bullet_vector3(*/offset_/*)*/),cs_body);
    
        return s;
    }

}


namespace airplane
{

    btCompoundShape*  fill_cs(osg::Node* node, cg::point_3& offset)
    {  
            osg::ComputeBoundsVisitor cbv;
            node->accept( cbv );
            const osg::BoundingBox& bb = cbv.getBoundingBox();

            float xm = abs(bb.xMax() - bb.xMin());
            float ym = abs(bb.yMax() - bb.yMin());
            float zm = abs(bb.zMax() - bb.zMin());

            float dx = abs(bb.xMax()) - xm / 2.f;
            float dy = abs(bb.yMax()) - ym / 2.f;
            float dz = abs(bb.zMax()) - zm / 2.f;

            btVector3 offset_ = btVector3(0,/*lod3?-zm/2:*/-dz,0);
            offset = cg::point_3(0,-dz,0);

            auto body   = findFirstNode(node,"Body",findNodeVisitor::not_exact);
            
            const char* nn[] = { "shassi", "rotor" };
            nodes_hider nh(node,std::vector< std::string >( nn, nn + array_size(nn) ));

            btCollisionShape* cs_body = nullptr;
            btCompoundShape*  s = new btCompoundShape;

    #if 0
            FIXME("ѕри btTriMeshCollisionShape проваливаемс€ при столкновени€х, btConvexTriMeshCollisionShape не сериализуетс€" )
            cs_body   = /*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
    #else
            cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::Y,3 );
    #endif

            if(cs_body)
                s->addChildShape(btTransform(btQuaternion(0,0,0),offset_),cs_body);

           return s;
    }

}

namespace heli
{

    btCompoundShape*  fill_cs(osg::Node* node, cg::point_3& offset)
    {  
        osg::ComputeBoundsVisitor cbv;
        node->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = abs(bb.xMax() - bb.xMin());
        float ym = abs(bb.yMax() - bb.yMin());
        float zm = abs(bb.zMax() - bb.zMin());

        float dx = abs(bb.xMax()) - xm / 2.f;
        float dy = abs(bb.yMax()) - ym / 2.f;
        float dz = abs(bb.zMax()) - zm / 2.f;

        btVector3 offset_ = btVector3(0,/*lod3?-zm/2:*/-dz,0);
        offset = cg::point_3(0,-dz,0);

        auto body   = findFirstNode(node,"Body",findNodeVisitor::not_exact);

        const char* nn[] = { "shassi", "rotor" };
        nodes_hider nh(node,std::vector< std::string >( nn, nn + array_size(nn) ));
        
        btCollisionShape* cs_body = nullptr;
        btCompoundShape*  s = new btCompoundShape;

#if 0
        FIXME("ѕри btTriMeshCollisionShape проваливаемс€ при столкновени€х, btConvexTriMeshCollisionShape не сериализуетс€" )
            cs_body   = /*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
#else
        cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::Y,3 );
#endif

        if(cs_body)
            s->addChildShape(btTransform(btQuaternion(0,0,0),offset_),cs_body);

        return s;
    }

}

namespace default
{
    btCompoundShape*  fill_cs(osg::Node* node, cg::point_3& offset )
    {          
        osg::Node* lod3 =  findFirstNode(node,"Lod3");

        osg::ComputeBoundsVisitor cbv;
        (lod3?lod3:node)->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();

        btVector3 offset_ = btVector3(0,/*lod3?-zm/2:*/0,0);
        offset = cg::point_3(0,0,0);
       
        const char* nn[] = { "shassi", "rotor" };
        nodes_hider nh(node,std::vector< std::string >( nn, nn + array_size(nn) ));

        
        btCompoundShape*  s = new btCompoundShape;

#if 0
        btCollisionShape* cs_body   = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( lod3?lod3:node );
#else    
        btCollisionShape* cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( lod3?lod3:node,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::Y,3 );
#endif

        s->addChildShape(btTransform(btQuaternion(0,0,0),/*to_bullet_vector3(*/offset_/*)*/),cs_body);

        return s;
    }

}

btCompoundShape* fill_cs(osg::Node* node, cg::point_3& offset)
{
    auto lod3 =  findFirstNode(node,"lod3");

    bool airplane = findFirstNode(node ,"shassi_",findNodeVisitor::not_exact)!=nullptr;
    bool got_rotor = findFirstNode(node ,"rotor",findNodeVisitor::not_exact)!=nullptr;
    bool vehicle  = findFirstNode(node ,"wheel",findNodeVisitor::not_exact)!=nullptr;
    FIXME(ѕалец пол и потолок при определении модели)
    bool heli     = findFirstNode(node ,"tailrotor",findNodeVisitor::not_exact)!=nullptr;

    if(airplane /*&& !got_rotor*/)
        return  airplane::fill_cs(lod3?lod3:node,offset);
    else if(vehicle)
        return  vehicle::fill_cs(lod3?lod3:node,offset);
    else if(heli)
        return  heli::fill_cs(lod3?lod3:node,offset);
    else
        return  default::fill_cs(lod3?lod3:node,offset);

    return nullptr;
}

bool generateBulletFile(std::string name, osg::Node* node, cg::point_3& offset)
{
	btDefaultSerializer*	serializer = new btDefaultSerializer();
	
	// /*Convex*/btTriangleMeshShape* trimeshShape = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( node );
    
    btCompoundShape * cs =  fill_cs(node,offset);

    static const char* nodeName = node->getName().c_str();
    FIXME(ћожет надо дл€ всех узлов проставить имена)
    serializer->registerNameForPointer(cs, /*nodeName*/"lod3");   

    serializer->startSerialization();
	cs->serializeSingleShape(serializer);
	serializer->finishSerialization();

	FILE* file = fopen(name.c_str(),"wb");
	fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1, file);
	fclose(file);
    
    delete cs;

	return true;
};