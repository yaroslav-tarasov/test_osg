#include "stdafx.h"
#include "av/precompiled.h"

class PickHandler : public osgGA::GUIEventHandler
{
public:
    osg::Node* getOrCreateSelectionBox()
    {
        if ( !_selectionBox )
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            auto shape = new osg::ShapeDrawable(new osg::Box(osg::Vec3(), 1.0f));
            shape->setColor(osg::Vec4(0,0,255,255));
            geode->addDrawable( shape );
            
            _selectionBox = new osg::MatrixTransform;
            _selectionBox->setNodeMask( DO_NOT_PICK_NODE_MASK );
            _selectionBox->addChild( geode.get() );
            
            osg::StateSet* ss = _selectionBox->getOrCreateStateSet();
            ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            ss->setAttributeAndModes( new osg::PolygonMode(
                osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE) );
        }
        return _selectionBox.get();
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( ea.getEventType()!=osgGA::GUIEventAdapter::RELEASE ||
             ea.getButton()!=osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON ||
             !(ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL) )
            return false;
        
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if ( viewer )
        {
            osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
                new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
            osgUtil::IntersectionVisitor iv( intersector.get() );
            iv.setTraversalMask( ~DO_NOT_PICK_NODE_MASK );
            viewer->getCamera()->accept( iv );
            
            if ( intersector->containsIntersections() )
            {
                const osgUtil::LineSegmentIntersector::Intersection& result =
                    *(intersector->getIntersections().begin());
                
                osg::BoundingBox bb = result.drawable->getBound();
                osg::Vec3 worldCenter = bb.center() * osg::computeLocalToWorld(result.nodePath);
                _selectionBox->setMatrix(
                    osg::Matrix::scale(bb.xMax()-bb.xMin(), bb.yMax()-bb.yMin(), bb.zMax()-bb.zMin()) *
                    osg::Matrix::translate(worldCenter) );
            }
        }
        return false;
    }
    
protected:
    osg::ref_ptr<osg::MatrixTransform> _selectionBox;
};




int main_select( int argc, char** argv )
{
    osg::ref_ptr<osg::Node> model1 = osgDB::readNodeFile( "cessna.osg" );
    osg::ref_ptr<osg::Node> model2 = osgDB::readNodeFile( "cow.osg" );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( model1.get() );
    root->addChild( model2.get() );
    
    osg::ref_ptr<PickHandler> picker = new PickHandler;
    root->addChild( picker->getOrCreateSelectionBox() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( picker.get() );
    return viewer.run();
}

AUTO_REG(main_select)