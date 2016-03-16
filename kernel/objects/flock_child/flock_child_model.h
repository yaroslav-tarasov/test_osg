#pragma once

#include "flock_child_common.h"
#include "flock_child_view.h"
#include "common/phys_sys.h"
#include "objects/nodes_management.h"
#include "common/phys_object_model_base.h"

namespace flock
{

namespace child
{

struct model
    : model_presentation        // базовый класс, виртуальрость
    , view                      // базовое представление
    , phys_object_model_base    // базовый класс для объектов с физическими моделями
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

private:
    model( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
private:
    void update(double time) override;
	void pre_update(double time) override;

    // base_view_presentation
private:
    void on_child_removing(object_info_ptr child) override;

private:
    
    void create_phys        ();
    void sync_phys          ( double dt );
    void sync_nodes_manager ( double dt );
    void update_model       ( double time, double dt );

private:
    void flap();
    void dive();
    void soar();
    void wander(float delay);

private:
    phys::static_mesh_ptr                  mesh_;

    optional<size_t>                  phys_zone_;
    
    cg::geo_base_3                root_next_pos_;
    cg::quaternion              root_next_orien_;

	// Some data
private:

	bool									_soar; // state ?
	bool								   _dived;
	bool                                 _landing;

    double                                 _speed;
	double  						   _soarTimer;
	float								 _damping;
    float			    			 _lerpCounter;
	bool							 _flatFlyDown;
    float                           _stuckCounter;
	bool						  _landingSpotted;

//  phys staff
private:
    model_system *                           sys_;
	optional<double>                 last_update_;
	phys::flock::info_ptr             phys_model_;
    
    cg::geo_point_3                 desired_position_;
    quaternion                      desired_orien_;

private: 
	boost::function<void()>                start_;
    
	enum flock_state
	{
		fl_soar,
		fl_dive,
		fl_flap
	};

	flock_state                     flock_state_;
};

} // child

} // end of namespace flock
