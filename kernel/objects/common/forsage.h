#pragma once 


namespace visual_objects
{
    struct forsage_support
    {

        SAFE_BOOL_OPERATOR(visual_object_)
        
        forsage_support(visual_object_ptr obj , nm::node_info_ptr parent, nm::node_info_ptr root)
            : visual_object_(obj)
            , parent_ (parent)
            , factor_ (4.0)
            , effect_end_duration_(10.0)
            , last_update_time_ (0.0)
            , root_(root) 
        {
            init_();
        }
        
        __forceinline void update( double time, const point_3f& velocity, geo_base_3 const & base)
        {
            update_(time, velocity, base); 
        }

 		visual_object_ptr get() const
		{
			return visual_object_;
		}

		visual_object * operator->() { return visual_object_.get();}
		visual_object const* operator->() const { return visual_object_.get();}

        __forceinline void set_factor(double factor )  { factor_ = factor;}
        __forceinline void set_update_time (double time )  { last_update_time_ = time;}

    private:

        inline void init_()
        {
            effect_weak_ptr_ = nullptr;
            if (auto fire_node = findFirstNode(visual_object_->node().get(),"FireFx"))
            {
                effect_weak_ptr_ = dynamic_cast<SmokeSfxNode *>(fire_node);
            }

        }


        inline void update_(double time, const point_3f& velocity, geo_base_3 const & base)
        {
            if (visual_object_ && parent_)
            {
                if (root_->is_visible())
                {
                    geo_base_3 node_pos   = parent_->get_global_pos();
                    quaternion node_orien = parent_->get_global_orien();
                    point_3f pos = base(node_pos);
                    
                    quaternion root_orien = nm::node_info_ptr(root_)->get_global_orien();
                    
                    // root_orien.rotate_vector(velocity);

                    visual_object_->set_visible(true);

                    if (effect_weak_ptr_)
                    {
                        effect_weak_ptr_->setFactor      (factor_ * cg::clamp(0., effect_end_duration_, 1., 0.)(time-last_update_time_));
                        effect_weak_ptr_->setIntensity   (factor_ * 40);
                        effect_weak_ptr_->setEmitWorldPos(pos);
                        effect_weak_ptr_->setEmitterWorldVelocity(root_orien.rotate_vector( cg::point_3f(40., 0., 0.) ));  
                    }
                }
                else
                    visual_object_->set_visible(false);
            }

        }
        
        SmokeSfxNode *        effect_weak_ptr_;
        visual_object_ptr     visual_object_;
        nm::node_info_ptr     parent_;
        nm::vis_node_info_ptr   root_;
        double                factor_;

        
        double                last_update_time_;
        double                effect_end_duration_;

    };

}