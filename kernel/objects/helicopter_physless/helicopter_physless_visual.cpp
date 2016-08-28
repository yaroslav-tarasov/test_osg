#include "stdafx.h"
#include "precompiled_objects.h"
#include "helicopter_physless_visual.h"

#include "ext/spark/SmokeNode.h"
#include "common/morphs_support.h"
#include "common/text_label.h"
#include "common/parachute.h"
#include "common/forsage.h"
#include "common/smoke_support.h"

namespace helicopter_physless
{
	const double visual::sparks_end_duration_ = 0.2;
}


namespace helicopter_physless
{

	object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new visual(oc, dict));
	}

	AUTO_REG_NAME(helicopter_physless_visual, visual::create);

	visual::visual( kernel::object_create_t const& oc, dict_copt dict )
		: view      (oc,dict)
        , deffered_init_(true)
        , vsys_     (dynamic_cast<visual_system*>(sys_))  
	{

	 	ls_ =  boost::make_shared<visual_objects::label_support_proxy>();

#ifndef ASYNC_OBJECT_LOADING         
        //fill_nodes();
        label_object_ = vsys->create_visual_object(nm::node_control_ptr(root()),"text_label.scg");
        ls_ = boost::make_shared<visual_objects::label_support>(label_object_, settings_.custom_label);
#endif
		// start_  = boost::bind(&visual::smoke_sfx_t::on_malfunction_changed, &smoke_sfx_, aircraft::MF_FIRE_ON_BOARD );
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
                if (boost::starts_with(n->name(), "rotordyn") || boost::starts_with(n->name(), "rotorstat"))
                {
                    nm::vis_node_control_ptr(n)->set_visibility(false);
                    return true;
                }
                else if (boost::starts_with(n->name(), "rotorsag"))
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


        geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
#if 0
        geo_base_3 root_pos = root()->get_global_pos();
        quaternion root_orien = root()->get_global_orien();
#endif

#ifdef ASYNC_OBJECT_LOADING 
        if(deffered_init_ && !(ls_->get_ls()) && nm::vis_node_control_ptr(root())->vis_nodes().size()>0)
        {
			*ls_ = boost::make_shared<visual_objects::label_support>(
				vsys_->create_visual_object(nm::node_control_ptr(root()),"text_label.scg",0,0,false), settings_.custom_label);
            
            FindNodeVisitor findMorph("rotor_morph",FindNodeVisitor::not_exact); 
            nm::vis_node_control_ptr(root())->vis_nodes()[0]->accept(findMorph);
            
            auto const& nl =  findMorph.getNodeList();
            
            if(nl.size()>0)
            {
                ms_ = boost::make_shared<visual_objects::morphs_support>();
                std::for_each(nl.begin(),nl.end(),[&](osg::Node* node ){ms_->add(node);});
            }
			
			fill_nodes();

            deffered_init_ = false;
        }
#endif
        FIXME(Где инициализация?)
		//fill_nodes();

        if (smoke_object_ && engine_node_)
        {
            if (nodes_management::vis_node_info_ptr(root())->is_visible())
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

            bool has_smoke = malfunction(aircraft::MF_FIRE_ON_BOARD) || malfunction(aircraft::MF_SMOKE_ON_BOARD);

#if 0
            if (!smoke_object_ && has_smoke && engine_node_)
            {
                smoke_object_ = vsys_->create_visual_object(nm::node_control_ptr(engine_node_),"smoke");
            }
#endif
            if (!smoke_sup_ && has_smoke && engine_node_ )
            {
                cg::transform_4 tr = get_nodes_manager()->get_relative_transform(damned_offset());
                smoke_sup_ = boost::make_shared<visual_objects::smoke_support>(
                    vsys_->create_visual_object("sfx//smoke.scg",0,0,false), engine_node_, root(), tr);
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

    void visual::on_rotor_state  (double target, double speed, rotor_state_t visible) 
    {
          if(ms_)
          {
              ms_->set_weight(0,target);
              ms_->set_visibility( visible != rs_dynamic );
          }
    }

	labels_management::labels_provider_ptr  visual::get_label_provider() const
	{
		return ls_;
	}
}


