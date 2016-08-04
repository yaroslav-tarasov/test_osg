/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/
#include "stdafx.h"

#include <cegui/CEGUI.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
//#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>
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
