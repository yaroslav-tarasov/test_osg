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
struct debug_render : public osg::Referenced
{

    virtual void    setEnabled( bool enable ) = 0;
    virtual bool    getEnabled() const = 0;

    virtual void	drawLine( const osg::Vec3& from,const osg::Vec3& to,const osg::Vec3& color ) = 0;
    virtual void	drawSphere( const osg::Vec3& p, float radius, const osg::Vec3& color ) = 0;

    virtual void	drawTriangle(const osg::Vec3& a,const osg::Vec3& b,const osg::Vec3& c,const osg::Vec3& color,float alpha) = 0;
    virtual void	drawContactPoint(const osg::Vec3& PointOnB,const osg::Vec3& normalOnB,float distance,int lifeTime,const osg::Vec3& color) = 0;
    virtual void	reportErrorWarning(const char* warningString) = 0;
    virtual void	draw3dText(const osg::Vec3& location,const char* textString) = 0;

    virtual void	setDebugMode(int debugMode) = 0;
    virtual int		getDebugMode() const = 0;

    virtual void    EndDraw() = 0;
    virtual void    BeginDraw() = 0;

    virtual void    setTextSize( const float size ) = 0;
    virtual float   getTextSize() const = 0;

};