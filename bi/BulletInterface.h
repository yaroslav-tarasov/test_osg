#ifndef H_BULLETINTERFACE
#define H_BULLETINTERFACE

#include <btBulletDynamicsCommon.h> 

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Plane>
#include <osg/Matrix>
#include <string>
#include <map>

class BulletInterface : public osg::Referenced
{
public:
    typedef std::pair<const btRigidBody*, const btRigidBody*> CollisionPair;
    typedef std::set<CollisionPair> CollisionPairs;
    #define BIT(x) (1<<(x))
    enum object_t {
        THE_GREAT_NOTHING = 0,
        WORLD  = BIT(1),
        SPHERE = BIT(2),
        BOX    = BIT(3)};
    struct data
    {
      btRigidBody* _body;
      object_t     _type;
    };
    typedef std::function<void(int id)> on_collision_f;
public:
    static BulletInterface*  instance();
    btDiscreteDynamicsWorld* getScene() { return _scene; }
    
    void createWorld  ( const osg::Plane& plane, const osg::Vec3& gravity , on_collision_f on_collision = nullptr);
    void createBox    ( int id, const osg::Vec3& dim, double mass );
    void createSphere ( int id, double radius, double mass );
    
    void setVelocity  ( int id, const osg::Vec3& pos );
    void setMatrix    ( int id, const osg::Matrix& matrix );
    osg::Matrix getMatrix( int id );
    
    void simulate( double step );
    
protected:
    BulletInterface();
    virtual ~BulletInterface();

    void checkForCollisionEvents();
    void CollisionEvent(btRigidBody * pBody0, btRigidBody * pBody1);
private:

    typedef std::map<int, data>  ActorMap;
    ActorMap                              _actors;
    btDiscreteDynamicsWorld*              _scene;
    btDefaultCollisionConfiguration*      _configuration;
    btCollisionDispatcher*                _dispatcher;
    btBroadphaseInterface*                _overlappingPairCache;
    btSequentialImpulseConstraintSolver*  _solver;
    CollisionPairs                        _pairsLastUpdate;
    on_collision_f                        _on_collision;
};

#endif
