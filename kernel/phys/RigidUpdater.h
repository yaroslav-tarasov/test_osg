#pragma once 

#include "GLDebugDrawer.h"
#include "trajectory_drawer.h"
//#include "aircraft/aircraft_model.h"
//#include "vehicle/vehicle_model.h"
// #include "kernel/msg_proxy.h"

namespace bi
{
    class RigidUpdater : public osgGA::GUIEventHandler 
    {
	    friend struct RigidUpdater_private;
    public:
        typedef std::function<void(osg::MatrixTransform* mt)> on_collision_f;
    public:
        RigidUpdater( osg::Group* root, on_collision_f on_collision = nullptr ); 

		void addGround( const osg::Vec3& gravity );

#ifdef OLD_STYLE
        void addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
		void addUFO(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
		void addUFO2(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
        void addUFO3(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
        void addUFO4(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);

        void addVehicle(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
        void addVehicle2(const string& model_name,osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
#endif

        void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
        void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
        //void createNodeHierarchy(osg::Node* node);

        void handlePointEvent(std::vector<cg::point_3> const &simple_route);
        void handleSelectObjectEvent(uint32_t id );

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
        
        osg::Node* addGUIObject( osg::Node* node )
        {
            // assert(false);
            // return addGUIObject_( node );
            return nullptr;
        }

        void setTrajectoryDrawer(TrajectoryDrawer * drawer)
        {
           _trajectory_drawer =  drawer;
        }

        DECLARE_EVENT(selected_object_type, (objects_t) ) ;

    protected:
        void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass );

        void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass );


    private:
        typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;

#ifdef OLD_STYLE
		typedef std::vector<phys::aircraft::info_ptr>                   aircrafts_t;
        typedef std::vector<aircraft::phys_aircraft_ptr>                phys_aircrafts_t;
        typedef std::vector<aircraft::info_ptr>                         aircraft_models_t;
        typedef std::vector<phys::ray_cast_vehicle::info_ptr>           phys_vehicles_t;
        typedef std::vector<vehicle::model_base_ptr>                    vehicles_t;
#endif
        NodeMap                                  _physicsNodes;
        osg::observer_ptr<osg::Group>            _root;
        high_res_timer                           _hr_timer;
        on_collision_f                           _on_collision;
        avCollision::GLDebugDrawer*              _dbgDraw;
        bool                                     _debug;
        
        osg::ref_ptr<TrajectoryDrawer>           _trajectory_drawer;
        osg::ref_ptr<TrajectoryDrawer>           _trajectory_drawer2;

#ifdef OLD_STYLE
		aircrafts_t                              _aircrafts;
        aircraft_models_t                        _model_aircrafts;
        vehicles_t                               _phys_vehicles;
#endif        


        polymorph_ptr<phys::BulletInterface>      _sys;
		struct RigidUpdater_private;
	    boost::shared_ptr<RigidUpdater_private>  _d;

        //kernel::msg_service                      msg_service_;

private:
        double                                   _last_frame_time;
        double                                   _time_delta_mod_sys;
        double                                   _time_delta_vis_sys;
        double                                   _time_delta_ctrl_sys;

        uint32_t                                 selected_obj_id_;
    };

}