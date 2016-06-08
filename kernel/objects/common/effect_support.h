#pragma once 


namespace visual_objects
{
    
    struct effect_support_traits
    {
        static const char*          effect_node_name ()    { return "FireFx"; }
        static float                base_alpha       ()    { return 0.2; }
        static float                effect_end_duration () { return 10.0; }
        static float                base_factor       ()   { return 4.0; }
    };

    template <  typename T, typename EffectInterface, typename Traits >
    struct effect_support 
    {
        typedef EffectInterface  EffectNodeType;
        typedef effect_support<T, EffectInterface, Traits> EffectSupportType;

        SAFE_BOOL_OPERATOR(visual_object_)
        
        effect_support(visual_object_ptr obj , nm::node_info_ptr parent, nm::node_info_ptr root , const cg::transform_4& damned_offset)
            : visual_object_      (obj)
            , effect_weak_ptr_    (nullptr)
            , parent_             (parent)
            , factor_             (traits_.base_factor())
            , effect_end_duration_(traits_.effect_end_duration ())
            , last_update_time_   (0.0)
            , root_               (root) 
            , damned_offset_      (damned_offset)
        {
            static_cast<T*>(this)->init_();
        }
        
        __forceinline void update( double time, const point_3f& velocity, geo_base_3 const & base)
        {
            static_cast<T*>(this)->update_(time, velocity, base); 
        }

 		visual_object_ptr get() const
		{
			return visual_object_;
		}

		visual_object * operator->() { return visual_object_.get();}
		visual_object const* operator->() const { return visual_object_.get();}

        __forceinline void set_factor(double factor )  { factor_ = factor;}
        __forceinline void set_update_time (double time )  { last_update_time_ = time;}


    protected:
        
        EffectNodeType *      effect_weak_ptr_;
        visual_object_ptr     visual_object_;
        nm::node_info_ptr     parent_;
        nm::vis_node_info_ptr root_;
        double                factor_;

        cg::transform_4       damned_offset_;

        double                last_update_time_;
        double                effect_end_duration_;

        const Traits          traits_;

    };

}