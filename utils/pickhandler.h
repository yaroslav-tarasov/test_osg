#pragma once

#include "callbacks.h" 

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
    enum action_t {ADD_ROUTE_POINT,DELETE_ROUTE_POINT,SELECT_OBJECT};
public: 

    PickHandler(): _selected_object(NONE_TYPE) {}        

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
    {
        if (!(( ea.getEventType()==osgGA::GUIEventAdapter::PUSH  && ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL)
             || ( ea.getEventType()==osgGA::GUIEventAdapter::PUSH  && ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_ALT)
            ))
            return false;

        action_t act = ADD_ROUTE_POINT;
        if(ea.getButton()==osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) 
           act = DELETE_ROUTE_POINT; 
        
        if(ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_ALT) act= SELECT_OBJECT;

        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        pick(viewer,ea,act);
        return false;
    }

    osg::Node* getOrCreateSelectionBox()
    {
        if ( !_selectionBox )
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            auto shape = new osg::ShapeDrawable(new osg::Box(osg::Vec3(), 1.0f));
            shape->setColor(osg::Vec4(0,0,255,255));
            geode->addDrawable( shape );

            _selectionBox = new osg::MatrixTransform;
            _selectionBox->setNodeMask( PICK_NODE_MASK );
            _selectionBox->addChild( geode.get() );

            osg::StateSet* ss = _selectionBox->getOrCreateStateSet();
            ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
            ss->setAttributeAndModes( new osg::PolygonMode(
                osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE) );
        }
        return _selectionBox.get();
    }

    void pick(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea, action_t act)
    {
        osg::Group* root = dynamic_cast<osg::Group*>(viewer->getSceneData());       
        if (!root) return;

        osgUtil::LineSegmentIntersector::Intersections intersections;

        osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
            new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
        osgUtil::IntersectionVisitor iv( intersector.get() );
        iv.setTraversalMask( /*~DO_NOT_*/PICK_NODE_MASK );
        viewer->getCamera()->accept( iv );

        if ( !intersector->containsIntersections() )
              return;

        if (viewer->computeIntersections(ea,intersections))
        {
            const osgUtil::LineSegmentIntersector::Intersection& hit = *intersections.begin();

            bool handleMovingModels = false;
            const osg::NodePath& nodePath = hit.nodePath;
            for(osg::NodePath::const_iterator nitr=nodePath.begin();
                nitr!=nodePath.end();
                ++nitr)
            {
                const osg::Transform* transform = dynamic_cast<const osg::Transform*>(*nitr);
                if (transform)
                {
                    if (transform->getDataVariance()==osg::Object::DYNAMIC) handleMovingModels=true;
                }

            }
            

            osg::Vec3 position = handleMovingModels ? hit.getLocalIntersectPoint() : hit.getWorldIntersectPoint();
            float scale = 1;
 
            if(act==ADD_ROUTE_POINT)           
            {
                if( _selected_object == NONE_TYPE)
                   return;

                osg::Geode* geode = new osg::Geode;
                geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(position,scale)));
                dynamic_cast<osg::ShapeDrawable *>(geode->getDrawable(0))->setColor( _selected_object == AIRCRAFT_TYPE? osg::Vec4(1.0,0,0,0) : osg::Vec4(0.0,1.0,0,0) );
                root->addChild(geode);
                _points.push_back(geode);
                _route.push_back(cg::point_3(position.x(),position.y(),position.z()));
                choosed_point_signal_(_route);
            }
            else if(act==DELETE_ROUTE_POINT) 
            { 
                if( _selected_object == NONE_TYPE)
                    return;

                if(_points.size()>0)
                {
                    root->removeChild(_points.back());
                    _points.erase(--_points.end());
                    _route.erase(--_route.end());
                }

            }
            else if(act==SELECT_OBJECT) 
            {
                bool is_root=false;

                osg::Node* parent = nullptr;
                for(auto it = hit.nodePath.begin();it!=hit.nodePath.end() ;++it)
                {
                    if(boost::to_lower_copy((*it)->getName())=="phys_ctrl")
                    {
                        is_root = true;
                        parent = *it;
                        break;
                    };
                }
                
                if(parent && is_root)
                {
                    // Устанавливается в CreateObject
                    // object_id при создании физического объекта
                    uint32_t id = 0;
                    parent->getUserValue("id",id);
                    //if(id)
                    selected_node_signal_(id);

                    _selectionBox->setUpdateCallback(utils::makeNodeCallback(_selectionBox.get(), [this,parent,hit](osg::NodeVisitor * pNV)->void {
                        osg::ComputeBoundsVisitor cbv;
                        parent->accept( cbv );
                        const osg::BoundingBox& bb = cbv.getBoundingBox();

                        osg::Vec3 worldCenter = bb.center() /** osg::computeLocalToWorld(hit.nodePath)*/;
                        _selectionBox->setMatrix(
                            osg::Matrix::scale(bb.xMax()-bb.xMin(), bb.yMax()-bb.yMin(), bb.zMax()-bb.zMin()) *
                            osg::Matrix::translate(worldCenter) );
                    }));

                }

            }   

        }
    }

    void handleSelectObjectEvent(objects_t type )
    {
         _selected_object =  type;
    }

    DECLARE_EVENT(choosed_point, (std::vector<cg::point_3> const &) ) ;
    DECLARE_EVENT(selected_node, (uint32_t) ) ;

protected:
    virtual ~PickHandler() {}
    std::list<osg::ref_ptr<osg::Geode>> _points;
    std::vector<cg::point_3>             _route;
    objects_t                  _selected_object;

protected:
    osg::ref_ptr<osg::MatrixTransform> _selectionBox;
};