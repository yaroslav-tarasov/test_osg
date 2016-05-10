/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * osgBullet is (C) Copyright 2009-2012 by Kenneth Mark Bryden
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include "stdafx.h"

#ifdef _DEBUG
#pragma comment(lib, "BulletSoftBody_Debug.lib")
#else 
#pragma comment(lib, "BulletSoftBody.lib")
#endif

#include <osgbDynamics/GroundPlane.h>
#include <osgbCollision/GLDebugDrawer.h>
#include <osgbCollision/Utils.h>
#include <osgbInteraction/DragHandler.h>
#include <osgbInteraction/LaunchHandler.h>
#include <osgbInteraction/SaveRestoreHandler.h>

#include <osgwTools/Shapes.h>

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftBody.h>

#include "utils/visitors/ComputeTriMeshVisitor.h"

btSoftBodyWorldInfo	worldInfo;


/** \cond */
struct MeshUpdater : public osg::Drawable::UpdateCallback
{
    MeshUpdater( const btSoftBody* softBody, const unsigned int size )
      : _softBody( softBody ),
        _size( size )
    {}
    virtual ~MeshUpdater()
    {}

    virtual void update( osg::NodeVisitor*, osg::Drawable* draw )
    {
        osg::Geometry* geom( draw->asGeometry() );
        osg::Vec3Array* verts( dynamic_cast< osg::Vec3Array* >( geom->getVertexArray() ) );

        // Update the vertex array from the soft body node array.
        const btSoftBody::tNodeArray& nodes = _softBody->m_nodes;
        osg::Vec3Array::iterator it( verts->begin() );
        unsigned int idx;
        for( idx=0; idx<_size && idx<nodes.size(); idx++)
        {
            *it++ = osgbCollision::asOsgVec3( nodes[ idx ].m_x );
        }
        verts->dirty();
        draw->dirtyBound();

        // Generate new normals.
#if 0
        osgUtil::SmoothingVisitor smooth;
        smooth.smooth( *geom );
        geom->getNormalArray()->dirty();
#endif
    }

    const btSoftBody* _softBody;
    const unsigned int _size;
};
/** \endcond */

//
// Random
//

static inline btScalar	UnitRand()
{
	return(rand()/(btScalar)RAND_MAX);
}

static inline btScalar	SignedUnitRand()
{
	return(UnitRand()*2-1);
}

static inline btVector3	Vector3Rand()
{
	const btVector3	p=btVector3(SignedUnitRand(),SignedUnitRand(),SignedUnitRand());
	return(p.normalized());
}

namespace SoftBodyHelpers
{


btSoftBody*		CreateRope(	btSoftBodyWorldInfo& worldInfo, const btVector3& from,
	const btVector3& to,
	int res,
	int fixeds)
{
	/* Create nodes	*/ 
	const int		r=res+2;
	btVector3*		x=new btVector3[r];
	btScalar*		m=new btScalar[r];
	int i;

	for(i=0;i<r;++i)
	{
		const btScalar	t=i/(btScalar)(r-1);
		x[i]=lerp(from,to,t);
		m[i]=1;
	}
	btSoftBody*		psb= new btSoftBody(&worldInfo,r,x,m);
	if(fixeds&1) psb->setMass(0,0);
	if(fixeds&2) psb->setMass(r-1,0);
	delete[] x;
	delete[] m;
	/* Create links	*/ 
	for(i=1;i<r;++i)
	{
		psb->appendLink(i-1,i);
	}
	/* Finished		*/ 
	return(psb);
}

//
btSoftBody*		CreatePatch(btSoftBodyWorldInfo& worldInfo,const btVector3& corner00,
	const btVector3& corner10,
	const btVector3& corner01,
	const btVector3& corner11,
	int resx,
	int resy,
	int fixeds,
	bool gendiags)
{
#define IDX(_x_,_y_)	((_y_)*rx+(_x_))
	/* Create nodes	*/ 
	if((resx<2)||(resy<2)) return(0);
	const int	rx=resx;
	const int	ry=resy;
	const int	tot=rx*ry;
	btVector3*	x=new btVector3[tot];
	btScalar*	m=new btScalar[tot];
	int iy;

	for(iy=0;iy<ry;++iy)
	{
		const btScalar	ty=iy/(btScalar)(ry-1);
		const btVector3	py0=lerp(corner00,corner01,ty);
		const btVector3	py1=lerp(corner10,corner11,ty);
		for(int ix=0;ix<rx;++ix)
		{
			const btScalar	tx=ix/(btScalar)(rx-1);
			x[IDX(ix,iy)]=lerp(py0,py1,tx);
			m[IDX(ix,iy)]=1;
		}
	}
	btSoftBody*		psb=new btSoftBody(&worldInfo,tot,x,m);
	if(fixeds&1)	psb->setMass(IDX(0,0),0);
	if(fixeds&2)	psb->setMass(IDX(rx-1,0),0);
	if(fixeds&4)	psb->setMass(IDX(0,ry-1),0);
	if(fixeds&8)	psb->setMass(IDX(rx-1,ry-1),0);
	delete[] x;
	delete[] m;
	/* Create links	and faces */ 
	for(iy=0;iy<ry;++iy)
	{
		for(int ix=0;ix<rx;++ix)
		{
			const int	idx=IDX(ix,iy);
			const bool	mdx=(ix+1)<rx;
			const bool	mdy=(iy+1)<ry;
			if(mdx) psb->appendLink(idx,IDX(ix+1,iy));
			if(mdy) psb->appendLink(idx,IDX(ix,iy+1));
			if(mdx&&mdy)
			{
				if((ix+iy)&1)
				{
					psb->appendFace(IDX(ix,iy),IDX(ix+1,iy),IDX(ix+1,iy+1));
					psb->appendFace(IDX(ix,iy),IDX(ix+1,iy+1),IDX(ix,iy+1));
					if(gendiags)
					{
						psb->appendLink(IDX(ix,iy),IDX(ix+1,iy+1));
					}
				}
				else
				{
					psb->appendFace(IDX(ix,iy+1),IDX(ix,iy),IDX(ix+1,iy));
					psb->appendFace(IDX(ix,iy+1),IDX(ix+1,iy),IDX(ix+1,iy+1));
					if(gendiags)
					{
						psb->appendLink(IDX(ix+1,iy),IDX(ix,iy+1));
					}
				}
			}
		}
	}
	/* Finished		*/ 
#undef IDX
	return(psb);
}

btRigidBody*	createRigidBody(btDiscreteDynamicsWorld* dW,float mass, const btTransform& startTransform, btCollisionShape* shape,  const btVector4& color = btVector4(1, 0, 0, 1))
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif//

	body->setUserIndex(-1);
	dW->addRigidBody(body);
	return body;
}

}

struct FindGeometry : public osg::NodeVisitor
{
	osg::Drawable*  _geom;

	FindGeometry() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

	FindGeometry( osg::Node& node ) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
		node.accept(*this);
	}

	void apply(osg::Geode& geode)
	{
		for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
			apply(*geode.getDrawable(i));
	}

	void apply(osg::Drawable& geom)
	{
		_geom = &geom;
	}
};

btSoftBody* btSoftBodyFromOSG( osg::Node* node )
{
#if 0
	utils::ComputeTriMeshVisitor visitor;
	node->accept( visitor );

	osg::Vec3Array* vertices = visitor.getTriMesh();
	if( vertices->size() < 3 )
	{
		osg::notify( osg::WARN ) << "osgbCollision::btTriMeshCollisionShapeFromOSG, no triangles found" << std::endl;
		return( NULL );
	}
#endif

	FindGeometry fg(*node);
	osg::Geometry* geom( fg._geom->asGeometry() );
	osg::Vec3Array* vertices( dynamic_cast< osg::Vec3Array* >( geom->getVertexArray() ) );

	geom->setDataVariance( osg::Object::DYNAMIC );
	geom->setUseDisplayList( false );
	geom->setUseVertexBufferObjects( true );
	geom->getOrCreateVertexBufferObject()->setUsage( GL_DYNAMIC_DRAW );

#if 1

#if 0
	btTriangleMesh* mesh = new btTriangleMesh;
	for( size_t i = 0; i + 2 < vertices->size(); i += 3 )
	{
		osg::Vec3& p1 = ( *vertices )[ i ];
		osg::Vec3& p2 = ( *vertices )[ i + 1 ];
		osg::Vec3& p3 = ( *vertices )[ i + 2 ];
		mesh->addTriangle( osgbCollision::asBtVector3( p1 ),
			osgbCollision::asBtVector3( p2 ), osgbCollision::asBtVector3( p3 ) );
	}


	unsigned char *vertexbase;
    int            numverts=0;
	PHY_ScalarType type;
	int            vertexStride;
	unsigned char* indexbase;
	int            indexstride;
    int            numfaces;
    PHY_ScalarType indicestype;

	mesh->getLockedVertexIndexBase(&vertexbase, numverts, type,
		                            vertexStride,&indexbase, indexstride,
							        numfaces, indicestype);
//#else
	std::vector<btScalar> vtxs((btScalar*)vertexbase, (btScalar*)vertexbase + numverts*4);
    std::vector<int>      idxs((int*)indexbase , (int*)indexbase + numverts);
#endif

	std::vector<btScalar>	vtx;
	std::vector<int>        idx;
	vtx.resize(vertices->size() * 3);
	idx.resize(vertices->size());

	for( size_t i = 0; i < vertices->size(); ++i )
	{
		osg::Vec3& p1 = ( *vertices )[ i ];
		vtx[i * 3 + 0] = p1.x();
		vtx[i * 3 + 1] = p1.y();
		vtx[i * 3 + 2] = p1.z();
		idx[i] = i;
	}

////////////////////////////////////////////////////////////////
	const size_t mVertexCount = vertices->size();

	std::vector<int> dupVertices(mVertexCount);
	int dupVerticesCount = 0;
	int i,j;
	std::vector<int> newIndexes(mVertexCount);
	for(i=0; i < mVertexCount; i++)
	{
		osg::Vec3& v1 = ( *vertices )[ i ];
		dupVertices[i] = -1;
		newIndexes[i] = i - dupVerticesCount;
		for(j=0; j < i; j++)
		{
		    osg::Vec3& v2 = ( *vertices )[ j ];
			if (v1 == v2) {
				dupVertices[i] = j;
				dupVerticesCount++;
				break;
			}
		}
	}
	printf("dupVerticesCount %d\n", dupVerticesCount);

	int newVertexCount = mVertexCount - dupVerticesCount;
	printf("newVertexCount %d\n", newVertexCount);
	std::vector<btScalar> verts;
	verts.resize(newVertexCount * 3);
	for(i=0, j=0; i < mVertexCount; i++)
	{
		if (dupVertices[i] == -1) {
		    osg::Vec3& v = ( *vertices )[ i ];
			verts[j++] = v.x();
			verts[j++] = v.y();
			verts[j++] = v.z();
		}
	}

	std::vector<int> indexes(vertices->size());
	int idnx, idnxDup;
	for(i=0; i < vertices->size(); i++)
	{
		idnx = idx[i];
		idnxDup = dupVertices[idnx];
		printf("dup %d\n", idnxDup);
		idnx = idnxDup == -1 ? idnx : idnxDup;
		indexes[i] = newIndexes[idnx];
	}

#endif

	return  btSoftBodyHelpers::CreateFromTriMesh(worldInfo,&vtx[0],&idx[0], vertices->size()/3 );
}


osg::Node* makeFlag( btSoftRigidDynamicsWorld* bw )
{
    const short resX( 12 ), resY( 9 );

	osg::ref_ptr< osg::Group > group = new osg::Group;

    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    const osg::Vec3 llCorner( -2., 0., 5. );
    const osg::Vec3 uVec( 4., 0., 0. );
    const osg::Vec3 vVec( 0., 0.1, 3. ); // Must be at a slight angle for wind to catch it.
    osg::Geometry* geom = osgwTools::makePlane( llCorner,
        uVec, vVec, osg::Vec2s( resX-1, resY-1 ) );
    geode->addDrawable( geom );

    // Set up for dynamic data buffer objects
    geom->setDataVariance( osg::Object::DYNAMIC );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    geom->getOrCreateVertexBufferObject()->setUsage( GL_DYNAMIC_DRAW );

    // Flag state: 2-sided lighting and a texture map.
    {
        osg::StateSet* stateSet( geom->getOrCreateStateSet() );

        osg::LightModel* lm( new osg::LightModel() );
        lm->setTwoSided( true );
        stateSet->setAttributeAndModes( lm );

        const std::string texName( "fort_mchenry_flag.jpg" );
        osg::Texture2D* tex( new osg::Texture2D(
            osgDB::readImageFile( texName ) ) );
        if( ( tex == NULL ) || ( tex->getImage() == NULL ) )
            osg::notify( osg::WARN ) << "Unable to read texture: \"" << texName << "\"." << std::endl;
        else
        {
            tex->setResizeNonPowerOfTwoHint( false );
            stateSet->setTextureAttributeAndModes( 0, tex );
        }
    }


    // Create the soft body using a Bullet helper function. Note that
    // our update callback will update vertex data from the soft body
    // node data, so it's important that the corners and resolution
    // parameters produce node data that matches the vertex data.
    btSoftBody* softBody( btSoftBodyHelpers::CreatePatch( worldInfo,
        osgbCollision::asBtVector3( llCorner ),
        osgbCollision::asBtVector3( llCorner + uVec ),
        osgbCollision::asBtVector3( llCorner + vVec ),
        osgbCollision::asBtVector3( llCorner + uVec + vVec ),
        resX, resY, 1+4, true ) );


    // Configure the soft body for interaction with the wind.
    softBody->getCollisionShape()->setMargin( 0.1 );
    softBody->m_materials[ 0 ]->m_kLST = 0.3;
    softBody->generateBendingConstraints( 2, softBody->m_materials[ 0 ] );
    softBody->m_cfg.kLF = 0.05;
    softBody->m_cfg.kDG = 0.01;
    softBody->m_cfg.piterations = 2;
#if( BT_BULLET_VERSION >= 279 )
    softBody->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSidedLiftDrag;
#else
    // Hm. Not sure how to make the wind blow on older Bullet.
    // This doesn't seem to work.
    softBody->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSided;
#endif
    softBody->setWindVelocity( btVector3( 50., 0., 0. ) );
    softBody->setTotalMass( 1. );

    bw->addSoftBody( softBody );
    geom->setUpdateCallback( new MeshUpdater( softBody, resX*resY ) );

#if 0
	struct	Functors
	{
		static btSoftBody* CtorRope(btSoftRigidDynamicsWorld* dW,const btVector3& p)
		{
			btSoftBody*	psb=SoftBodyHelpers::CreateRope(worldInfo,p,p+btVector3(10,0,0),8,1);
			psb->setTotalMass(50);
			dW->addSoftBody(psb);
			return(psb);
		}
	};
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(12,0,0));
	btRigidBody*		body=SoftBodyHelpers::createRigidBody(bw,50,startTransform,new btBoxShape(btVector3(2,6,2)));
	btSoftBody*	psb0=Functors::CtorRope(bw,btVector3(1,4,0));
	btSoftBody*	psb1=Functors::CtorRope(bw,btVector3(1,-4,0));
	psb0->appendAnchor(psb0->m_nodes.size()-1,body);
	psb1->appendAnchor(psb1->m_nodes.size()-1,body);
#endif
	

//////////////////////////////////////////////////////////////////////////////////

#if 1
	const btScalar	s=2;
	const btScalar	h=10;
	const int		segments=6;
	const int		count=50;
	for(int i=0;i<count;++i)
	{
		btSoftBody*		psb=btSoftBodyHelpers::CreatePatch(worldInfo,btVector3(-s,h,-s),
			btVector3(+s,h,-s),
			btVector3(-s,h,+s),
			btVector3(+s,h,+s),
			segments,segments,
			0,true);
		btSoftBody::Material*	pm=psb->appendMaterial();
		pm->m_flags				-=	btSoftBody::fMaterial::DebugDraw;
		psb->generateBendingConstraints(2,pm);
		psb->m_cfg.kLF			=	0.004;
		psb->m_cfg.kDG			=	0.0003;
		psb->m_cfg.aeromodel	=	btSoftBody::eAeroModel::V_TwoSided;
		btTransform		trs;
		btQuaternion	rot;
		btVector3		ra=Vector3Rand()*0.1;
		btVector3		rp=Vector3Rand()*15+btVector3(0,20,80);
		rot.setEuler(SIMD_PI/8+ra.x(),-SIMD_PI/7+ra.y(),ra.z());
		trs.setIdentity();
		trs.setOrigin(rp);
		trs.setRotation(rot);
		psb->transform(trs);
		psb->setTotalMass(0.1);
		psb->addForce(btVector3(0,2,0),0);
		bw->addSoftBody(psb);

	}
#endif

//////////////////////////////////////////////////////////////////////////////////////
	//osg::Sphere* sphere    = new osg::Sphere( osg::Vec3( 0.f, 0.f, 0.f ), 0.25f );
	//osg::ShapeDrawable* sd = new osg::ShapeDrawable( sphere );
	//sd->setColor( osg::Vec4( 1.f, 0.f, 0.f, 1.f ) );
	//sd->setName( "A nice sphere" );

	//osg::Geode* sgeode = new osg::Geode;
	//sgeode->addDrawable( sd );

#if 0
    osg::Node*  para_node = /*sgeode;*/osgDB::readNodeFile( "cube2.dae" );
	btSoftBody* para = btSoftBodyFromOSG( para_node );
	// Configure the soft body for interaction with the wind.
	btSoftBody::Material*	pm=para->appendMaterial();
	pm->m_flags				-=	btSoftBody::fMaterial::DebugDraw;
	para->generateBendingConstraints(2,pm);
#if 0
	para->getCollisionShape()->setMargin( 0.1 );
	para->m_materials[ 0 ]->m_kLST = 0.3;
	para->generateBendingConstraints( 2, softBody->m_materials[ 0 ] );
	para->m_cfg.kLF = 0.05;
	para->m_cfg.kDG = 0.01;
	para->m_cfg.piterations = 20;
	para->m_cfg.citerations = 20;
    para->m_cfg.collisions = btSoftBody::fCollision::CL_SS + btSoftBody::fCollision::CL_RS;
    para->m_cfg.kPR = 0.0;
	para->m_cfg.kDF   =   1;
	para->m_cfg.kSRHR_CL      =   0;
	para->m_cfg.kSR_SPLT_CL   =   0.0;
	para->m_cfg.kKHR = 0.0;
	para->generateClusters(0);
#endif
	para->m_cfg.kLF			=	0.004;
	para->m_cfg.kDG			=	0.0003;
	para->m_cfg.aeromodel	=	btSoftBody::eAeroModel::V_TwoSided;
	btTransform		trs;
	btQuaternion	rot;
	btVector3		ra=Vector3Rand()*0.1;
	btVector3		rp=Vector3Rand()*15+btVector3(0,20,80);
	rot.setEuler(SIMD_PI/2,-SIMD_PI/2,ra.z());
	trs.setIdentity();
	//trs.setOrigin(rp);
	trs.setRotation(rot);
	para->transform(trs);
	para->setTotalMass(0.01);
	// para->addForce(btVector3(0,2,0),0);

#if 0
#if( BT_BULLET_VERSION >= 279 )
	para->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSidedLiftDrag;
#else
	// Hm. Not sure how to make the wind blow on older Bullet.
	// This doesn't seem to work.
	para->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSided;
#endif
#endif
	
	bw->addSoftBody( para );

	FindGeometry fg(*para_node);

	osg::Geometry* pgeom( fg._geom->asGeometry() );
	osg::Vec3Array* vertices( dynamic_cast< osg::Vec3Array* >( pgeom->getVertexArray() ) );

	pgeom->setUpdateCallback( new MeshUpdater( para, vertices->size() ) );
	

	
	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	pat->setAttitude(osg::Quat(osg::inDegrees(-90.0),osg::X_AXIS));
	pat->addChild(para_node);

    group->addChild(pat);
#endif

	group->addChild(geode);
	
    return( group.release() );
}

btSoftRigidDynamicsWorld* initPhysics()
{
    btSoftBodyRigidBodyCollisionConfiguration* collision = new btSoftBodyRigidBodyCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher( collision );
    worldInfo.m_dispatcher = dispatcher;
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    btVector3 worldAabbMin( -10000, -10000, -10000 );
    btVector3 worldAabbMax( 10000, 10000, 10000 );
    btBroadphaseInterface* broadphase = new btAxisSweep3( worldAabbMin, worldAabbMax, 1000 );
    worldInfo.m_broadphase = broadphase;

    btSoftRigidDynamicsWorld* dynamicsWorld = new btSoftRigidDynamicsWorld( dispatcher, broadphase, solver, collision );

    btVector3 gravity( 0, 0, -9.8/*-32.17*/ );
    dynamicsWorld->setGravity( gravity );
    worldInfo.m_gravity = gravity;

    worldInfo.air_density = btScalar( 1.2 );
    worldInfo.water_density = 0;
    worldInfo.water_offset = 0;
    worldInfo.water_normal = btVector3( 0, 0, 0 );
    worldInfo.m_sparsesdf.Initialize();

    return( dynamicsWorld );
}



int main_patched_lowlevel( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    const bool debugDisplay( arguments.find( "--debug" ) > 0 );

    btSoftRigidDynamicsWorld* bw = initPhysics();
    osg::Group* root = new osg::Group;

    osg::Group* launchHandlerAttachPoint = new osg::Group;
    root->addChild( launchHandlerAttachPoint );


    osg::ref_ptr< osg::Node > rootModel( makeFlag( bw ) );
    if( !rootModel.valid() )
    {
        osg::notify( osg::FATAL ) << "mesh: Can't create flag mesh." << std::endl;
        return( 1 );
    }

    root->addChild( rootModel.get() );


    osg::ref_ptr< osgbInteraction::SaveRestoreHandler > srh = new
        osgbInteraction::SaveRestoreHandler;

    // Add ground
    const osg::Vec4 plane( 0., 0., 1., -100. );
    root->addChild( osgbDynamics::generateGroundPlane( plane, bw	) );



/////////////////////////////////////////////////////////////////////////////
    //create  object
    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(btVector3(0,0,-100));
     
    btCollisionShape* cylinderShape = new btCylinderShape (btVector3(50,50,100));

    btCollisionObject* newOb = new btCollisionObject();
    newOb->setWorldTransform(tr);
    newOb->setInterpolationWorldTransform( tr);

    newOb->setCollisionShape(cylinderShape);

    bw->addCollisionObject(newOb);

    worldInfo.m_sparsesdf.Reset();
///////////////////////////////////////////////////////////////////////



    osgbCollision::GLDebugDrawer* dbgDraw( NULL );
    if( /*debugDisplay*/ true)
    {
        dbgDraw = new osgbCollision::GLDebugDrawer();
        dbgDraw->setDebugMode( ~btIDebugDraw::DBG_DrawText );
        bw->setDebugDrawer( dbgDraw );
        root->addChild( dbgDraw->getSceneGraph() );
    }


    osgViewer::Viewer viewer( arguments );
	viewer.apply(new osgViewer::SingleScreen(0));
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    //viewer.setUpViewInWindow( 30, 30, 768, 480 );
    viewer.setSceneData( root );

    osgGA::TrackballManipulator* tb = new osgGA::TrackballManipulator;
    // tb->setHomePosition( osg::Vec3( 0., -16., 6. ), osg::Vec3( 0., 0., 5. ), osg::Vec3( 0., 0., 1. ) ); 
    viewer.setCameraManipulator( tb );
    viewer.getCamera()->setClearColor( osg::Vec4( .5, .5, .5, 1. ) );
    viewer.realize();

    // Create the launch handler.
    osgbInteraction::LaunchHandler* lh = new osgbInteraction::LaunchHandler(
        bw, launchHandlerAttachPoint, viewer.getCamera() );
    {
        // Use a custom launch model: Sphere with radius 0.2 (instead of default 1.0).
        osg::Geode* geode = new osg::Geode;
        const double radius( .2 );
        geode->addDrawable( osgwTools::makeGeodesicSphere( radius ) );
        lh->setLaunchModel( geode, new btSphereShape( radius ) );
        lh->setInitialVelocity( 40. );

        viewer.addEventHandler( lh );
    }

    srh->setLaunchHandler( lh );
    srh->capture();
    viewer.addEventHandler( srh.get() );
    viewer.addEventHandler( new osgbInteraction::DragHandler(
        bw, viewer.getCamera() ) );


    double prevSimTime = 0.;
    while( !viewer.done() )
    {
        if( dbgDraw != NULL )
            dbgDraw->BeginDraw();

        const double currSimTime = viewer.getFrameStamp()->getSimulationTime();
        bw->stepSimulation( currSimTime - prevSimTime );
        prevSimTime = currSimTime;

        if( dbgDraw != NULL )
        {
            bw->debugDrawWorld();
            dbgDraw->EndDraw();
        }

        worldInfo.m_sparsesdf.GarbageCollect();

        viewer.frame();
    }

    return( 0 );
}


AUTO_REG(main_patched_lowlevel)