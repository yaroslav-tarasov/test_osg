#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_manager_model.h"


namespace flock
{

namespace manager
{


object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc, dict));
}


AUTO_REG_NAME(flock_manager_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    , phys_object_model_base(collection_)
{

}

void model::set_desired        (double time, const cg::point_3& pos, const cg::quaternion& orien, const double speed )
{
    decart_position target_pos;

    target_pos.pos   = pos;
    target_pos.orien = orien;
    geo_position gtp(target_pos, get_base());

    if(!traj_)
    {
        traj_ = fms::trajectory::create();
        // LogInfo(" Re-create trajectory  " << settings_.custom_label );
    }

    traj_->append(time, pos, orien, speed);
}

void model::set_ext_wind       (double speed, double azimuth) 
{
    FIXME(Need some wind)
}

void model::update( double time )
{   
    view::update(time);

    double dt = time - (last_update_ ? *last_update_ : 0);

    if (!cg::eq_zero(dt))
    {
        if ( traj_ && traj_->base_length() - time < -0.1 )
        {
            const cg::geo_base_3& base = phys_->get_base(*phys_zone_); 

            decart_position cur_pos = decart_position(traj_->kp_value(time),traj_->curs_value(time));
            geo_position cur_glb_pos(cur_pos, base);
            set_state(state_t(cur_glb_pos.pos, cur_pos.orien)); 
        }

        last_update_ = time;
    }

}

} // manager

} // flock