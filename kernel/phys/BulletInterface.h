#ifndef H_BULLETINTERFACE
#define H_BULLETINTERFACE

#include "phys/phys_sys_fwd.h"
#include "common/phys_sys.h" 
#include "phys_sys.h"
#include "phys_sys_common.h" //FIXME  Â stdafx phys_sys
#include "sensor.h"

#include "common/debug_render.h"

namespace phys
{

class BulletInterface 
	: public phys::system_impl
    , public phys::system
    , public boost::enable_shared_from_this<BulletInterface>
    , public control
{
public:

    typedef std::function<void(int id)> on_collision_f;
public:
	BulletInterface();
	virtual ~BulletInterface();
  
#ifdef DEPRECATED	    
    void                        createWorld  ( const osg::Plane& plane, const osg::Vec3& gravity , on_collision_f on_collision = nullptr);
#endif

    void                        createBox    ( int id, const osg::Vec3& dim, double mass );
    void                        createSphere ( int id, double radius, double mass );


    void                        createShape  ( osg::Node* node, int id, double mass);
#ifdef DEPRECATED
	void                        createUFO    ( osg::Node* node, int id, double mass);
	aircraft::info_ptr          createUFO2    ( osg::Node* node, int id, double mass);
#endif

    aircraft::info_ptr          create_aircraft        (const phys::aircraft::params_t &,compound_sensor_ptr s,const decart_position &) override;
	aircraft::info_ptr          create_aircraft_pl     (const phys::aircraft::params_t &,compound_sensor_ptr s,const decart_position &) override;
    aircraft::info_ptr          create_helicopter_pl   (const phys::aircraft::params_t &,compound_sensor_ptr s,const decart_position &) override;
    ray_cast_vehicle::info_ptr  create_ray_cast_vehicle(double,phys::compound_sensor_ptr,const decart_position & pos) override;
    static_mesh_ptr             create_static_mesh     ( sensor_ptr s ) override;
    static_convex_ptr           create_static_convex   ( sensor_ptr s, point_3 const& pos, quaternion const& orien ) override;
    flock::info_ptr             create_flock_child     (const phys::flock::params_t & p,compound_sensor_ptr s,const decart_position & pos) override;
    aerostat::info_ptr          create_aerostat        (const phys::aerostat::params_t  & p,compound_sensor_ptr s,const decart_position & pos) override;
#ifdef DEPRECATED    
    ray_cast_vehicle::info_ptr  createVehicle(osg::Node* node,int id,double mass);
#endif

    void                        registerBody(int id,phys::rigid_body_ptr ctrl);
    void                        registerBody(int id);

    void                        setVelocity  ( int id, const osg::Vec3& pos );
    void                        setMatrix    ( int id, const osg::Matrix& matrix );
    osg::Matrix                 getMatrix( int id );
    
    void                        update( double step );     // former simulate

    void                        setDebugDrawer(/*btIDebugDraw*/debug_render* dd);
    void                        debugDrawWorld();
	//
    //  phys::system_impl
	//
public:
	phys::bt_dynamics_world_ptr    dynamics_world()    const override;
	phys::bt_vehicle_raycaster_ptr vehicle_raycaster() const override;
	void register_rigid_body( phys::rigid_body_impl * rb )   override;
	void unregister_rigid_body( phys::rigid_body_impl * rb ) override;

    // control
public:

    system_ptr                get_system(size_t zone) override;

    optional<size_t>          get_zone(cg::geo_point_3 const & pos) const override;
    optional<size_t>          get_zone(std::string const& airport) const override;
    cg::geo_base_3 const&     get_base(size_t zone) const override;
    std::string               zone_name(size_t id) const override;

	// collision
private:
	boost::optional<double>   intersect_first(cg::point_3 const& p, cg::point_3 const& q) const;

protected:  
    void checkForCollisionEvents();

private:
    #define BIT(x) (1<<(x))
    enum object_t {
        THE_GREAT_NOTHING = 0,
        WORLD  = BIT(1),
        SPHERE = BIT(2),
        BOX    = BIT(3),
        SHAPE  = BIT(4)
    };

   
    struct _private;
    _private*  d_;
    
    on_collision_f                        on_collision_;
    cg::geo_base_3                        base_;

};

BulletInterface*  sys();


boost::shared_ptr<BulletInterface> create();

} // ns phys
#endif
