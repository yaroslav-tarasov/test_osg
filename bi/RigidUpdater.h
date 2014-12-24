#pragma once 

#include "bi/GLDebugDrawer.h"

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
        {}

        void addGround( const osg::Vec3& gravity )
        {
            BulletInterface::instance()->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), gravity,
                [&](int id){
                    if(_on_collision)
                        _on_collision(_physicsNodes[id].get());   
            } );

            //const bool debug( arguments.read( "--debug" ) );
            if( _debug )
            {
                //osg::notify( osg::INFO ) << "osgbpp: Debug" << std::endl;
                _dbgDraw = new avCollision::GLDebugDrawer();
                _dbgDraw->setDebugMode( ~btIDebugDraw::DBG_DrawText );
                if(_root->getNumParents()>0)
                    _root->getParent(0)->addChild( _dbgDraw->getSceneGraph() );
                else
                   _root->addChild( _dbgDraw->getSceneGraph() );
            }

            if(_dbgDraw)
                BulletInterface::instance()->setDebugDrawer(_dbgDraw);
        }

        void addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();

            osg::ComputeBoundsVisitor cbv;
            node->accept( cbv );
            const osg::BoundingBox& bb = cbv.getBoundingBox();

            float xm = bb.xMax() - bb.xMin();
            float ym = bb.yMax() - bb.yMin();
            float zm = bb.zMax() - bb.zMin();

            float rot_angle = -90.f;
            auto tr = osg::Matrix::translate(osg::Vec3(0.0,-(zm)/2.0f,-(ym)/2.0f));
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

            rotated->addChild(node);
            positioned->addChild(rotated);
            osg::Vec3 half_length ( (bb.xMax() - bb.xMin())/2.0f,(bb.zMax() - bb.zMin())/2.0f,(bb.yMax() - bb.yMin()) /2.0f );
            if(dynamic_cast<osg::LOD*>(node))
            {
               half_length = osg::Vec3 ( (bb.xMax() - bb.xMin())/2.0f,(bb.yMax() - bb.yMin())/2.0f,(bb.zMax() - bb.zMin()) /2.0f );
            }
            BulletInterface::instance()->createBox( id, half_length, mass );
            addPhysicsData( id, positioned, pos, vel, mass );
        }

        void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();
            BulletInterface::instance()->createBox( id, shape->getHalfLengths(), mass );
            addPhysicsData( id, shape, pos, vel, mass );
        }

        void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();
            BulletInterface::instance()->createSphere( id, shape->getRadius(), mass );
            addPhysicsData( id, shape, pos, vel, mass );
        }

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
        {
            osgViewer::View* view = static_cast<osgViewer::View*>( &aa );
            if ( !view || !_root ) return false;

            switch ( ea.getEventType() )
            {
            case osgGA::GUIEventAdapter::KEYUP:
                if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Return )
                {
                    osg::Vec3 eye, center, up, dir;
                    view->getCamera()->getViewMatrixAsLookAt( eye, center, up );
                    dir = center - eye; dir.normalize();
                    addPhysicsSphere( new osg::Sphere(osg::Vec3(), 0.5f), eye, dir * 200.0f, 2.0 );
                }
                break;
            case osgGA::GUIEventAdapter::FRAME:
                {
                    double dt = _hr_timer.get_delta();
                    if( _dbgDraw)
                        _dbgDraw->BeginDraw();
                    BulletInterface::instance()->simulate( dt );
                    for ( NodeMap::iterator itr=_physicsNodes.begin();
                        itr!=_physicsNodes.end(); ++itr )
                    {
                        osg::Matrix matrix = BulletInterface::instance()->getMatrix(itr->first);
                        itr->second->setMatrix( matrix );
                    }

                    if( _dbgDraw)
                    {
                        BulletInterface::instance()->getScene()->debugDrawWorld();
                        _dbgDraw->EndDraw();
                    }
                } 

                break;
            default: break;
            }
            return false;
        }

    protected:
        void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass )
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable( new osg::ShapeDrawable(shape) );

            osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
            mt->addChild( geode.get() );
            _root->addChild( mt.get() );

            BulletInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
            BulletInterface::instance()->setVelocity( id, vel );
            _physicsNodes[id] = mt;
        }

        void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass )
        {
            osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
            mt->addChild( node/*.get()*/ );
            _root->addChild( mt.get() );

            BulletInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
            BulletInterface::instance()->setVelocity( id, vel );
            _physicsNodes[id] = mt;
        }
    private:
        typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
        NodeMap _physicsNodes;
        osg::observer_ptr<osg::Group> _root;
        high_res_timer                _hr_timer;
        on_collision_f                _on_collision;
        avCollision::GLDebugDrawer*   _dbgDraw;
        bool                          _debug;
    };

    //osg::ref_ptr<osgGA::GUIEventHandler>& getUpdater()
    //{
    //    static osg::ref_ptr<osgGA::GUIEventHandler> updater;
    //    return  updater;
    //}


}