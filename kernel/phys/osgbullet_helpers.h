#pragma once

#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>

namespace phys
{
    namespace osg_helpers
    {
        inline btVector3 to_bullet_vector3( osg::Vec3 const& v )
        {
            return btVector3(btScalar(v.x()), btScalar(v.y()), btScalar(v.z()));
        }

        inline osg::Vec3 from_bullet_vector3( btVector3 const& v )
        {
            return osg::Vec3(v.x(), v.y(), v.z());
        }

        inline btMatrix3x3 to_bullet_matrix( osg::Matrix mm )
        {                 
            btMatrix3x3 m;
            m.setValue(btScalar(mm(0, 0)), btScalar(mm(0, 1)), btScalar(mm(0, 2)), 
                btScalar(mm(1, 0)), btScalar(mm(1, 1)), btScalar(mm(1, 2)),
                btScalar(mm(2, 0)), btScalar(mm(2, 1)), btScalar(mm(2, 2)));
            return m;
        }

        inline osg::Matrix from_bullet_matrix( btMatrix3x3 const& m )
        {
            osg::Matrix mm;
            mm(0,0) = m.getRow(0).x(), mm(0,1) = m.getRow(0).y(), mm(0,2) = m.getRow(0).z();
            mm(1,0) = m.getRow(1).x(), mm(1,1) = m.getRow(1).y(), mm(1,2) = m.getRow(1).z();
            mm(2,0) = m.getRow(2).x(), mm(2,1) = m.getRow(2).y(), mm(2,2) = m.getRow(2).z();

            return mm;
        }

        inline osg::Quat from_bullet_quaternion( btQuaternion const& q )
        {                    
            return osg::Quat(q.x(), q.y(), q.z(),q.w());
        }

        inline btQuaternion to_bullet_quaternion( osg::Quat const& q )
        {                    
            return btQuaternion((btScalar)q.x(), (btScalar)q.y(), (btScalar)q.z(), (btScalar)q.w());
        }

        inline btTransform to_bullet_transform( osg::Matrix const& tr )
        {
            return osgbCollision::asBtTransform(tr);
        }

	    inline osg::Matrix from_bullet_transform( btTransform const& tr )
	    {
	        return osgbCollision::asOsgMatrix(tr);
	    }
    }

    inline osg::Matrix asOsgMatrix( const btTransform& t )
    {
        btScalar ogl[ 16 ];
        t.getOpenGLMatrix( ogl );
        osg::Matrix m( ogl );
        return m;
    }

    inline btTransform asBtTransform( const osg::Matrix& m )
    {
        const osg::Matrix::value_type* oPtr = m.ptr();
        btScalar bPtr[ 16 ];
        int idx;
        for (idx=0; idx<16; idx++)
            bPtr[ idx ] = oPtr[ idx ];
        btTransform t;
        t.setFromOpenGLMatrix( bPtr );
        return t;
    }    

}