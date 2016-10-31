#include "aircraft_visual.h"

#include "common/text_label.h"
#include "common/parachute.h"
#include "common/forsage.h"
#include "common/smoke_support.h"
#include "common/landing_dust_support.h"

#if !defined(VISUAL_EXPORTS)
#include "ext/spark/SmokeNode.h"
#endif

namespace aircraft
{

	object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new visual(oc, dict));
	}

	AUTO_REG_NAME(aircraft_visual, visual::create);

	visual::visual( kernel::object_create_t const& oc, dict_copt dict )
		: view  (oc,dict)
        , vsys_ (dynamic_cast<visual_system*>(sys_))  
	{
        fill_nodes();

        ls_ =  boost::make_shared<visual_objects::label_support_proxy>(); 

#ifndef ASYNC_OBJECT_LOADING  

        ls_ = boost::make_shared<visual_objects::label_support>(
            vsys_->create_visual_object(nm::node_control_ptr(root()),"text_label.scg"), settings_.custom_label);

        ps_ = boost::make_shared<visual_objects::parashute_support>(
            vsys_->create_visual_object(nm::node_control_ptr(root()),"parachute.scg",0,0,false));

#endif
        start_  = boost::bind(&visual::on_malfunction_changed, this, aircraft::MF_FIRE_ON_BOARD );
	}

    void visual::fill_nodes()
    {
        nm::visit_sub_tree(get_nodes_manager()->get_node_tree_iterator(root()->node_id()), [this](nm::node_info_ptr n)->bool
        {
            if (boost::starts_with(n->name(), "engine"))
            {
                this->engines_nodes_.push_back(n);
                return true;
            }
            else
                if (boost::starts_with(n->name(), "forsage"))
                {
                    this->forsage_nodes_.push_back(n);
                    return true;
                }
                else
                    if (boost::starts_with(n->name(), "parachute"))
                    {
                        this->parachute_nodes_.push_back(n);
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

        if(fs_.size()>0)
            std::for_each(fs_.begin(),fs_.end(),
            [this,time](boost::shared_ptr<visual_objects::forsage_support>& fs) {
                fs->set_update_time(time);
        });

        geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
#if 0
        geo_base_3 root_pos = root()->get_global_pos();
        quaternion root_orien = root()->get_global_orien();
#endif

#ifdef ASYNC_OBJECT_LOADING 
        if( !(ls_->get_ls()) && nm::vis_node_control_ptr(root())->vis_nodes().size()>0)
        { 
            *ls_ = boost::make_shared<visual_objects::label_support>(
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

            // fill_nodes();


        }
#endif

        if(forsage_nodes_.size()>0 && fs_.size()>0)
        {
            std::for_each(fs_.begin(),fs_.end(),
                [=](boost::shared_ptr<visual_objects::forsage_support>& fs) {
                    (*fs)->set_visible(root_visible);
                    if(root_visible) fs->update(time, point_3f(), base); 
            });
        }

#if !defined(VISUAL_EXPORTS)
        if (smoke_object_ && engines_nodes_.size()>0)
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
#endif

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

            if (!smoke_sup_ && has_smoke && engines_nodes_.size()>0 )
            {
                smoke_sup_ = boost::make_shared<visual_objects::smoke_support>(
                    vsys_->create_visual_object("sfx//smoke.scg",0,0,false), engines_nodes_[0], root(), damned_offset());
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
            if(fs_.size()>0)
                fs_.clear();
        }
        else if (state.eng_state == aircraft::ES_FORSAGE)
        {
            if(forsage_nodes_.size()>0 && fs_.size() == 0)
            {
                fs_.reserve(forsage_nodes_.size());
                std::for_each(forsage_nodes_.begin(),forsage_nodes_.end(),
                    [this](nm::node_info_ptr fn) {
                        fs_.push_back(boost::make_shared<visual_objects::forsage_support>(
                            vsys_->create_visual_object(nm::node_control_ptr(fn),"sfx//forsage.scg",0,0,false),
                            fn, root(), damned_offset() )); 
                });
            }
        }

        if(state.para_state == aircraft::PS_SHOW)
        {
            if(parachute_nodes_.size()>0 && ps_.size() == 0)
            {
                std::for_each(parachute_nodes_.begin(),parachute_nodes_.end(),
                    [this](nm::node_info_ptr fn) {
                        ps_.push_back(boost::make_shared<visual_objects::parashute_support>(
                            vsys_->create_visual_object(nm::node_control_ptr(fn),fn->name() + ".scg" /*"parachute.scg"*/,0,0,false)
                            )); 
                });
            }

        }
        else
            ps_.clear();


    }

    labels_management::labels_provider_ptr  visual::get_label_provider() const
    {
        return ls_;
    }
}


