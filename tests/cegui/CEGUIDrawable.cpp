/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/
#include "stdafx.h"

#include <cegui/CEGUI.h>
#include <CEGUI\RendererModules/OpenGL/GLRenderer.h>
//#include <CEGUI\RendererModules\OpenGL\GL3Renderer.h>
#include "CEGUIDrawable"

#ifdef _DEBUG
#pragma comment(lib, "CEGUIBase-0_d.lib")
#pragma comment(lib, "CEGUIOpenGLRenderer-0_d.lib")
#else
#pragma comment(lib, "CEGUIBase-0.lib")
#pragma comment(lib, "CEGUIOpenGLRenderer-0.lib")
#endif

OpenThreads::Mutex  CEGUIDrawable::_mutex;
bool                CEGUIDrawable::_initialized = false;

using namespace CEGUI;

CEGUIDrawable::CEGUIDrawable()
    :   _lastSimulationTime(0.0), _activeContextID(0)/*, _initialized(false)*/
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
    /*_initialized(copy._initialized)*/
{}

void CEGUIDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    
    if ( !_initialized )
    {

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        if ( !_initialized )
        {
        CEGUI::OpenGLRenderer::bootstrapSystem( CEGUI::OpenGLRenderer::TTT_NONE/*TTT_FBO*//*TTT_AUTO*//*TTT_NONE*/ );
        if ( !CEGUI::System::getSingletonPtr() ) return;

        CEGUI::DefaultResourceProvider* resource =
            static_cast<CEGUI::DefaultResourceProvider*>( CEGUI::System::getSingleton().getResourceProvider() );
        resource->setResourceGroupDirectory( "schemes", "./datafiles/schemes/" );
        resource->setResourceGroupDirectory( "imagesets", "./datafiles/imagesets/" );
        resource->setResourceGroupDirectory( "fonts", "./datafiles/fonts/" );
        resource->setResourceGroupDirectory( "layouts", "./datafiles/layouts/" );
        resource->setResourceGroupDirectory( "looknfeels", "./datafiles/looknfeel/" );
        resource->setResourceGroupDirectory( "lua_scripts", "./datafiles/lua_scripts/" );

        CEGUI::ImageManager::setImagesetDefaultResourceGroup( "imagesets" );
        CEGUI::Font::setDefaultResourceGroup( "fonts" );
        CEGUI::Scheme::setDefaultResourceGroup( "schemes" );
        CEGUI::WidgetLookManager::setDefaultResourceGroup( "looknfeels" );
        CEGUI::WindowManager::setDefaultResourceGroup( "layouts" );
        CEGUI::ScriptModule::setDefaultResourceGroup( "lua_scripts" );

        // const_cast<CEGUIDrawable*>(this)->initializeControls();
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
        
    }

}

void CEGUIDrawable::initializeControls()
{
	
    GUIContext& context = System::getSingleton().getDefaultGUIContext();

    SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    // context.getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");

    FontManager::getSingleton().createFromFile( "DejaVuSans-10.font" );
    context.setDefaultFont( "DejaVuSans-10" );
    context.getDefaultFont()->setAutoScaled(CEGUI::ASM_Disabled);

    Window* root = WindowManager::getSingleton().createWindow( "DefaultWindow", "Root" );
    context.setRootWindow(root);

    //FrameWindow* demoWindow = static_cast<FrameWindow*>(
    //    CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "DemoWindow") );
    //demoWindow->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim(0.01f)));
    //demoWindow->setSize(USize(cegui_reldim(0.5f), cegui_reldim(0.3f)));
    //demoWindow->setMinSize(USize(cegui_reldim(0.1f), cegui_reldim(0.1f)));
    //demoWindow->setText( "Example Dialog" );

    //PushButton* demoButtonOK = static_cast<PushButton*>(
    //    WindowManager::getSingleton().createWindow("TaharezLook/Button", "DemoButtonOK") );
    //demoButtonOK->setPosition(UVector2(cegui_reldim(0.3f), cegui_reldim(0.75f)));
    //demoButtonOK->setSize( USize(cegui_reldim(0.4f), cegui_reldim(0.15f)) );
    //demoButtonOK->setText( "OK" );

    //demoWindow->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked,
    //    CEGUI::Event::Subscriber(&CEGUIDrawable::handleClose, this) );

    //demoButtonOK->subscribeEvent( CEGUI::PushButton::EventClicked,
    //    CEGUI::Event::Subscriber(&CEGUIDrawable::handleClose, this) );

    //demoWindow->addChild( demoButtonOK );
    //root->addChild( demoWindow );

	// create combo-box.
	Combobox* cbbo = static_cast<Combobox*>( CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox", "SelModeBox"));
	root->addChild(cbbo);
	cbbo->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.01f)));
	//cbbo->setSize(USize(cegui_reldim(0.66f), cegui_reldim( 0.33f)));

	ListboxTextItem* itm;
    
    //russian
    std::wstring ruswide = L"Русский";
    CEGUI::Win32StringTranscoder stc;
    CEGUI::String ruslang = stc.stringFromStdWString(ruswide);

	// populate combobox with possible selection modes
	const CEGUI::Image* sel_img = &ImageManager::getSingleton().get("TaharezLook/MultiListSelectionBrush");
	itm = new ListboxTextItem(stc.stringFromStdWString(L"Пустая сцена"), 0);
	itm->setSelectionBrushImage(sel_img);
	cbbo->addItem(itm);
	itm = new ListboxTextItem(stc.stringFromStdWString(L"Шереметьево"), 1);
	itm->setSelectionBrushImage(sel_img);
	cbbo->addItem(itm);
	itm = new ListboxTextItem(stc.stringFromStdWString(L"Сочи"), 2);
	itm->setSelectionBrushImage(sel_img);
	cbbo->addItem(itm);
	cbbo->setReadOnly(true);
	//cbbo->setSortingEnabled(false);
	//cbbo->setSortingEnabled(true);
	//cbbo->handleUpdatedListItemData();
}

bool CEGUIDrawable::handleClose( const CEGUI::EventArgs& e )
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    root->getChild("DemoWindow")->setVisible(false);
    return true;
}
