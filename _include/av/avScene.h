#pragma once

#include "avVisualFwd.h"
#include "avEnvironmentParams.h"
#include "application/application_fwd.h"

namespace fms
{
    struct trajectory;
    typedef polymorph_ptr<trajectory> trajectory_ptr;
};


namespace av
{

#if 0
    // debug renderer
    struct debug_render : ref_counter
    {
        enum debug_instances
        {
            lines = 1 << 0,
            faces = 1 << 1,
            all = 0xFFFFU // has to be last
        };
        // visibility control
        virtual void set_visible( unsigned inst_mask ) = 0;
        // clear all
        virtual void clear( unsigned inst_mask = all ) = 0;

        // face control
        virtual void set_face_color( cg::coloraf const & col ) = 0;
        virtual void set_face_outline( cg::coloraf const & col ) = 0;
        virtual void add_face( cg::point_3f const & v0, cg::point_3f const & v1, cg::point_3f const & v2 ) = 0;
        // line control
        virtual void set_line_color( cg::coloraf const & col ) = 0;
        virtual void add_line( cg::point_3f const & v0, cg::point_3f const & v1 ) = 0;
    };
    typedef boost::intrusive_ptr<debug_render> debug_render_ptr;
#endif


    // environment and weather - everything will be here
    struct environment_weather /*: ref_counter*/
    {
        // env params
        virtual void SetEnvironment( environment_params const & p ) = 0;
        virtual environment_params const & GetEnvironment() const = 0;

        // global weather params
        virtual void SetWeather( weather_params const & p ) = 0;
        virtual weather_params const & GetWeather() const = 0;

        // local weather types
        struct local_bank_data
        {
            cg::point_3f                       pos;
            float                              heading;
            cg::point_2f                       ellipse;
            weather_params::precipitation_type type;
            float                              intensity;
            float                              central_portion;

            local_bank_data() : heading(0), ellipse(600.f, 300.f), type(weather_params::rain), intensity(1), central_portion(0.6f) {}
        };
        typedef uint32_t local_bank_id_t;

        // local weather banks control
        virtual void UpdateLocalBank( local_bank_id_t id, local_bank_data const & data ) = 0;
        virtual void RemoveLocalBank( local_bank_id_t id ) = 0;
    };
    // typedef boost::intrusive_ptr<environment_weather> environment_weather_ptr;

    // interface declaration
    struct ITrajectoryDrawer /*: ref_counter*/
    {
        virtual void set(const fms::trajectory_ptr traj,const cg::coloraf& color) = 0;
    };

    struct CameraParam
    {
        cg::point_3     pos;
        cg::quaternion  orien;
    };

    typedef vector<CameraParam>  SceneCamsList;

    // interface declaration
    struct IScene /*: ref_counter*/
    {
        virtual environment_weather*    getEnvWeather         () const = 0;

        virtual bool                    PreUpdate() = 0;
#if 0
        virtual debug_render_ptr        get_scene_debug_renderer() const = 0;
#endif
        virtual ITrajectoryDrawer*      GetTrajectoryDrawer() const = 0;

        virtual app::main_window_ptr    GetMainGUIWindow()  const = 0;

		DECLARE_EVENT(gui_ready, ()) ;
    };


}
