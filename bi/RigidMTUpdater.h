#pragma once 



namespace bi
{
    class RigidMTUpdater : public osgGA::GUIEventHandler
    {
    public:
        typedef std::function<void(osg::MatrixTransform* mt)> on_collision_f;
    public:
        RigidMTUpdater( osgViewer::Viewer* viewer, osg::Group* root, on_collision_f on_collision = nullptr ) 
            : _root        (root)
            , _on_collision(on_collision)
            , _viewer(viewer)
        {
             registerHandlers();


        }

        void addGround( const osg::Vec3& gravity )
        {
            BulletMTInterface::instance()->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), gravity,
                [&](int id){
                    if(_on_collision)
                        _on_collision(_physicsNodes[id].get());   
            } );

            addCow( osg::Vec3( -11., 6., 4. ) );
        }

        void addCow( osg::Vec3 pos )
        {
            osg::Matrix m( osg::Matrix::rotate( 1.5, osg::Vec3( 0., 0., 1. ) ) *
                osg::Matrix::translate( pos ) );
            osg::MatrixTransform* root = new osg::MatrixTransform( m );
            osgwTools::AbsoluteModelTransform* amt = new osgwTools::AbsoluteModelTransform;
            amt->setDataVariance( osg::Object::DYNAMIC );
            _root->addChild( amt );

            const std::string fileName( "cow.osg" );
            osg::Node* node = osgDB::readNodeFile( fileName );
            if( node == NULL )
            {
                osg::notify( osg::FATAL ) << "Can't find \"" << fileName << "\". Make sure OSG_FILE_PATH includes the OSG sample data directory." << std::endl;
                return;
            }

            amt->addChild( node );
            BulletMTInterface::instance()->createCow(node,pos,m,amt);
        }

        osg::Transform*
            makeModel( const std::string& fileName, osg::Vec3 pos )
        {
            static std::vector<std::string> modelList;
            osg::Matrix m( osg::Matrix::translate( pos ) );
            osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt = new osgwTools::AbsoluteModelTransform;
            amt->setDataVariance( osg::Object::DYNAMIC );
            
            osg::Node*  modelNode = nullptr;
            auto it = std::find(modelList.begin(),modelList.end(),fileName);
            if( it!=modelList.end() )
            {   
                modelList.push_back(fileName);
                modelNode = osgDB::readNodeFile( fileName );
                if( !modelNode )
                {
                    osg::notify( osg::FATAL ) << "Can't find \"" << fileName << "\". Make sure OSG_FILE_PATH is set correctly." << std::endl;
                    //exit( 0 );
                }
            }
            amt->addChild( modelNode );
            
            _root->addChild( amt );

            BulletMTInterface::instance()->createCow(modelNode,pos,m,amt);
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

            osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(osg::Vec3(0.0,-(bb.yMax() - bb.yMin())/2.0f,-(bb.yMax() - bb.yMin())/2.0f)));

            float rot_angle = -90.f;
            if(dynamic_cast<osg::LOD*>(node))
                rot_angle = 0;

            const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(0.f)   , osg::Z_AXIS ); 

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));

            rotated->addChild(node);
            positioned->addChild(rotated);


            osg::Vec3 half_length ( (bb.xMax() - bb.xMin())/2.0f,(bb.zMax() - bb.zMin())/2.0f,(bb.yMax() - bb.yMin()) /2.0f );
            // BulletMTInterface::instance()->createBox( id, half_length, mass );
            addPhysicsData( id, positioned, pos, vel, mass );
        }

        void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();
            //phys::sys()->createBox( id, shape->getHalfLengths(), mass );
            addPhysicsData( id, shape, pos, vel, mass );
        }

        void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();
            //BulletMTInterface::instance()->createSphere( id, shape->getRadius(), mass );
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
                    BulletMTInterface::instance()->simulate(  );
                    //for ( NodeMap::iterator itr=_physicsNodes.begin();
                    //    itr!=_physicsNodes.end(); ++itr )
                    //{
                    //    osg::Matrix matrix = BulletMTInterface::instance()->getMatrix(itr->first);
                    //    itr->second->setMatrix( matrix );
                    //}
                } 

                

                break;
            default: break;
            }
            return false;
        }

        void registerHandlers()
        {
            BulletMTInterface::instance()->registerHandlers(_root.get(),_viewer.get());
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

            //BulletMTInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
            //BulletMTInterface::instance()->setVelocity( id, vel );
            _physicsNodes[id] = mt;
        }

        void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass )
        {
            osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
            mt->addChild( node/*.get()*/ );
            _root->addChild( mt.get() );

            //BulletMTInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
            //BulletMTInterface::instance()->setVelocity( id, vel );
            _physicsNodes[id] = mt;
        }
    private:
        typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
        NodeMap                               _physicsNodes;
        osg::ref_ptr<osg::Group>              _root;
        high_res_timer                        _hr_timer;
        on_collision_f                        _on_collision;
        osg::observer_ptr<osgViewer::Viewer>  _viewer;
    };

    osg::ref_ptr<osgGA::GUIEventHandler>& getMTUpdater()
    {
        static osg::ref_ptr<osgGA::GUIEventHandler> updater;
        return  updater;
    }


}