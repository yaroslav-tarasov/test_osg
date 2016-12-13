#include <cegui/CEGUI.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
//#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>
#include "CEGUIDrawable.h"


#include  "av/avCore/Utils.h"
//#include "../common/CommonFunctions"

#ifdef _DEBUG
#pragma comment(lib, "CEGUIBase-0_d.lib")
#pragma comment(lib, "CEGUIOpenGLRenderer-0_d.lib")
#else
#pragma comment(lib, "CEGUIBase-0.lib")
#pragma comment(lib, "CEGUIOpenGLRenderer-0.lib")
#endif

OpenThreads::Mutex  avGui::CEGUIDrawable::_mutex;
bool                avGui::CEGUIDrawable::_initialized = false;

using namespace CEGUI;


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


namespace avGui 
{

    bool CEGUIEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
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

    CEGUI::MouseButton CEGUIEventHandler::convertMouseButton( int button )
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


    osgGA::GUIEventHandler*  createCEGUI(osg::Group* root, std::function<void()> init_gui_handler)
    {                                                                     
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive( false );
        auto dr =new CEGUIDrawable;
        geode->addDrawable( dr );
        geode->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED );
        geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        //geode->getOrCreateStateSet()->setRenderBinDetails( -4, "RenderBin" );

        osg::ref_ptr<osg::Camera> hudCamera = Utils::createHUDCamera(1920, 0, 1920, 1200/*0, 800, 0, 600*/);
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



CEGUIDrawable::CEGUIDrawable()
    : _lastSimulationTime(0.0)
    , _activeContextID(0)
{
    setSupportsDisplayList( false );
    setDataVariance( osg::Object::DYNAMIC );
    getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
}

CEGUIDrawable::CEGUIDrawable( const CEGUIDrawable& copy,const osg::CopyOp& copyop )
    :   osg::Drawable(copy, copyop)
    ,_lastSimulationTime(copy._lastSimulationTime)
    ,_activeContextID(copy._activeContextID)
{}

void CEGUIDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    
    if ( !_initialized )
    {

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        if ( !_initialized )
        {
        CEGUI::OpenGLRenderer::bootstrapSystem( CEGUI::OpenGLRenderer::TTT_NONE );
        if ( !CEGUI::System::getSingletonPtr() ) return;

        CEGUI::DefaultResourceProvider* resource =
            static_cast<CEGUI::DefaultResourceProvider*>( CEGUI::System::getSingleton().getResourceProvider() );
        resource->setResourceGroupDirectory( "schemes"    , cfg().path.data + "/hmi/datafiles/schemes/"     );
        resource->setResourceGroupDirectory( "imagesets"  , cfg().path.data + "/hmi/datafiles/imagesets/"   );
        resource->setResourceGroupDirectory( "fonts"      , cfg().path.data + "/hmi/datafiles/fonts/"       );
        resource->setResourceGroupDirectory( "layouts"    , cfg().path.data + "/hmi/datafiles/layouts/"     );
        resource->setResourceGroupDirectory( "looknfeels" , cfg().path.data + "/hmi/datafiles/looknfeel/"   );
        resource->setResourceGroupDirectory( "lua_scripts", cfg().path.data + "/hmi/datafiles/lua_scripts/" );

        CEGUI::ImageManager::setImagesetDefaultResourceGroup( "imagesets" );
        CEGUI::Font::setDefaultResourceGroup( "fonts" );
        CEGUI::Scheme::setDefaultResourceGroup( "schemes" );
        CEGUI::WidgetLookManager::setDefaultResourceGroup( "looknfeels" );
        CEGUI::WindowManager::setDefaultResourceGroup( "layouts" );
        CEGUI::ScriptModule::setDefaultResourceGroup( "lua_scripts" );

         ready_for_init_signal_();
        _activeContextID = contextID;
        _initialized = true;
        }
   }
    else if ( contextID==_activeContextID )
    {
        osg::State* state = renderInfo.getState();
        state->disableAllVertexArrays();
        state->disableTexCoordPointer( 0 );
        auto tex_unit = state->getActiveTextureUnit();
		state->setActiveTextureUnit(0);
        
		glPushMatrix();
        glPushAttrib( GL_ALL_ATTRIB_BITS );

        CEGUI::OpenGLRenderer* renderer = static_cast<CEGUI::OpenGLRenderer*>(
            CEGUI::System::getSingleton().getRenderer() );
        osg::Viewport* viewport = renderInfo.getCurrentCamera()->getViewport();
        if ( renderer && viewport )
        {
            const CEGUI::Sizef& size = renderer->getDisplaySize();
            if ( size.d_width!=viewport->width() || size.d_height!=viewport->height() )
            {
                CEGUI::System::getSingleton().notifyDisplaySizeChanged(
                    CEGUI::Sizef(viewport->width(), viewport->height()) );
            }
        }

        double currentTime = (state->getFrameStamp() ? state->getFrameStamp()->getSimulationTime() : 0.0);
        CEGUI::System::getSingleton().injectTimePulse( (currentTime - _lastSimulationTime)/1000.0 );
        CEGUI::System::getSingleton().renderAllGUIContexts();
        _lastSimulationTime = currentTime;

        glPopAttrib();
        glPopMatrix();

        state->setActiveTextureUnit(tex_unit);
    }

}


bool CEGUIDrawable::handleClose( const CEGUI::EventArgs& e )
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    root->getChild("DemoWindow")->setVisible(false);
    return true;
}


} // avGui