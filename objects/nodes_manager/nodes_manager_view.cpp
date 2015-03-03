#include "stdafx.h"
#include "precompiled_objects.h"

#include "nodes_manager_view.h"
#include "nodes_manager_common.h"
#include "node_tree_iterator.h"
#include "nodes_factory.h"

namespace nodes_management
{

object_info_ptr view::create(kernel::object_create_t const& oc/*, dict_copt dict*/)
{
    view *obj = new view(oc/*, dict*/);
    object_info_ptr info(obj);
    obj->init();
    return info;
}

object_collection const* view::collection() const 
{ 
    return collection_; 
}

kernel::system const* view::system() const 
{ 
    return &*sys_; 
}

optional<double> view::update_time() const
{
    return sys_->update_time();
}

model_structure::collision_structure const* view::get_collision_structure() const
{
    return collision_structure_ 
        ? &*collision_structure_ 
        : NULL;
}

void view::set_node_msg(uint16_t node_id, binary::bytes_t&& data, bool sure)
{          
    set(msg::node_data(node_id, move(data)), sure);
}

void view::send_node_msg(uint16_t node_id, binary::bytes_t&& data, bool sure)
{
    send_cmd(msg::node_data(node_id, move(data)), sure);
}

void view::save(dict_ref dict, bool safe_key) const
{
    binary::output_stream os;
    write(os, (binary::size_type)nodes_.size());
    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        (*it)->save(os);
    
    model_data_ = move(os.detach());
    base_view_presentation::save(dict, safe_key);
}

void view::init()
{
    init_collision_volume();

    if (contains_model_)
    {
        binary::input_stream is(model_data_);
        binary::size_type count;

        read(is, count);
        for (binary::size_type i = 0; i < count; ++i)
        {
            auto nod = create_node(this, is);
            nodes_.insert(nod->node_id(), nod);
        }
    }
    else
    {
        init_logic_tree();
        contains_model_ = true;
    }
}

void view::apply_model(string const& model)
{
    settings_.model = model;
    init_collision_volume();
    init_logic_tree();
}

view::view(kernel::object_create_t const& oc/*, dict_copt dict*/)
    : base_view_presentation(oc)
    //, obj_data_base         (dict)
{
    msg_disp()
        .add<msg::model_msg>(boost::bind(&view::on_model    , this, _1))
        .add<msg::node_data>(boost::bind(&view::on_node_data, this, _1));
}

void view::on_model(msg::model_msg const& m)
{
    apply_model(m.model);    
    model_changed_signal_();
}

void view::on_node_data(msg::node_data const& m)
{
    if (nodes_.valid(m.node_id))
        node_impl_ptr(nodes_[m.node_id])->on_msg(m.data);
}

node_info_ptr view::get_node(uint32_t node_id) const
{
    Assert(nodes_.valid(node_id));
    return nodes_.at(node_id);    
}

node_info_ptr view::find_node(string const& name) const
{
    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        if ((*it)->data().name == name)
            return (*it);

    return node_info_ptr();
}

void view::set_model( string const& model, bool save_root_pos )
{
    node_position pos = get_node(0)->position();

    set(msg::model_msg(model));
    if (save_root_pos)
        node_control_ptr(get_node(0))->set_position(pos);
}

node_tree_iterator_ptr view::get_node_tree_iterator(uint32_t node_id) const
{
    return boost::make_shared<node_tree_iterator_impl>(get_node(node_id), this);
}

string const& view::get_model() const
{
    return settings_.model;
}

void view::visit_nodes( boost::function<void(node_info_ptr)> const& f ) const
{
    for_each(nodes_.begin(), nodes_.end(), f);
}

void view::pre_update(double time)
{
    base_view_presentation::pre_update(time);

    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        (*it)->pre_update(time);
}

void view::post_update(double time)
{          
    base_view_presentation::post_update(time);

    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        (*it)->post_update(time);
}

void view::on_object_created(object_info_ptr object)
{
    base_view_presentation::on_object_created(object);
    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        (*it)->on_object_created(object);
}

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object);
    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        (*it)->on_object_destroying(object);
}

void view::create_node_hierarchy(geo_position const& pos, binary::input_stream& model_stream)
{
    using namespace binary;

    binary::size_type count;
    read(model_stream, count);
    for (size_t i = 0; i < count; ++i)
    {
        model_structure::node_data data;
        read(model_stream, data);
        uint32_t id = nodes_.next_id();

        auto nod = create_node(this, pos, data, id);
        nodes_.insert(nod->node_id(), nod);

        create_node_hierarchy(*nod, model_stream);
    }
}

void view::create_node_hierarchy(node_impl const& parent, binary::input_stream& model_stream)
{
    using namespace binary;

    binary::size_type count;
    read(model_stream, count);
    for (size_t i = 0; i < count; ++i)
    {
        model_structure::node_data data;
        read(model_stream, data);
        uint32_t id = nodes_.next_id();

        auto nod = create_node(this, parent, data, id);
        nodes_.insert(nod->node_id(), nod);

        create_node_hierarchy(*nod, model_stream);
    }
}

void view::init_collision_volume()
{
    collision_structure_.reset();

    vector<char> data;

    if (!settings_.model.empty())
    {
        string filename = cfg().path.data + "//models//" + settings_.model + "//" + settings_.model + ".collision";
        scoped_ptr<ifstream> file(new ifstream(filename, ifstream::binary));

        if (!file->good())
        {
            filename = cfg().path.data + "//areas//" + settings_.model + "//" + settings_.model + ".collision";
            file.reset(new ifstream(filename, ifstream::binary));
        }

        if (file->good())
        {
            data.resize(boost::filesystem::file_size(filename));
            file->read(data.data(), data.size());
        }
    }

    if (!data.empty())
    {
        binary::input_stream colvol_stream(data.data(), data.size());
        collision_structure_ = make_shared<model_structure::collision_structure>();
        read(colvol_stream, *collision_structure_);
    }
}

void view::init_logic_tree()
{
    nodes_.clear();
                  
    vector<char> model_data;

    if (!settings_.model.empty())
    {
        string filename = cfg().path.data + "//models//" + settings_.model + "//" + settings_.model + ".stbin";
        scoped_ptr<ifstream> file(new ifstream(filename, ifstream::binary));

        if (!file->good())
        {
            filename = cfg().path.data + "//areas//" + settings_.model + "//" + settings_.model + ".stbin";
            file.reset(new ifstream(filename, ifstream::binary));
        }

        if (file->good())
        {
            model_data.resize(boost::filesystem::file_size(filename));
            file->read(model_data.data(), model_data.size());
        }
    }

    if (model_data.empty())
    {
        using namespace binary;

        // fill default
        output_stream stream;
        write(stream, 1);
        model_structure::node_data ndata;
        ndata.name = "root";
        write(stream, ndata);
        write(stream, 0);

        model_data = stream.detach();
    }

    binary::input_stream model_stream(model_data.data(), model_data.size());

    create_node_hierarchy(geo_position(), model_stream);
}

uint32_t view::msg_sub_id(size_t id, binary::bytes_cref data) const
{
    if (id != msg::nm_node_data)
        return 0u;

    using namespace binary;
    input_stream is(data);

    Verify(id == network::read_id(is));

    msg::node_data nd;
    network::read_msg(is, nd);

    uint32_t node_id = nd.node_id;

    is = input_stream(nd.data);
    uint32_t msgid = network::read_id(is);

    return msgid + (node_id << 16);
}

AUTO_REG_NAME(nodes_manager_view , view ::create);
//AUTO_REG_NAME(nodes_manager_chart, chart::create);

} // nodes_management

