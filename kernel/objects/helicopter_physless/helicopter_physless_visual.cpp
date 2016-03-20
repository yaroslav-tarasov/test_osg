#include "stdafx.h"
#include "precompiled_objects.h"
#include "helicopter_physless_visual.h"

#include "common/text_label.h"
#include "ext/spark/SmokeNode.h"
#include "common/morphs_support.h"

namespace helicopter_physless
{
	const double visual::smoke_end_duration_  = 10;
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
		, smoke_sfx_(dynamic_cast<visual_system*>(sys_), this)
        , deffered_init_(true)
	{
        visual_system* vsys = smoke_sfx_.vsys;

#ifndef ASYNC_OBJECT_LOADING         
        //fill_nodes();
        label_object_ = vsys->create_visual_object(nm::node_control_ptr(root()),"text_label.scg");
        ls_ = boost::make_shared<visual_objects::label_support>(label_object_, settings_.custom_label);
#endif
		start_  = boost::bind(&visual::smoke_sfx_t::on_malfunction_changed, &smoke_sfx_, aircraft::MF_FIRE_ON_BOARD );
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

			smoke_sfx_.last_fire_time_ = time;
		}


        geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
#if 0
        geo_base_3 root_pos = root()->get_global_pos();
        quaternion root_orien = root()->get_global_orien();
#endif

#ifdef ASYNC_OBJECT_LOADING 
        if(deffered_init_ && nm::vis_node_control_ptr(root())->vis_nodes().size()>0)
        {
            if(!label_object_ )
	        { 
		        visual_system* vsys = dynamic_cast<visual_system*>(sys_);
		        label_object_ = vsys->create_visual_object(nm::node_control_ptr(root()),"text_label.scg",0,false);
				if(label_object_->root())
					ls_ = boost::make_shared<visual_objects::label_support>(label_object_, settings_.custom_label);
	        }
            
            findNodeVisitor findMorph("rotor_morph",findNodeVisitor::not_exact); 
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

		if (smoke_sfx_.smoke_object_ && engine_node_)
		{
			auto & ss = smoke_sfx_;
			if (nodes_management::vis_node_info_ptr(root())->is_visible())
			{
				geo_base_3 node_pos   = engine_node_->get_global_pos();
				quaternion node_orien = engine_node_->get_global_orien();
				point_3f pos = base(node_pos);

				// ss.smoke_object_->node()->as_transform()->set_transform(cg::transform_4f(cg::as_translation(pos), cg::rotation_3f(node_orien.rotation())));
				ss.smoke_object_->set_visible(true);

				if (ss.smoke_sfx_weak_ptr_)
				{
					ss.smoke_sfx_weak_ptr_->setFactor(ss.smoke_factor_ * cg::clamp(0., smoke_end_duration_, 1., 0.)(time-ss.last_fire_time_));
					ss.smoke_sfx_weak_ptr_->setIntensity(ss.smoke_factor_ * 2);
					ss.smoke_sfx_weak_ptr_->setEmitWorldPos(pos);
				}
			}
			else
				ss.smoke_object_->set_visible(false);
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


        }
    }


	void visual::smoke_sfx_t::on_malfunction_changed( aircraft::malfunction_kind_t kind )
	{
		using namespace aircraft;

		if (kind == MF_FIRE_ON_BOARD || kind == MF_SMOKE_ON_BOARD)
		{
			if (!smoke_object_ && vthis_->engine_node_)
			{
				smoke_object_ = vsys->create_visual_object("sfx//smoke.scg",0,false);
				smoke_sfx_weak_ptr_ = nullptr;
				if (auto smoke_node = findFirstNode(smoke_object_->node().get(),"SmokeFx"))
				{
					smoke_sfx_weak_ptr_ = dynamic_cast<SmokeSfxNode *>(smoke_node);
				}
			}

			if (vthis_->malfunction(MF_SMOKE_ON_BOARD))
				smoke_factor_ = 1;
			else if (vthis_->malfunction(MF_FIRE_ON_BOARD))
				smoke_factor_ = 3;
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

}


