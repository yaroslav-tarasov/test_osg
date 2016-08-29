#include "precompiled_objects.h"
#include "labels_manager_vis.h"

#include "common/aircraft.h"

#include "application/main_window.h"
#include "application/menu.h"

namespace labels_manager
{

// fabrics
object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}
AUTO_REG_NAME(labels_manager_visual, visual::create);


// ctor
visual::visual( kernel::object_create_t const& oc, dict_copt dict )
    : view(oc, dict)
    , vis_sys_(dynamic_cast<visual_system *>(oc.sys))
    , update_timer_(boost::bind(&visual::update_impl, this))
    , rolled_id_(0)
{

    // start timer
    // update_timer_.wait(pt::milliseconds(in_atc_ ? 1000 : 250), true);
    vis_sys_->scene()->subscribe_gui_ready(boost::bind(&visual::on_gui_ready,this));
}

// dtor
visual::~visual()
{
    for (auto it = labels_.begin(), it_end = labels_.end(); it != it_end; ++it)
        (*it)->set_labels_manager(nullptr);
}


// base_presentation
void visual::update( double /*time*/ )
{
    update_impl();
}

// base_view
void visual::on_object_created(object_info_ptr object)
{
    view::on_object_created(object) ;
    add_label(object) ;
}

void visual::on_object_destroying(object_info_ptr object)
{
    view::on_object_destroying(object) ;
    remove_label(object) ;
}


// labels_manager
void visual::insert( labels_provider_ptr p )
{
    if (p)
    {
        p->set_labels_manager(this);
        labels_.insert(p);
    }
}

void visual::erase( labels_provider_ptr p )
{
    if (p)
        labels_.erase(p);
}

// own
void visual::update_impl()
{
    //chart_sys_->doc()->chart()->visit_panels(boost::bind(&visual::manage_forms, this, _1, _2));
}




void visual::add_label(object_info_ptr object)
{
    if (aircraft::info_ptr airc = object)
    {
#if 0
        labels_provider_ptr fp = find_first_child<labels_provider_ptr>(object.get());
        Assert(fp);
#endif
		labels_management::label_provider_getter* getter = dynamic_cast<labels_management::label_provider_getter*>(object.get());
		if(getter)
		{
			labels_provider_ptr fp = getter->get_label_provider();
			Assert(fp);
			
            (fp)->set_visible(false);

			insert(fp);
			air_managed_providers_.insert(std::make_pair(object->object_id(), fp));
		}
    }
}

void visual::remove_label(object_info_ptr object)
{
    if (aircraft::info_ptr airc = object)
    {
        auto it = air_managed_providers_.find(object->object_id());

        if (air_managed_providers_.end() != it)
        {
            erase(it->second);
            air_managed_providers_.erase(it);
        }
    }
}

void visual::on_new_settings()
{
    // set new font if needed
    if (new_settings_.font != settings_.font)
    {
        for (auto it = labels_.begin(), it_end = labels_.end(); it != it_end; ++it)
        {
            (*it)->set_font(new_settings_.font.name, new_settings_.font.size);
            // (*it)->force_update();
        }
    }

    // save them
    settings_ = new_settings_;
}

void visual::on_gui_ready()
{
    app::main_window_ptr  mw  = vis_sys_->scene()->GetMainGUIWindow();

    app::menu_ptr vm = mw->get_main_menu(L"Вид");
    if(!vm)
        vm = mw->add_main_menu(L"Вид");

    vm->add_string(L"Метки", boost::bind(&visual::on_check_callsign,this));
}

void visual::on_check_callsign ()
{
    for (auto it = labels_.begin(), it_end = labels_.end(); it != it_end; ++it)
    {
        (*it)->set_visible(!(*it)->get_visible());
    }
}

}
