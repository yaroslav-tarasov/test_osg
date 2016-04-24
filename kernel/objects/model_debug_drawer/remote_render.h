#pragma once 

namespace mdd
{

struct remote_render : public debug_render
{

	remote_render ( mdd::model& sender )
		: sender_ (sender)
	{

	}

	virtual void    setEnabled( bool enable ) override;
	virtual bool    getEnabled() const override;

	virtual void	drawLine( const cg::point_3& from,const cg::point_3& to,const cg::point_3& color ) override;
	virtual void	drawSphere( const cg::point_3& p, float radius, const cg::point_3& color ) override;

	virtual void	drawTriangle(const cg::point_3& a,const cg::point_3& b,const cg::point_3& c,const cg::point_3& color,float alpha) override;
	virtual void	drawContactPoint(const cg::point_3& PointOnB,const cg::point_3& normalOnB,float distance,int lifeTime,const cg::point_3& color) override;
	
	virtual void	reportErrorWarning(const char* warningString) override;
	virtual void	draw3dText(const cg::point_3& location,const char* textString) override;

	virtual void	setDebugMode(int debugMode) override;
	virtual int		getDebugMode() const override;

	virtual void    EndDraw() override;
	virtual void    BeginDraw() override;

	virtual void    setTextSize( const float size ) override;
	virtual float   getTextSize() const override;

private:
	mdd::model& sender_;

};

}