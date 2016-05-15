#pragma once

#include "nodes_manager_common.h"
#include "nm/node_impl.h"

#include "objects/nodes_management.h"

namespace nodes_management
{
using model_structure::collision_structure;

struct nodes_data 
{
    nodes_data()
        : contains_model_(false)
    {
    }

protected:
    settings_t      settings_;
    bool            contains_model_;
    mutable bytes_t model_data_;

    REFL_INNER(nodes_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(contains_model_)
        REFL_SER_BIN(model_data_)
    REFL_END()
};

struct view
    : base_view_presentation
    , obj_data_holder<nodes_data>
    , manager
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

public:
    object_collection const*    collection () const;
    kernel::system const*       system     () const;
    optional<double>            update_time() const;
    optional<double>            last_update_time() const;
    collision_structure const*  get_collision_structure() const;
    /*inline */cg::transform_4  get_relative_transform(/*manager_ptr manager,*/ node_info_ptr node, node_info_ptr rel );

public:
    void set_node_msg (uint16_t node_id, binary::bytes_t&& data, bool sure = true);
    void send_node_msg(uint16_t node_id, binary::bytes_t&& data, bool sure = true);

public:
    virtual void init();

protected:
    virtual void apply_model(string const& model);

protected:
    view(kernel::object_create_t const& oc, dict_copt dict);

private:
    void save(dict_ref  dict, bool safe_key) const override;
    
private:
    void on_model       (msg::model_msg const&);
    void on_node_data   (msg::node_data const&);
    
// manager
public:
    node_info_ptr   get_node    (uint32_t node_id) const;
    node_info_ptr   find_node   (string const& name) const;
    void            set_model   (string const& model, bool save_root_pos);
    string const&   get_model   () const;
    void            visit_nodes (boost::function<void(node_info_ptr)> const& f) const;
    node_tree_iterator_ptr get_node_tree_iterator(uint32_t node_id) const;

// base_presentation
private:
    void pre_update (double time);
    void post_update(double time);

private:
    void on_object_created   (object_info_ptr object) override;
    void on_object_destroying(object_info_ptr object) override;

private:
    void create_node_hierarchy(geo_position const& pos, binary::input_stream& model_stream);
    void create_node_hierarchy(node_impl const& parent, binary::input_stream& model_stream);

    void init_collision_volume();
    void init_logic_tree      ();

// history_prs
private:
    uint32_t msg_sub_id(size_t /*id*/, binary::bytes_cref /*data*/) const;

protected:
    shared_ptr<collision_structure> collision_structure_;
    fixed_id_vector<node_impl_ptr>  nodes_;
};

FIXME(¬ью все в одном)

#if 0
struct chart
    : view
    , chart_presentation_base
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict)
    {
        view *obj = new chart(oc, dict);
        object_info_ptr info(obj);
        obj->init();
        return info;
    }

private:
    chart(kernel::object_create_t const& oc, dict_copt dict)
        : view(oc, dict)
        , chart_presentation_base(oc, this)
    {
    }
};
#endif

} // nodes_management
