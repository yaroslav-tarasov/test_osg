#pragma once

#include "network/msg_dispatcher.h"
#include "atc/model_structure.h"
#include "kernel/object_info_fwd.h"

FIXME(using network.gen_msg)
using network::gen_msg;
using binary::read_stream; 
using kernel::object_info_ptr;

#include "nodes_manager/nodes_manager_common.h"

namespace nodes_management
{

struct view;

struct node_impl : public node_control
{
public:
    node_impl( osg::Node* n,  view  * manager )
        : node_(n)
        , manager_(manager)
    {
        node_->getUserValue("id",node_id_);
    }

    node_impl(view * manager, node_impl const&  parent, model_structure::node_data const& data, uint32_t id);
    node_impl(view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id);
    node_impl(view * manager, binary::input_stream & stream );

    virtual ~node_impl(){childs_callbacks_.clear();}

public:
    void save( binary::output_stream& stream ) const;
    virtual void pre_update (double time);
    virtual void post_update(double time);

public:
    model_structure::node_data const&   data() const;
    virtual void extrapolated_position_reseted();


//  node_control
private:
    void play_animation  (std::string const& seq, double len, double from, double size) override;
    void set_texture     (std::string const& texture) override;
    void set_visibility  (bool visible) override;
    void set_position    (node_position const& pos) override;
    virtual model_structure::collision_volume const* get_collision() const override;

public:
    virtual void on_object_created   (object_info_ptr object);
    virtual void on_object_destroying(object_info_ptr object);

public:
//  node_info
    node_position const&        position        ()   const override;
    transform_4                 transform       ()   const;
    uint32_t                    node_id         ()   const;
    uint32_t                    object_id       ()   const;
    cg::sphere_3                get_bound       ()   const override;
    std::string const&          name            ()   const override;
    node_info_ptr               rel_node        ()   const override;
    node_info_ptr               root_node       ()   const override;
    cg::geo_point_3             get_global_pos  ()   const override;
    cg::quaternion              get_global_orien()   const override;
    cg::transform_4             get_root_transform() const override;

public:
    void on_msg(binary::bytes_cref data);

public:
    void         on_position(msg::node_pos_descr const& pos);
    virtual void on_texture (msg::node_texture_msg const& m);

protected:
    network::msg_dispatcher<>& msg_disp();

private:
    void init_disp();

public:
//  node_impl
    osg::Node*                  as_osg_node() {return node_.get();}
protected:
    view *      manager_;
    uint32_t    node_id_;

    optional<double>                              time_; 

    model_structure::node_data                    data_;
    osg::ref_ptr<osg::Node>                       node_;
    node_position                                 position_;
    optional<std::string>                         texture_;
    mutable node_position                         extrapolated_position_;
    mutable optional<node_info_ptr>               rel_node_;
    std::vector<osg::ref_ptr<osg::NodeCallback>>  childs_callbacks_;
    mutable std::string                           name_;

protected:
    mutable optional<model_structure::collision_volume const*> collision_;

private:
    network::msg_dispatcher<> msg_disp_;
};

typedef polymorph_ptr<node_impl> node_impl_ptr;

}

