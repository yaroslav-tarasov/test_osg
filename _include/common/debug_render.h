#pragma once 


enum	debug_draw_mode
{
    dbg_NoDebug=0,
    dbg_DrawWireframe = 1,
    dbg_DrawAabb=2,
    dbg_DrawFeaturesText=4,
    dbg_DrawContactPoints=8,
    dbg_NoDeactivation=16,
    dbg_NoHelpText = 32,
    dbg_DrawText=64,
    dbg_ProfileTimings = 128,
    dbg_EnableSatComparison = 256,
    dbg_DisableBulletLCP = 512,
    dbg_EnableCCD = 1024,
    dbg_DrawConstraints = (1 << 11),
    dbg_DrawConstraintLimits = (1 << 12),
    dbg_FastWireframe = (1<<13),
    dbg_DrawNormals = (1<<14),
    dbg_max_debug_draw_mode
};

// debug renderer
struct debug_render 
{

    virtual void    setEnabled( bool enable ) = 0;
    virtual bool    getEnabled() const = 0;

    virtual void	drawLine( const cg::point_3& from,const cg::point_3& to,const cg::point_3& color ) = 0;
    virtual void	drawSphere( const cg::point_3& p, float radius, const cg::point_3& color ) = 0;

    virtual void	drawTriangle(const cg::point_3& a,const cg::point_3& b,const cg::point_3& c,const cg::point_3& color,float alpha) = 0;
    virtual void	drawContactPoint(const cg::point_3& PointOnB,const cg::point_3& normalOnB,float distance,int lifeTime,const cg::point_3& color) = 0;
    virtual void	reportErrorWarning(const char* warningString) = 0;
    virtual void	draw3dText(const cg::point_3& location,const char* textString) = 0;

    virtual void	setDebugMode(int debugMode) = 0;
    virtual int		getDebugMode() const = 0;

    virtual void    EndDraw() = 0;
    virtual void    BeginDraw() = 0;

    virtual void    setTextSize( const float size ) = 0;
    virtual float   getTextSize() const = 0;

};

typedef debug_render* debug_render_ptr;
