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

    #if 0
        btCollisionShape* cs_body   = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
    #else    
        btCollisionShape* cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::AXIS::Y,3 );
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

            auto sh_r_l = findFirstNode(node,"shassi_r_l",findNodeVisitor::not_exact);
            auto sh_r_r = findFirstNode(node,"shassi_r_r",findNodeVisitor::not_exact);
            auto sh_f   = findFirstNode(node,"shassi_f",findNodeVisitor::not_exact);

            auto sh_r_l_wheel = findFirstNode(sh_r_l,"wheel",findNodeVisitor::not_exact);
            auto sh_r_r_wheel = findFirstNode(sh_r_r,"wheel",findNodeVisitor::not_exact);
            auto sh_f_wheel   = findFirstNode(sh_f,"wheel",findNodeVisitor::not_exact);

            //const bool is_front = true;
            //{
            //    wheel_info wii(sh_f_wheel->getBound().radius(),is_front);
            //    wii.trans_f_body = get_relative_transform(node,sh_f_wheel/*,body*/);
            //    wi.push_back(wii);
            //}
            //{
            //    wheel_info wii(sh_r_r_wheel->getBound().radius(),!is_front);
            //    wii.trans_f_body = get_relative_transform(node,sh_r_r_wheel/*,body*/);
            //    wi.push_back(wii);
            //}
            //{
            //    wheel_info wii(sh_r_l_wheel->getBound().radius(),!is_front);
            //    wii.trans_f_body = get_relative_transform(node,sh_r_l_wheel/*,body*/);
            //    wi.push_back(wii);
            //}

            btCollisionShape* cs_body = nullptr;
            btCompoundShape*  s = new btCompoundShape;

            sh_f->setNodeMask(0);
            sh_r_r->setNodeMask(0);
            sh_r_l->setNodeMask(0);
    #if 0
            FIXME("При btTriMeshCollisionShape проваливаемся при столкновениях, btConvexTriMeshCollisionShape не сериализуется" )
            cs_body   = /*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
    #else
            cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::AXIS::Y,3 );
    #endif
            sh_f->setNodeMask(0xffffffff);
            sh_r_r->setNodeMask(0xffffffff);
            sh_r_l->setNodeMask(0xffffffff);

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

        auto main_rotor     = findFirstNode(node,"main",findNodeVisitor::not_exact);
        auto tail_rotor     = findFirstNode(node,"tailrotor",findNodeVisitor::not_exact);
        auto sagged_rotor   = findFirstNode(node,"sagged",findNodeVisitor::not_exact);


        //const bool is_front = true;
        //{
        //    wheel_info wii(sh_f_wheel->getBound().radius(),is_front);
        //    wii.trans_f_body = get_relative_transform(node,sh_f_wheel/*,body*/);
        //    wi.push_back(wii);
        //}
        //{
        //    wheel_info wii(sh_r_r_wheel->getBound().radius(),!is_front);
        //    wii.trans_f_body = get_relative_transform(node,sh_r_r_wheel/*,body*/);
        //    wi.push_back(wii);
        //}
        //{
        //    wheel_info wii(sh_r_l_wheel->getBound().radius(),!is_front);
        //    wii.trans_f_body = get_relative_transform(node,sh_r_l_wheel/*,body*/);
        //    wi.push_back(wii);
        //}

        btCollisionShape* cs_body = nullptr;
        btCompoundShape*  s = new btCompoundShape;

        if(main_rotor)  main_rotor->setNodeMask(0);
        if(tail_rotor)    tail_rotor->setNodeMask(0);
        if(sagged_rotor)  sagged_rotor->setNodeMask(0);
#if 0
        FIXME("При btTriMeshCollisionShape проваливаемся при столкновениях, btConvexTriMeshCollisionShape не сериализуется" )
            cs_body   = /*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
#else
        cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::AXIS::Y,3 );
#endif
        if(main_rotor) main_rotor->setNodeMask(0xffffffff);
        if(tail_rotor)   tail_rotor->setNodeMask(0xffffffff);
        if(sagged_rotor) sagged_rotor->setNodeMask(0xffffffff);

        if(cs_body)
            s->addChildShape(btTransform(btQuaternion(0,0,0),offset_),cs_body);

        return s;
    }

}

btCompoundShape* fill_cs(osg::Node* node, cg::point_3& offset)
{
    auto lod3 =  findFirstNode(node,"lod3");

    bool airplane = findFirstNode(node ,"shassi_",findNodeVisitor::not_exact)!=nullptr;
    bool vehicle  = findFirstNode(node ,"wheel",findNodeVisitor::not_exact)!=nullptr;
    FIXME(Палец пол и потолок при определении модели)
    bool heli     = findFirstNode(node ,"tailrotor",findNodeVisitor::not_exact)!=nullptr;

    if(airplane)
        return airplane::fill_cs(lod3?lod3:node,offset);
    else if(vehicle)
        return vehicle::fill_cs(lod3?lod3:node,offset);
    else if(vehicle)
        return  airplane::fill_cs(lod3?lod3:node,offset);

    return nullptr;
}

bool generateBulletFile(std::string name, osg::Node* node, cg::point_3& offset)
{
	btDefaultSerializer*	serializer = new btDefaultSerializer();
	
	// /*Convex*/btTriangleMeshShape* trimeshShape = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( node );
    
    btCompoundShape * cs =  fill_cs(node,offset);

    static const char* nodeName = node->getName().c_str();
    FIXME(Может надо для всех узлов проставить имена)
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