#pragma once 

namespace mdd
{
    namespace msg 
    {
        //! идентификаторы подвижных объектов
        enum id 
        {
            mdd_state,
            mdd_draw_line,
            mdd_draw_sphere,
            mdd_draw_triangle,
            mdd_draw_contact_point,
            mdd_draw_3d_text
        };


        // 
                                                                                #if 0
virtual void    setEnabled( bool enable ) = 0;
virtual bool    getEnabled() const = 0;

virtual void	reportErrorWarning(const char* warningString) = 0;

virtual void	setDebugMode(int debugMode) = 0;
virtual int		getDebugMode() const = 0;

virtual void    EndDraw() = 0;
virtual void    BeginDraw() = 0;

virtual void    setTextSize( const float size ) = 0;
virtual float   getTextSize() const = 0;
        #endif


        struct state_msg_t
            : network::msg_id<mdd_state>
        {               
            state_msg_t()
            {
            }

            bool            enable;
            char        debug_mode;
        };

        REFL_STRUCT(state_msg_t)
            REFL_ENTRY(enable)
            REFL_ENTRY(debug_mode)
        REFL_END   ()

        // virtual void	   drawLine( const cg::point_3& from,const cg::point_3& to,const cg::point_3& color ) = 0;

        struct draw_line_msg_t
            : network::msg_id<mdd_draw_line>
        {               
            draw_line_msg_t()
            {
            }

            cg::point_3  from;
            cg::point_3    to;
            cg::point_3 color;
        };

        REFL_STRUCT(draw_line_msg_t)
            REFL_ENTRY(from)
            REFL_ENTRY(to)
            REFL_ENTRY(color)
        REFL_END   ()

        // virtual void	drawSphere( const cg::point_3& p, float radius, const cg::point_3& color ) = 0;
        struct draw_sphere_msg_t
            : network::msg_id<mdd_draw_sphere>
        {               
            draw_sphere_msg_t()
            {
            }

            cg::point_3     p;
            float      radius;
            cg::point_3 color;
        };

        REFL_STRUCT(draw_sphere_msg_t)
            REFL_ENTRY(p)
            REFL_ENTRY(radius)
            REFL_ENTRY(color)
        REFL_END   ()

        // virtual void	  drawTriangle(const cg::point_3& a,const cg::point_3& b,const cg::point_3& c,const cg::point_3& color,float alpha) = 0;
        struct draw_triangle_msg_t
            : network::msg_id<mdd_draw_triangle>
        {               
            draw_triangle_msg_t()
            {
            }

            cg::point_3     a;
            cg::point_3     b;
            cg::point_3     c;
            cg::point_3 color;
            float       alpha;

        };

        REFL_STRUCT(draw_triangle_msg_t)
            REFL_ENTRY(a)
            REFL_ENTRY(b)
            REFL_ENTRY(c)
            REFL_ENTRY(color)
            REFL_ENTRY(alpha)
        REFL_END   ()

        // virtual void	drawContactPoint(const cg::point_3& PointOnB,const cg::point_3& normalOnB,float distance,int lifeTime,const cg::point_3& color) = 0;
        struct draw_contact_point_msg_t
            : network::msg_id<mdd_draw_contact_point>
        {               
            draw_contact_point_msg_t()
            {
            }

            cg::point_3  point_on_b;
            cg::point_3  normal_on_b;
            float        distance;
            int          life_time;
            cg::point_3  color;
        };

        REFL_STRUCT(draw_contact_point_msg_t)
            REFL_ENTRY(point_on_b)
            REFL_ENTRY(normal_on_b)
            REFL_ENTRY(distance)
            REFL_ENTRY(life_time)
            REFL_ENTRY(color)
        REFL_END   ()

    } // msg

} // namespace mdd

