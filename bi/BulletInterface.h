#ifndef H_BULLETINTERFACE
#define H_BULLETINTERFACE

#include <btBulletDynamicsCommon.h> 
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>
#include "bullet_helpers.h"
#include "phys_sys_common.h"
#include "../phys_sys_fwd.h"

namespace phys
{

class BulletInterface 
	: public phys::system_impl
	// , public osg::Referenced
    , public boost::enable_shared_from_this<BulletInterface>
{
public:
    typedef std::pair<const btRigidBody*, const btRigidBody*> CollisionPair;
    typedef std::set<CollisionPair> CollisionPairs;
    #define BIT(x) (1<<(x))
    enum object_t {
        THE_GREAT_NOTHING = 0,
        WORLD  = BIT(1),
        SPHERE = BIT(2),
        BOX    = BIT(3),
        SHAPE  = BIT(4)
    };
    struct data
    {
      btRigidBody* _body;
      object_t     _type;
    };
    typedef std::function<void(int id)> on_collision_f;
public:
	BulletInterface();
	virtual ~BulletInterface();

#if 0
    static BulletInterface*  instance();
#endif    
	btDynamicsWorld* getScene() { return _scene.get(); }
    
    void createWorld  ( const osg::Plane& plane, const osg::Vec3& gravity , on_collision_f on_collision = nullptr);
    void createBox    ( int id, const osg::Vec3& dim, double mass );
    void createSphere ( int id, double radius, double mass );
    void createShape  ( osg::Node* node, int id, double mass);
	void createUFO    ( osg::Node* node, int id, double mass);
	aircraft::info_ptr createUFO2    ( osg::Node* node, int id, double mass);

    void setVelocity  ( int id, const osg::Vec3& pos );
    void setMatrix    ( int id, const osg::Matrix& matrix );
    osg::Matrix getMatrix( int id );
    
    void simulate( double step );
    void setDebugDrawer(btIDebugDraw* dd)
    {
        _dd=dd;    
        if(_dd)
        _scene->setDebugDrawer(_dd);
    }
	//
    //  phys::system_impl
	//
public:
	phys::bt_dynamics_world_ptr    dynamics_world()    const override;
	phys::bt_vehicle_raycaster_ptr vehicle_raycaster() const override;
	void register_rigid_body( phys::rigid_body_impl * rb )   override;
	void unregister_rigid_body( phys::rigid_body_impl * rb ) override;
protected:  

    void checkForCollisionEvents();
    void CollisionEvent(btRigidBody * pBody0, btRigidBody * pBody1);
private:
    //struct private_data;
    //osg::ref_ptr<private_data> d;

    typedef std::map<int, data>            ActorMap;
    ActorMap                              _actors;
    // btDiscreteDynamicsWorld*              _scene;
    bt_dynamics_world_ptr                 _scene;   
    btDefaultCollisionConfiguration*      _configuration;
    btCollisionDispatcher*                _dispatcher;
    btBroadphaseInterface*                _overlappingPairCache;
    btSequentialImpulseConstraintSolver*  _solver;
    CollisionPairs                        _pairsLastUpdate;
    on_collision_f                        _on_collision;
    btIDebugDraw*                         _dd;

	bt_vehicle_raycaster_ptr              vehicle_raycaster_;
	std::set<rigid_body_impl *>			  rigid_bodies_;
};

//inline static BulletInterface*  sys()
//{
//	return BulletInterface::instance();
//}

BulletInterface*  sys();


boost::shared_ptr<BulletInterface> create();

} // ns phys
#endif
