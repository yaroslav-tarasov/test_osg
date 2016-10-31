#include "mdd_model.h"
#include "common/debug_render.h"
#include "remote_render.h"


#if 0
void on_state              (msg::state_msg_t const& msg);
void on_draw_triangle      (msg::draw_triangle_msg_t const& msg);
void on_draw_contact_point (msg::draw_contact_point_msg_t const& msg);
#endif

namespace mdd
{

  void    remote_render::setEnabled( bool enable ) {}
  bool    remote_render::getEnabled() const {  return true; }

  void	remote_render::drawLine( const cg::point_3& from,const cg::point_3& to,const cg::point_3& color ) 
  {
	  sender_.set(msg::draw_line_msg_t(from, to, color),false);
  }

  void	remote_render::drawSphere( const cg::point_3& p, float radius, const cg::point_3& color )
  {
	 sender_.set(msg::draw_sphere_msg_t(p,radius,color),false);
  }

  void	remote_render::drawTriangle(const cg::point_3& a,const cg::point_3& b,const cg::point_3& c,const cg::point_3& color,float alpha) 
  {

  }

  void	remote_render::drawContactPoint(const cg::point_3& PointOnB,const cg::point_3& normalOnB,float distance,int lifeTime,const cg::point_3& color) 
  {
	  sender_.set(msg::draw_contact_point_msg_t( PointOnB, normalOnB, distance, lifeTime, color),false);
  }

  void	remote_render::reportErrorWarning(const char* warningString) {}
  void	remote_render::draw3dText(const cg::point_3& location,const char* textString) {}

  void	remote_render::setDebugMode(int debugMode) {}
  int		remote_render::getDebugMode() const { return dbg_DrawWireframe;}

  void    remote_render::EndDraw() {}
  void    remote_render::BeginDraw() {}

  void    remote_render::setTextSize( const float size ) {}
  float   remote_render::getTextSize() const { return 0;}

}