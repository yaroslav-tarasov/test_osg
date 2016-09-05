#include "stdafx.h"

#include "precompiled_objects.h"

#include "kernel/objects/base_view.h"

namespace kernel
{

base_view_presentation::base_view_presentation(kernel::object_create_t const& oc)
    : /*py_reg_       (oc.sys->kind() == kernel::sys_model )
    , */
      object_id_    (oc.object_id)
    , name_         (oc.name)
    , objects_      (oc.objects )
    , sys_          (oc.sys)
    , collection_   (dynamic_cast<kernel::object_collection *>(oc.sys))
    , send_msg_     (oc.send_msg)
    , block_msgs_   (oc.block_msgs)
    , hierarchy_class_(oc.hierarchy_class)
    , object_data_  (oc.object_data)
{
    typedef base_view_presentation this_t;

    if(oc.sys) 
    conn_holder() 
        <<  collection_->subscribe_object_created   (boost::bind(&this_t::object_created   , this, _1))
        <<  collection_->subscribe_object_destroying(boost::bind(&this_t::object_destroying, this, _1));

    conn_holder() 
        <<  subscribe_child_appended(boost::bind(&this_t::child_appended, this, _1))
        <<  subscribe_child_removing(boost::bind(&this_t::child_removing, this, _1))
        <<  subscribe_parent_changed(boost::bind(&this_t::parent_changed, this)    );
}

string const& base_view_presentation::name() const
{
    return name_;
}

void base_view_presentation::set_name(std::string const& name)
{
    name_ = name;
    name_changed_signal_();
}

FIXME (Ну это просто офигенно храним в size_t возвращаем uint32_t )
uint32_t base_view_presentation::object_id() const
{
    return object_id_;
}

kernel::object_info_wptr base_view_presentation::parent() const
{
    return parent_;
}

kernel::object_info_vector const& base_view_presentation::objects  () const
{
    return objects_;
}

kernel::object_class_ptr base_view_presentation::hierarchy_class() const
{
    return hierarchy_class_;
}

void base_view_presentation::save(dict_ref dict, bool key_safe) const
{
    if (auto ptr = dynamic_cast<obj_data_saver const*>(this))
        ptr->save_obj_data(dict, key_safe);
}

const object_data_t&  base_view_presentation::get_data      () const 
{
    return object_data_;
}

connection_holder& base_view_presentation::conn_holder()
{
    return conn_holder_;
}

void base_view_presentation::send_msg(binary::bytes_cref bytes, bool sure, bool just_cmd)
{
    send_msg_(bytes, sure, just_cmd);
}

void base_view_presentation::pre_update(double time)
{
    for (auto it = objects_.begin(); it != objects_.end(); ++it)
        kernel::base_presentation_ptr(*it)->pre_update(time);
}

void base_view_presentation::update( double time ) 
{
    for (auto it = objects_.begin(); it != objects_.end(); ++it)
        kernel::base_presentation_ptr(*it)->update(time);
}

void base_view_presentation::post_update( double time ) 
{
    for (auto it = objects_.begin(); it != objects_.end(); ++it)
        kernel::base_presentation_ptr(*it)->post_update(time);
}

void base_view_presentation::update_atc (double time)
{
    for (auto it = objects_.begin(); it != objects_.end(); ++it)
        kernel::base_presentation_ptr(*it)->update_atc(time);
}

void base_view_presentation::on_msg(binary::bytes_cref bytes) 
{
    msg_disp().dispatch_bytes(bytes);
}

void base_view_presentation::reset_parent(kernel::object_info_wptr parent)
{
    parent_ = parent;
    parent_changed_signal_();
}

network::msg_dispatcher<>::ids_t base_view_presentation::msg_ids() const
{
    return msg_disp_.msg_ids();
}

uint32_t base_view_presentation::msg_sub_id(size_t /*id*/, binary::bytes_cref /*data*/) const
{
    return 0u;
}

void base_view_presentation::child_appended(kernel::object_info_ptr child) 
{ 
    on_child_appended(child); 
}

void base_view_presentation::child_removing(kernel::object_info_ptr child) 
{ 
    on_child_removing(child); 
}

void base_view_presentation::parent_changed()                      
{ 
    on_parent_changed(); 
}

void base_view_presentation::object_created(kernel::object_info_ptr object) 
{ 
    on_object_created(object); 
}

void base_view_presentation::object_destroying(kernel::object_info_ptr object) 
{ 
    on_object_destroying(object); 
}


FIXME("И таки да, это никогда не вызывается")
void base_view_presentation::append_child( kernel::object_info_ptr object )
{
    objects_.push_back(object);
    child_appended_signal_(object);
}

FIXME("И таки да, это никогда не вызывается")
void base_view_presentation::remove_child( kernel::object_info_ptr object )
{
    child_removing_signal_(object);

    for (auto it = objects_.begin(); it != objects_.end(); ++it)
        if (*it == object)
        {
            objects_.erase(it);
            break;
        }
}

network::msg_dispatcher<>& base_view_presentation::msg_disp()
{
    return msg_disp_;
}

}