#include "stdafx.h"
#include "precompiled_objects.h"
#include "aircraft_physless_visual.h"

#include "common/text_label.h"
#include "common/parachute.h"
#include "common/forsage.h"
#include "common/smoke_support.h"
#include "common/landing_dust_support.h"
#include "ext/spark/SmokeNode.h"


namespace aircraft_physless
{
	const double visual::sparks_end_duration_ = 0.2;
}

namespace aircraft_physless
{

	object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new visual(oc, dict));
	}

	AUTO_REG_NAME(aircraft_physless_visual, visual::create);

	visual::visual( kernel::object_create_t const& oc, dict_copt dict )
		: view      (oc,dict)
        , vsys_     (dynamic_cast<visual_system*>(sys_))  
	{

#ifndef ASYNC_OBJECT_LOADING  
        
        fill_nodes();

        ls_ = boost::make_shared<visual_objects::label_support>(
            vsys->create_visual_object(nm::node_control_ptr(root()),"text_label.scg"), settings_.custom_label);

        ps_ = boost::make_shared<visual_objects::parashute_support>(
            vsys->create_visual_object(nm::node_control_ptr(root()),"parachute.scg",0,0,false));

#endif
        start_  = boost::bind(&visual::on_malfunction_changed, this, aircraft::MF_FIRE_ON_BOARD );

    }


    void visual::fill_nodes()
    {
        nm::visit_sub_tree(get_nodes_manager()->get_node_tree_iterator(root()->node_id()), [this](nm::node_info_ptr n)->bool
        {
            if (boost::starts_with(n->name(), "engine_l"))
            {
                this->engine_node_ = n;
                return true;
            }
            else
                if (boost::starts_with(n->name(), "forsage"))
                {
                    this->forsage_node_ = n;
                    return true;
                }
                else
                if (boost::starts_with(n->name(), "rotordyn") || boost::starts_with(n->name(), "rotorsag"))
                {
                    nm::vis_node_control_ptr(n)->set_visibility(false);
                    return true;
                }
                else if (boost::starts_with(n->name(), "rotor"))
                {
                    nm::vis_node_control_ptr(n)->set_visibility(true);
                    return true;
                }
                return true;
        });
    }

    void visual::update(double time)
    {
        view::update(time);
        update_len(time);

        double dt = time - (last_update_ ? *last_update_ : 0);
        //if (cg::eq_zero(dt))
        //    return;        

        bool root_visible = nodes_management::vis_node_info_ptr(root())->is_visible();

        bool has_smoke = malfunction(aircraft::MF_FIRE_ON_BOARD) || malfunction(aircraft::MF_SMOKE_ON_BOARD);
        
		if (has_smoke)
		{
			if(start_)
			{
				start_();
				start_.clear();
			}

            if(smoke_sup_)
                smoke_sup_->set_update_time(time);
		}

        if(fs_)
            fs_->set_update_time(time);


        geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
#if 0
        geo_base_3 root_pos = root()->get_global_pos();
        quaternion root_orien = root()->get_global_orien();
#endif

#ifdef ASYNC_OBJECT_LOADING 
		if( !ls_ && nm::vis_node_control_ptr(root())->vis_nodes().size()>0)
		{ 
            ls_ = boost::make_shared<visual_objects::label_support>(
                    vsys_->create_visual_object(nm::node_control_ptr(root()),"text_label.scg",0,0,false), settings_.custom_label);


#if 0
            if (!landing_dust_object_)
                landing_dust_object_ = vsys_->create_visual_object("sfx//landing_dust.scg",0,0,false);

            if (landing_dust_object_)
            {
                landing_dust_weak_ptr_ = nullptr;
                if (auto landing_dust_node = findFirstNode(landing_dust_object_->node().get(),"LandingDustFx"))
                {
                    landing_dust_weak_ptr_ = dynamic_cast<LandingDustSfxNode *>(landing_dust_node);
                }
            }
#endif

            if(!lds_)
            {
                lds_ = boost::make_shared<visual_objects::landing_dust_support>(
                    vsys_->create_visual_object("sfx//landing_dust.scg",0,0,false), root(), root(), damned_offset());
            }

            fill_nodes();
                     

		}
#endif
        
        if(forsage_node_ && fs_)
        {
            if (root_visible)
            {
                (*fs_)->set_visible(true);
                fs_->update(time, point_3f(), base);
            }
            else
            {
                (*fs_)->set_visible(false);
            }
        }

        if (smoke_object_ && engine_node_)
        {
            if (root_visible)
            {
           
                auto p = dynamic_cast<SmokeNode*>(smoke_object_->node().get());
                
                if(p)
                {
                    point_2 dir = cg::polar_point_2(lp_.wind_speed, lp_.wind_azimuth);
                    p->setGravity(to_osg_vector3(cg::point_3(dir)));
                }

                smoke_object_->set_visible(true);

            }
            else
                smoke_object_->set_visible(false);
        }
        
#if 0
        if (landing_dust_object_)
        {
            landing_dust_object_->set_visible(nodes_management::vis_node_info_ptr(root())->is_visible());
        }
#endif

        if (smoke_sup_)
        {
            smoke_sup_->update(time, point_3f(), base);
        }

        last_update_ = time;
    }

    void visual::on_malfunction_changed( aircraft::malfunction_kind_t kind )
    {
        if (kind == aircraft::MF_FIRE_ON_BOARD || kind == aircraft::MF_SMOKE_ON_BOARD)
        {
            visual_system* vsys = dynamic_cast<visual_system*>(sys_);

            bool has_smoke = malfunction(aircraft::MF_FIRE_ON_BOARD) || malfunction(aircraft::MF_SMOKE_ON_BOARD);

#if 0
            if (!smoke_object_ && has_smoke && engine_node_)
            {
                smoke_object_ = vsys->create_visual_object(nm::node_control_ptr(engine_node_),"smoke");
            }
#endif
            
            if (!smoke_sup_ && has_smoke && engine_node_ )
            {
                 smoke_sup_ = boost::make_shared<visual_objects::smoke_support>(
                    vsys_->create_visual_object("sfx//smoke.scg",0,0,false), engine_node_, root(), damned_offset());
            }

            if (smoke_sup_)
            {
                if (malfunction(aircraft::MF_SMOKE_ON_BOARD))
                    smoke_sup_->set_factor(1.);
                else if (malfunction(aircraft::MF_FIRE_ON_BOARD))
                    smoke_sup_->set_factor(3.);
            }



        }
    }

    void visual::on_new_wheel_contact_effect(double time, point_3f vel, point_3f offset)
    {
        geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;

#if 0
        geo_base_3 root_pos   = root()->get_global_pos();
        quaternion root_orien = root()->get_global_orien();

        point_3f pos = base(root_pos) + root_orien.rotate_vector(offset);

        if (landing_dust_object_)
            landing_dust_weak_ptr_->makeContactDust(time, pos, vel);
#endif
        if(lds_)
        {
            lds_->make_contact_dust(time, vel, offset, base);
        }
    }
    
    void visual::on_equipment_state_changed( aircraft::equipment_state_t state )
    {
        if (state.eng_state < aircraft::ES_FORSAGE)
        {
            if(fs_)
                fs_.reset();
        }
        else if (state.eng_state == aircraft::ES_FORSAGE)
        {
            if(forsage_node_ && !fs_)
            {
                fs_ = boost::make_shared<visual_objects::forsage_support>(
                    vsys_->create_visual_object("sfx//forsage.scg",0,0,false),
                    forsage_node_, root(), damned_offset() );
            }
        }

        if(state.para_state == aircraft::PS_SHOW)
        {
            if(!ps_)
               ps_ = boost::make_shared<visual_objects::parashute_support>(
                vsys_->create_visual_object(nm::node_control_ptr(root()),"parachute.scg",0,0,false));
            
        }
        else
        {
            if(ps_)
                ps_.reset();
        }


    }


}


