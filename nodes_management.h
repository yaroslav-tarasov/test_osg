#pragma once

#include "nodes_management_fwd.h"
#include "cpp_utils/polymorph_ptr.h"

namespace nodes_management
{

struct node_info
{
    virtual ~node_info(){}

    //virtual node_position const&         position   () const = 0;
    //virtual transform_4                  transform  () const = 0;
    //virtual uint32_t                     node_id    () const = 0;
    //virtual uint32_t                     object_id  () const = 0;
    virtual std::string const&             name       () const = 0;

    //virtual node_info_ptr rel_node() const = 0;
    //virtual node_info_ptr root_node() const = 0;

    //virtual transform_4 get_root_transform() const = 0;
    //virtual geo_point_3 get_global_pos() const = 0;
    //virtual quaternion get_global_orien() const = 0;

    //virtual model_structure::collision_volume const* get_collision() const = 0;
    virtual cg::sphere_3 get_bound() = 0;
};


struct node_control
    : node_info
{
    virtual ~node_control(){}

    //virtual void set_position    (node_position const& pos) = 0;
    virtual void play_animation  (std::string const& seq, double len, double from, double size) = 0;
    virtual void set_texture     (std::string const& texture) = 0;
    virtual void set_visibility  (bool visible) = 0;
};

struct manager
{
    virtual ~manager(){}

    virtual node_info_ptr   get_node    (uint32_t node_id)   const  = 0;
    virtual node_info_ptr   find_node   (std::string const& name) const  = 0;
    
    // set_model fully reinitialize model and reset positions of all nodes and root
    
    //virtual void            set_model   (string const& model, bool save_root_pos = true)  = 0;
    //virtual string const&   get_model   () const                    = 0;
    //virtual void            visit_nodes (boost::function<void(node_info_ptr)> const& f) const = 0;

    //virtual node_tree_iterator_ptr get_node_tree_iterator(uint32_t node_id) const = 0;

    //DECLARE_EVENT(model_changed, ());
};

void visit_sub_tree(node_info_ptr root, std::function<bool(node_info_ptr)> f);

cg::transform_4 get_relative_transform(manager_ptr manager, node_info_ptr node, node_info_ptr rel = nullptr);


} //nm namespace