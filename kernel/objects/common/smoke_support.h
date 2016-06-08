#pragma once 

#include "effect_support.h"

namespace visual_objects
{
    struct smoke_effect_support_traits : effect_support_traits
    {
        static const char*          effect_node_name ()    { return "SmokeFx"; }
        static float                base_alpha       ()    { return 0.2; }
        static float                effect_end_duration () { return 10.0; }
        static float                base_factor       ()   { return 1.0; }
        static float                factor_ratio      ()   { return 0.25; }
    };
    
    
    struct smoke_support : effect_support< smoke_support, SmokeSfxNode, smoke_effect_support_traits >
    {
        friend struct EffectSupportType;

        smoke_support(visual_object_ptr obj , nm::node_info_ptr parent, nm::node_info_ptr root , const cg::transform_4& damned_offset)
           : effect_support(obj, parent, root, damned_offset)
        {}

     private:
        inline void init_()
        {

            if (auto node = findFirstNode(visual_object_->node().get(), traits_.effect_node_name ()))
            {
                effect_weak_ptr_ = dynamic_cast<EffectNodeType *>(node);
                
                // Base alpha?
#if 0
                if(effect_weak_ptr_)
                    effect_weak_ptr_->setBaseAlpha(traits_.base_alpha());
#endif

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

                    visual_object_->set_visible(true);

                    if (effect_weak_ptr_)
                    {
                        effect_weak_ptr_->setFactor      (factor_ * cg::clamp(0., effect_end_duration_, 1., 0.)(time-last_update_time_));
                        effect_weak_ptr_->setIntensity   (factor_ * 2);
                        effect_weak_ptr_->setEmitWorldPos( pos /*+ root_orien.rotate_vector( damned_offset_.translation())*/ );
                    }
                }
                else
                    visual_object_->set_visible(false);
            }
        }
    };
      


}