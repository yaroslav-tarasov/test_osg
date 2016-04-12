#include "stdafx.h"
#include "precompiled_objects.h"
#include "camera_view.h"

namespace camera_object
{

object_info_ptr view::create(object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

geo_point_3 view::pos() const
{
    return root()->position().global().pos;    
}

cpr view::orien() const
{
    return root()->position().global().orien.cpr();
}

view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)

    //, py_reg_(oc.sys->kind() == sys_model)
    , mng_   (find_first_child<nodes_management::manager_ptr>(this))
{
    if (mng_ /*&& !dict*/)  // FIXME Полюбому нет объекта
    {
        mng_->set_model("");
    }

    msg_disp()
        .add<msg::binoculars_msg>(boost::bind(&view::on_binoculars, this, _1));
}

void view::on_child_removing(kernel::object_info_ptr child)
{
    base_view_presentation::on_child_removing(child) ;

    if (mng_ == child)
        mng_.reset() ;
}

nodes_management::manager_ptr view::mng() const
{
    return mng_;
}

nodes_management::node_control_ptr view::root() const
{
    return mng_->get_node(0);
}

binoculars_t const& view::binoculars() const
{
    return obj_data().bins;
}

//! обработчик сообщения бинокля
void view::on_binoculars(msg::binoculars_msg const& msg)
{
    obj_data().bins = msg.binoculars;
    on_new_binoculars();
}

void view::binocular_on(unsigned target_id, double zoom)
{
    if (kernel::object_info_ptr info = collection_->get_object(target_id))
    {
        binoculars_t new_binoc = obj_data().bins;

        new_binoc.target = info->name();
        new_binoc.zoom   = zoom ;
        new_binoc.active = true ;

        if (new_binoc != obj_data().bins)
            set(msg::binoculars_msg(new_binoc)) ;
    }
}

void view::binocular_off()
{
    binoculars_t new_binoc = obj_data().bins;
    new_binoc.active = false;

    if (new_binoc != obj_data().bins)
        set(msg::binoculars_msg(new_binoc)) ;
}


} // camera_object

AUTO_REG_NAME(camera_view, camera_object::view::create);