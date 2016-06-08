#pragma once 

#include "effect_support.h"

namespace visual_objects
{
    struct ld_effect_support_traits : effect_support_traits
    {
        static const char*          effect_node_name ()    { return "LandingDustFx"; }
        static float                base_alpha       ()    { return 0.2; }
        static float                effect_end_duration () { return 10.0; }
        static float                base_factor       ()   { return 1.0; }
        static float                factor_ratio      ()   { return 0.25; }
    };
    
    
    struct landing_dust_support : effect_support< landing_dust_support, LandingDustSfxNode, ld_effect_support_traits >
    {
        friend struct EffectSupportType;

        landing_dust_support(visual_object_ptr obj , nm::node_info_ptr parent, nm::node_info_ptr root , const cg::transform_4& damned_offset)
           : effect_support(obj, parent, root, damned_offset)
        {}

        inline void make_contact_dust(double time, point_3f const & vel, point_3f const & offset, geo_base_3 const & base)
        {
            if (visual_object_ && parent_)
            {
                if (root_->is_visible())
                {
                    visual_object_->set_visible(true);

                    if (effect_weak_ptr_)
                    {
                        geo_base_3 root_pos   = nm::node_info_ptr(root_)->get_global_pos();
                        quaternion root_orien = nm::node_info_ptr(root_)->get_global_orien();

                        point_3f pos = base(root_pos) + root_orien.rotate_vector(offset);

                        if (visual_object_)
                            effect_weak_ptr_->makeContactDust(time, pos, vel);

                    }
                }
                else
                    visual_object_->set_visible(false);
            }
        }


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
        }
    };
      


}