#pragma once 

#include "GLDebugDrawer.h"
#include "trajectory_drawer.h"
#include "find_node_visitor.h"  

namespace bi
{
    struct aircraft_model
    {
        aircraft_model( aircraft::phys_aircraft_ptr          aircraft,
                        aircraft::shassis_support_ptr          shassis)
              : aircraft(aircraft)
              , shassis (shassis)
              , desired_velocity(min_desired_velocity)
        {}

        aircraft::phys_aircraft_ptr            aircraft;
        aircraft::shassis_support_ptr          shassis;
        fms::trajectory_ptr                    traj;
        double                                 desired_velocity;

        static const   int                     max_desired_velocity = 20;
        static const   int                     min_desired_velocity = 5;
        inline static  double                  min_radius() {return 18.75;} 
        inline static  double                  step()       {return 2.0;} 
    };



    class RigidUpdater : public osgGA::GUIEventHandler 
    {
    public:
        typedef std::function<void(osg::MatrixTransform* mt)> on_collision_f;
    public:
        RigidUpdater( osg::Group* root, on_collision_f on_collision = nullptr ) 
            : _root        (root)
            , _on_collision(on_collision)
            , _dbgDraw     (nullptr)
            , _debug       (true)
			, _sys         (phys::create())
            , _last_frame_time(0)
        {}


        void addGround( const osg::Vec3& gravity );
        void addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
		void addUFO(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
		void addUFO2(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
        void addUFO3(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
        void addVehicle(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);

        void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
        void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass );

        void handlePointEvent(std::vector<cg::point_3> const &simple_route);

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
        
        osg::Node* addGUIObject( osg::Node* node )
        {
            return addGUIObject_( node );
        }

        void setTrajectoryDrawer(TrajectoryDrawer * drawer)
        {
           _trajectory_drawer =  drawer;
        }

    protected:
        void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass );

        void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass );

        inline static osg::Node* addGUIObject_( osg::Node* node ) 
        {
            osg::ComputeBoundsVisitor cbv;
            node->accept( cbv );
            const osg::BoundingBox& bb = cbv.getBoundingBox();

            float xm = bb.xMax() - bb.xMin();
            float ym = bb.yMax() - bb.yMin();
            float zm = bb.zMax() - bb.zMin();

            float rot_angle = -90.f;
            auto tr = osg::Matrix::translate(osg::Vec3(0.0,-(ym)/2.0f,0.0));
            if(dynamic_cast<osg::LOD*>(node))
            {
                rot_angle = 0;
                tr = osg::Matrix::translate(osg::Vec3(0,0,-(zm)/2.0f));
            }        

            osg::MatrixTransform* positioned = new osg::MatrixTransform(tr);

            const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(0.f)   , osg::Z_AXIS ); 

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
            positioned->addChild(rotated);
            rotated->addChild(node);
            return positioned;
        }     

        inline static osg::Node* addGUIObject_v( osg::Node* node ) 
        {
            osg::Node* lod3 =  findFirstNode(node,"Lod3");

            osg::ComputeBoundsVisitor cbv;
            node->accept( cbv );
            const osg::BoundingBox& bb = cbv.getBoundingBox();

            float xm = bb.xMax() - bb.xMin();
            float ym = bb.yMax() - bb.yMin();
            float zm = bb.zMax() - bb.zMin();

            float rot_angle = -90.f;
            auto tr = osg::Matrix::translate(osg::Vec3(0.0,lod3?-(ym)/2.0f:0.0,0.0));
            if(dynamic_cast<osg::LOD*>(node))
            {
                rot_angle = 0;
                tr = osg::Matrix::translate(osg::Vec3(0,0,-(zm)/2.0f));
            }        

            osg::MatrixTransform* positioned = new osg::MatrixTransform(tr);

            const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
                osg::inDegrees(0.f)   , osg::Y_AXIS,
                osg::inDegrees(0.f)   , osg::Z_AXIS ); 

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
            positioned->addChild(rotated);
            rotated->addChild(node);
            return positioned;
        }

    private:
        typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
		typedef std::vector<phys::aircraft::info_ptr>                   aircrafts_t;
        typedef std::vector<aircraft::phys_aircraft_ptr>                phys_aircrafts_t;
        typedef std::vector<aircraft_model>                             aircraft_models_t;
        typedef std::vector<phys::ray_cast_vehicle::info_ptr>           phys_vehicles_t;

        NodeMap                                  _physicsNodes;
        osg::observer_ptr<osg::Group>            _root;
        high_res_timer                           _hr_timer;
        on_collision_f                           _on_collision;
        avCollision::GLDebugDrawer*              _dbgDraw;
        bool                                     _debug;
		aircrafts_t                              _aircrafts;
        aircraft_models_t                        _phys_aircrafts;
        phys_vehicles_t                          _phys_vehicles;
        osg::ref_ptr<TrajectoryDrawer>           _trajectory_drawer;


		boost::shared_ptr<phys::BulletInterface> _sys;
    private:
        double                                   _last_frame_time;

    };

}