#pragma once


// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public: 

    PickHandler() {}        

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
    {
        if (!( ea.getEventType()==osgGA::GUIEventAdapter::PUSH
            && ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL))
            return false;

        bool add = true;
        if(ea.getButton()==osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) 
           add =false; 
        
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        pick(viewer,ea,add);
        return false;
    }

    void pick(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea, bool add)
    {
        osg::Group* root = dynamic_cast<osg::Group*>(viewer->getSceneData());       
        if (!root) return;

        osgUtil::LineSegmentIntersector::Intersections intersections;
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
            float scale = 1;//10.0f * ((float)rand() / (float)RAND_MAX);
 
            if(add)           
            {
                osg::Geode* geode = new osg::Geode;
                geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(position,scale)));
                dynamic_cast<osg::ShapeDrawable *>(geode->getDrawable(0))->setColor( osg::Vec4(1.0,0,0,0) );
                root->addChild(geode);
                _points.push_back(geode);
            }
            else
            { 
                if(_points.size()>0)
                {
                    root->removeChild(_points.back());
                    _points.erase(--_points.end());
                }

            }

        }
    }

protected:
    virtual ~PickHandler() {}
    std::list<osg::ref_ptr<osg::Geode>> _points;
};