/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/
#include "stdafx.h"

#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>

#include <cegui/CEGUI.h>
#include "CEGUIDrawable"

#include "../common/CommonFunctions"

namespace {

	inline CEGUI::Key::Scan key_conv(int osg_key)
	{
		    using namespace CEGUI;
			const CEGUI::Key::Scan numpad[10] = {Key::Numpad0,Key::Numpad1,Key::Numpad2,Key::Numpad3,Key::Numpad4,Key::Numpad5,Key::Numpad6,Key::Numpad7,Key::Numpad8,Key::Numpad9};
		
		    if(osgGA::GUIEventAdapter::KEY_1 <=osg_key && osgGA::GUIEventAdapter::KEY_9 >=osg_key)
				return (CEGUI::Key::Scan)((int)CEGUI::Key::One + (osg_key - osgGA::GUIEventAdapter::KEY_1 ));

			if(osgGA::GUIEventAdapter::KEY_KP_0 <=osg_key && osgGA::GUIEventAdapter::KEY_KP_9 >=osg_key)
				return numpad[(osg_key - osgGA::GUIEventAdapter::KEY_KP_0 )];
		

			CEGUI::Key::Scan ret = CEGUI::Key::Unknown; 
			switch (osg_key)
			{
			case osgGA::GUIEventAdapter::KEY_0:
				ret = CEGUI::Key::Zero;
				break;
			case osgGA::GUIEventAdapter::KEY_BackSpace:
				ret = CEGUI::Key::Backspace;
				break;
			case osgGA::GUIEventAdapter::KEY_Return:
				ret = CEGUI::Key::Return;
				break;
			case osgGA::GUIEventAdapter::KEY_KP_Enter:
				ret = CEGUI::Key::NumpadEnter;
				break;
			case osgGA::GUIEventAdapter::KEY_Period:
				ret = CEGUI::Key::Period;
				break;
			case osgGA::GUIEventAdapter::KEY_Comma:
				ret = CEGUI::Key::Comma;
				break;
			}

			return  ret;
	}

	inline CEGUI::String::value_type char_conv(int osg_key)
	{
		if(osgGA::GUIEventAdapter::KEY_0 <=osg_key && osgGA::GUIEventAdapter::KEY_9 >=osg_key)
			return (L'0' + (osg_key - osgGA::GUIEventAdapter::KEY_0 ));
		
		if(osgGA::GUIEventAdapter::KEY_KP_0 <=osg_key && osgGA::GUIEventAdapter::KEY_KP_9 >=osg_key)
			return (L'0' + (osg_key - osgGA::GUIEventAdapter::KEY_KP_0 )) ;

		if (osg_key == osgGA::GUIEventAdapter::KEY_Period || osg_key == osgGA::GUIEventAdapter::KEY_Comma)
			return L'.';

		return L'';
	}
}

class CEGUIEventHandler : public osgGA::GUIEventHandler
{
public:
    CEGUIEventHandler( osg::Camera* camera ) : _camera(camera) {}

    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        int x = ea.getX(), y = ea.getY(), width = ea.getWindowWidth(), height = ea.getWindowHeight();
        if ( ea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS )
            y = ea.getWindowHeight() - y;
        
        if ( !CEGUI::System::getSingletonPtr() )
            return false;

        CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();

        switch ( ea.getEventType() )
        {
		case osgGA::GUIEventAdapter::KEYDOWN:
			context.injectKeyDown(key_conv(ea.getKey()));
			context.injectChar(char_conv(ea.getKey()));
			// return key_conv(ea.getKey()) != CEGUI::Key::Unknown;
			break;
		case osgGA::GUIEventAdapter::KEYUP:
			context.injectKeyUp(key_conv(ea.getKey()));
			// return key_conv(ea.getKey()) != CEGUI::Key::Unknown;
			break;
        case osgGA::GUIEventAdapter::PUSH:
            context.injectMousePosition( x, y );
            context.injectMouseButtonDown(convertMouseButton(ea.getButton()));
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            context.injectMousePosition(x, y);
            context.injectMouseButtonUp(convertMouseButton(ea.getButton()));
            break;
        case osgGA::GUIEventAdapter::SCROLL:
            if ( ea.getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN )
                context.injectMouseWheelChange(-1);
            else if ( ea.getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP )
                context.injectMouseWheelChange(+1);
            break;
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
            context.injectMousePosition(x, y);
            break;
        case osgGA::GUIEventAdapter::RESIZE:
            if ( _camera.valid() )
            {
                _camera->setProjectionMatrix( osg::Matrixd::ortho2D(0.0, width, 0.0, height) );
                _camera->setViewport( 0.0, 0.0, width, height );
            }
            break;
        default:
            return false;
        }

        CEGUI::Window* rootWindow = context.getRootWindow();
        if ( rootWindow )
        {
            CEGUI::Window* anyWindow = rootWindow->getChildAtPosition( CEGUI::Vector2f(x, y) );
            if ( anyWindow ) return true;
        }
        return false;
    }

protected:
    CEGUI::MouseButton convertMouseButton( int button )
    {
        switch ( button )
        {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
            return CEGUI::LeftButton;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
            return CEGUI::MiddleButton;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            return CEGUI::RightButton;
        default: break;
        }
        return static_cast<CEGUI::MouseButton>(button);
    }

    osg::observer_ptr<osg::Camera> _camera;
};

namespace gui 
{

    osgGA::GUIEventHandler*  createCEGUI(osg::Group* root, std::function<void()> init_gui_handler)
    {                                                                     
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive( false );
        auto dr =new CEGUIDrawable;
        geode->addDrawable( dr );
        geode->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED );
        geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        //geode->getOrCreateStateSet()->setRenderBinDetails( -4, "RenderBin" );

        osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(1920, 0, 1920, 1200/*0, 800, 0, 600*/);
        hudCamera->setAllowEventFocus( true );
        hudCamera->addChild( geode.get() );

        root->addChild( hudCamera.get() );
        
        dr->subscribe_ready_for_init(init_gui_handler);

        return  new CEGUIEventHandler(hudCamera.get()) ;
    }

    void  releaseCEGUI()
    {                                                                     
        if ( !CEGUI::System::getSingletonPtr() ) return;

        CEGUI::System::destroy();
    }
}

int main_cegui( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setCullingActive( false );
    geode->addDrawable( new CEGUIDrawable );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 800, 0, 600);
    hudCamera->setAllowEventFocus( true );
    hudCamera->addChild( geode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFile("cow.osg") );
    root->addChild( hudCamera.get() );
    
    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new CEGUIEventHandler(hudCamera.get()) );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}

AUTO_REG(main_cegui) 