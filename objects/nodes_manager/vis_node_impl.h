#pragma once

#include "node_impl.h"


namespace nodes_management
{
    struct visual;

    struct vis_node_impl 
        : node_impl
        , vis_node_control
        , vis_node_info
    {
        vis_node_impl( view * manager, node_impl const&  parent, model_structure::node_data const& data, uint32_t id );
        vis_node_impl( view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id );
        vis_node_impl( view * manager, binary::input_stream & stream );

    private:
        void on_animation   (msg::node_animation const& m);
        void on_visibility  (msg::visibility_msg const& m);
        void on_texture     (msg::node_texture_msg const& m) override;

    private:
        void pre_update  (double time) override;

    public:
        void on_visual_object_created();

    private:
        void fill_victory_nodes();
        void extrapolated_position_reseted() override; 
        void sync_position();

        // vis_node_info
    private:
        vis_nodes_t const& vis_nodes() const;
        bool is_visible() const;

    private:
        void init_disp();

    private:
        mutable optional<bool> visible_;
        std::vector</*victory::node_ptr*/osg::ref_ptr<osg::Node>> victory_nodes_;

        bool user_visible_;

        bool need_update_;
    };

    typedef polymorph_ptr<vis_node_impl> vis_node_impl_ptr;

}