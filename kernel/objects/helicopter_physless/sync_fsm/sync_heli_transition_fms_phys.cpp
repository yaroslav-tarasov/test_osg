#include "sync_heli_transition_fms_phys.h"
#include "sync_heli_phys_state.h"
#include "sync_heli_none_state.h"

namespace helicopter_physless
{
namespace sync_fsm
{

void transition_fms_phys_state::update(double time, double /*dt*/) 
{
    if (!phys_aircraft_)
        return;

    double const transition_time = 5;

    FIXME("Наверное очень нужный код");

    //fms::procedure_model_ptr pm =  self_.get_fms_info()->procedure_model();
    //double prediction = cg::clamp(pm->taxi_TAS(), pm->takeoff_TAS(), 15., 30.)(self_.get_fms_info()->get_state().dyn_state.TAS);

    //geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*0.1));
    //phys_aircraft_->go_to_pos(predict_pos, self_.get_fms_info()->get_state().orien());
    //phys_aircraft_->set_air_cfg(self_.get_fms_info()->get_state().dyn_state.cfg);
    //phys_aircraft_->set_prediction(prediction);

    //phys_aircraft_->update();

    //auto physpos = phys_aircraft_->get_position();
    //auto fmspos = self_.fms_pos();

    //double t = cg::clamp(start_transition_time_, start_transition_time_ + transition_time, 0., 1.)(time);
    //auto pos = cg::blend((geo_point_3)fmspos.pos, (geo_point_3)physpos.pos, t);
    //auto orien = cg::blend(fmspos.orien, physpos.orien, t);
    //double smooth = cg::blend(25., 2., t);

    //self_.set_desired_nm_pos(pos);
    //self_.set_desired_nm_orien(orien);
    //self_.set_nm_angular_smooth(smooth);


    //if (time >= start_transition_time_ + transition_time)
    {
        self_.switch_sync_state(/*boost::make_shared<phys_state>*/create_sync_phys_state(self_, phys_aircraft_, base_));
    }
}

void transition_fms_phys_state::on_zone_destroyed( size_t id )
{
    if (phys_aircraft_->get_zone() == id)
        self_.switch_sync_state(boost::make_shared<none_state>(self_));
}


}
}