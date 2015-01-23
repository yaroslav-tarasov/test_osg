#pragma once 

#include "GLDebugDrawer.h"


namespace bi
{
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
        {}


        void addGround( const osg::Vec3& gravity );
        void addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
		void addUFO(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
		void addUFO2(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass);
        void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass );
        void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass );

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    protected:
        void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass );

        void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass );

    private:
        typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
		typedef std::vector<phys::aircraft::info_ptr>  aircrafts_t;
        typedef std::vector<aircraft::phys_aircraft_ptr>  phys_aircrafts_t;
        NodeMap                                  _physicsNodes;
        osg::observer_ptr<osg::Group>            _root;
        high_res_timer                           _hr_timer;
        on_collision_f                           _on_collision;
        avCollision::GLDebugDrawer*              _dbgDraw;
        bool                                     _debug;
		aircrafts_t                              _aircrafts;
        phys_aircrafts_t                         _phys_aircrafts;
		boost::shared_ptr<phys::BulletInterface> _sys;
    };

}