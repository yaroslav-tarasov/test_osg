#include "stdafx.h"
//at the top of the file add
#include <btBulletDynamicsCommon.h>
#include "btBulletWorldImporter.h"
#include <btBulletFile.h>

#include "BulletCollision/CollisionShapes/btTriangleMeshShape.h"
#include "kernel/phys/bullet_helpers.h"

FIXME("Структура проекта, ссылки в никуда")

namespace
{
    struct hr_helper 
    {
          high_res_timer                           hr_timer;
          const std::string                        name;
          __forceinline hr_helper(const std::string&                       name)
              : name(name)
          {}
          
          __forceinline ~hr_helper()
          {
              force_log fl;
              LOG_ODS_MSG( "loadBulletFile name " << name << " time: "<< hr_timer.get_delta() << "\n");
          }
    };
}

namespace phys
{


namespace {
    typedef std::map< std::string, /*bt_compound_shape_ptr*/btCompoundShape* > compoundShapeMap; 

    compoundShapeMap compoundShapeCache;


void  releaseCache()
{
    compoundShapeCache.clear();
}

}

bool loadBulletFile(std::string name, btCompoundShape*& compoundShape)
{
    hr_helper   hr(name);
     
    compoundShapeMap::iterator it;

    if(( it = compoundShapeCache.find(name))!=compoundShapeCache.end() )
    {
         compoundShape = compoundShapeCache[name];
         return true;
    }
    else
    {
        btBulletWorldImporter import(0);//don't store info into the world
        // import.setVerboseMode(0xFF);

        if (import.loadFile(name.c_str()))
        {
            hr_helper   hr(name);

            int numBvh = import.getNumBvhs();
            int numBRB = import.getNumRigidBodies();
            //if (numBvh)
            //{
            //    btOptimizedBvh* bvh = import.getBvhByIndex(0);
            //    btVector3 aabbMin(-1000,-1000,-1000),aabbMax(1000,1000,1000);

            //    trimeshShape  = new btBvhTriangleMeshShape(m_indexVertexArrays,useQuantizedAabbCompression,aabbMin,aabbMax,false);
            //    trimeshShape->setOptimizedBvh(bvh);

            //}
            //btCompoundShape* cs;
            int numShape = import.getNumCollisionShapes();
            if (numShape)
            {
                //trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByIndex(0);

                ////if you know the name, you can also try to get the shape by name:
                //const char* meshName = import.getNameForPointer(trimeshShape);
                //if (meshName)
                //    trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByName(meshName);
                compoundShape = (btCompoundShape*)import.getCollisionShapeByName("lod3");
                compoundShapeCache[name] = compoundShape;
            }

            return true;
        }
    }

    return false;
}

bool loadBulletFile(std::string name, btBvhTriangleMeshShape*& trimeshShape)
{
    hr_helper   hr(name);
    
    btBulletWorldImporter import(0);//don't store info into the world
    // import.setVerboseMode(0xFF);

    if (import.loadFile(name.c_str()))
    {


        int numBvh = import.getNumBvhs();
        int numBRB = import.getNumRigidBodies();
        //if (numBvh)
        //{
        //    btOptimizedBvh* bvh = import.getBvhByIndex(0);
        //    btVector3 aabbMin(-1000,-1000,-1000),aabbMax(1000,1000,1000);

        //    trimeshShape  = new btBvhTriangleMeshShape(m_indexVertexArrays,useQuantizedAabbCompression,aabbMin,aabbMax,false);
        //    trimeshShape->setOptimizedBvh(bvh);

        //}
        //btCompoundShape* cs;
        int numShape = import.getNumCollisionShapes();
        if (numShape)
        {
            //trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByIndex(0);

            ////if you know the name, you can also try to get the shape by name:
            //const char* meshName = import.getNameForPointer(trimeshShape);
            //if (meshName)
            trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByName("lod3");
        }

        return true;
    }

    return false;
}

}