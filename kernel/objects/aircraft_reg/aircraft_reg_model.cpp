#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_model.h"
#include "objects\common\aircraft_physless.h"


namespace aircraft_reg
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict)
    : view                 (oc, dict)
{
    void (model::*on_run)  (net_layer::msg::run const& msg)             = &model::on_inject_msg;
    void (model::*on_malf) (net_layer::msg::malfunction_msg const& msg) = &model::on_inject_msg;

    msg_disp()
        .add<net_layer::msg::run >(boost::bind(on_run      , this, _1))
        .add<net_layer::msg::malfunction_msg >(boost::bind(on_malf      , this, _1))
        ;
}

void model::on_inject_msg(net_layer::msg::run const& msg)
{
     //LogInfo("on_inject_msg: " << msg.ext_id << "; e2n_.size() " << e2n_.size() << "   " << e2n_.size()>0?e2n_.begin()->first:-1);

    auto it_id = e2n_.find(msg.ext_id);

     //for(auto it = e2n_.begin();it!=e2n_.end();++it)
     //{
     //  LogInfo("e2n_: " << it->first << "  =  " << it->second);
     //}

     //if(e2n_.size()>0 && it_id != e2n_.end())
     //    LogInfo("on_inject_msg: " << msg.ext_id << "; e2n_.size() " <<e2n_[msg.ext_id]);

     if(msg.ext_id>0 && it_id != e2n_.end() && e2n_.size()>0 )
     {
         aircraft_physless::info_ptr a = aircrafts_[e2n_[msg.ext_id]];
         // LogInfo("on_inject_msg extern id: " << a?a->extern_id():-1 );
         if(auto pa = aircraft_physless::model_control_ptr(a))
         {
             pa->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pa->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth ); 
         }
     }
}

void model::on_inject_msg(net_layer::msg::malfunction_msg const& msg)
{

    auto it_id = e2n_.find(msg.ext_id);

    if(msg.ext_id>0 && it_id != e2n_.end() && e2n_.size()>0 )
    {
        aircraft_physless::info_ptr a = aircrafts_[e2n_[msg.ext_id]];
        if(auto pa = aircraft_physless::aircraft_ipo_control_ptr(a))
        {

        }
    }
}
                     

} // end of aircraft_reg