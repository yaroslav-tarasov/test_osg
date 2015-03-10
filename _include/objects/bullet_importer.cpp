#include "stdafx.h"
//at the top of the file add
#include "btBulletWorldImporter.h"


bool loadBulletFile(std::string name, btBvhTriangleMeshShape* trimeshShape)
{
    btBulletWorldImporter import(0);//don't store info into the world
    if (import.loadFile(name.c_str()))
    {
        //int numBvh = import.getNumBvhs();
        //if (numBvh)
        //{
        //    btOptimizedBvh* bvh = import.getBvhByIndex(0);
        //    btVector3 aabbMin(-1000,-1000,-1000),aabbMax(1000,1000,1000);

        //    trimeshShape  = new btBvhTriangleMeshShape(m_indexVertexArrays,useQuantizedAabbCompression,aabbMin,aabbMax,false);
        //    trimeshShape->setOptimizedBvh(bvh);

        //}
        int numShape = import.getNumCollisionShapes();
        if (numShape)
        {
            trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByIndex(0);

            //if you know the name, you can also try to get the shape by name:
            const char* meshName = import.getNameForPointer(trimeshShape);
            if (meshName)
                trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByName(meshName);

        }

        return true;
    }

    return false;
}
