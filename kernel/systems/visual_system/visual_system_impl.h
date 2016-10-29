#pragma once

#include "kernel/systems/vis_system.h"
#include "kernel/systems/impl/system_base.h"

#include "geometry/camera.h"
#include "objects/nodes_management.h"

#include "av/IVisual.h"

namespace kernel
{

	struct visual_system_impl
		: visual_system
		, visual_system_props
		, system_base
		, av::IFreeViewControl
		//, victory::widget_control
	{
		visual_system_impl(msg_service& service, av::IVisualPtr vis, vis_sys_props const& props);

	private:
		void update       (double time) override;
		void load_exercise(dict_cref dict) override;

		// visual_system
	private:
#ifdef ASYNC_OBJECT_LOADING
		visual_object_ptr       create_visual_object( std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0 , bool async=true) override;
		visual_object_ptr       create_visual_object( nm::node_control_ptr parent, std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0, bool async=true ) override;
#else
		visual_object_ptr       create_visual_object( std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0 , bool async=false) override;
		visual_object_ptr       create_visual_object( nm::node_control_ptr parent, std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0, bool async=false ) override;
#endif

	private:
		void                    visual_object_created( uint32_t seed );
		// visual_system_props
	private:

		vis_sys_props const&    vis_props   () const;
		void                    update_props(vis_sys_props const&);

	private:
		void            init_eye  ();
		void            update_eye();
		cg::camera_f    eye_camera() const;
		void            object_destroying(object_info_ptr object);

	private:
		virtual void  FreeViewOn()  {free_cam_ = true;}
		virtual void  FreeViewOff() {free_cam_ = false;} 

	private:
		av::IVisualPtr            visual  () override;
		av::IScenePtr             scene   () override;

	private:
		DECL_LOGGER("vis_sys");

	private:
		vis_sys_props           props_;

		av::IVisualPtr          vis_;
		av::IScenePtr           scene_;
		//victory::IViewportPtr viewport_;

		scoped_connection       object_destroying_connection_;

		//private:
		//    void init_frustum_projection();

	private:
		visual_control_ptr      eye_;
		bool                    free_cam_;


		std::set<uint32_t>      objects_to_create_;
		bool                    ready_;
		uint32_t                obj_counter_;


	};

} // kernel
