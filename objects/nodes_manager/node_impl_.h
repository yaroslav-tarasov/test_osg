#pragma once

#include "nodes_manager_common.h"
#include "objects/nodes_management.h"
#include "atc/model_structure.h"
#include "phys_sys/sensor.h"

namespace nodes_management
{

struct view;

struct node_impl 
    : node_control
{
    node_impl(view * manager, node_impl const&  parent, model_structure::node_data const& data, uint32_t id);
    node_impl(view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id);
    node_impl(view * manager, binary::input_stream & stream );

public:
    void save( binary::output_stream& stream ) const;
    virtual void pre_update (double time);
    virtual void post_update(double time);

// node_info
public:
    node_position const&    position            () const;
    transform_4             transform           () const;
    uint32_t                node_id             () const;
    uint32_t                object_id           () const;
    string const&           name                () const;
    node_info_ptr           rel_node            () const;
    node_info_ptr           root_node           () const;
    transform_4             get_root_transform  () const;
    geo_point_3             get_global_pos      () const;
    quaternion              get_global_orien    () const;
    model_structure::collision_volume const* get_collision       () const;
    cg::sphere_3            get_bound()            const;
public:
    void on_msg(binary::bytes_cref data);

public:
    void         on_position(msg::node_pos_descr const& pos);
    virtual void on_texture (msg::node_texture_msg const& m);

// node_control
private:
    void set_position   (node_position const& pos);
    void play_animation (string const& seq, double len, double from, double size);
    void set_texture    (string const& texture);
    void set_visibility (bool visible);

public:
    virtual void on_object_created   (object_info_ptr object);
    virtual void on_object_destroying(object_info_ptr object);

public:
    model_structure::node_data const&   data() const;
    virtual void extrapolated_position_reseted();

protected:
    network::msg_dispatcher<>& msg_disp();

private:
    void init_disp();

protected:
    view *      manager_;
    uint32_t    node_id_;

    model_structure::node_data      data_;
    optional<double>                time_;
    bool                            last_freeze_;
    node_position                   position_;
    optional<std::string>           texture_;
    node_position                   extrapolated_position_;
    mutable optional<node_info_ptr> rel_node_;
    
protected:
    mutable optional<model_structure::collision_volume const*> collision_;

private:
    network::msg_dispatcher<> msg_disp_;
};

typedef polymorph_ptr<node_impl> node_impl_ptr;

} // nodes_management

