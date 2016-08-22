#pragma once 
#include "airport_view.h"
#include "network/msg_dispatcher.h"
#include "objects/impl/local_position.h"
#include "kernel/systems/vis_system.h"

namespace airport 
{

struct vis
    : view 
    , vis_info
    , visual_control
{
    static object_info_ptr create(object_create_t const& oc, dict_copt dict);

	struct camera_t
	{
		camera_t( const std::string&  name, const geo_position& pos)
			: name ( name )
			, gp  ( pos  )
		    {}

		std::string  name;
		geo_position gp;  
	};

private:
    vis(object_create_t const& oc, dict_copt dict);

// vis_info
private:
    bool is_visible() const ;

private:
    void on_new_settings() override;
    void on_model_changed() override;
    void on_new_pov(uint32_t old_pov) override;

	void on_gui_ready();
	void on_switch_current_camera(uint32_t num );
    void retreive_camera();
#if 0
    void place_lights   ();
    void place_marking  ();
#endif

private:
    geo_point_3 camera_pos  () const;
    cpr         camera_orien() const;
    double      zoom () const; 


private:
    geo_point_3 pos  () const override;
    cpr         orien() const override;

private:
    kernel::visual_system     * vis_sys_;

private:
#if 0
    vector<victory::node_ptr>   lamps_;
#endif


    vector<camera_t>            cameras_;
	vector<size_t>              menu_items_;

};

} // airport 