#include "kernel/objects/chart_base.h"


namespace kernel
{


chart_presentation_base::chart_presentation_base(kernel::object_create_t const& oc, base_view_presentation * view)
    : objects_factory_  (dynamic_cast<kernel::objects_factory *>(oc.sys))
    , chart_sys_        (dynamic_cast<kernel::chart_system *>(oc.sys))
    , view_             (view)
    , atc_chart_        (dynamic_cast<app::atc_document*>(chart_sys_->doc()))

    , cur_time_factor_  (0) 

    , is_auto_          (oc.hierarchy_class->check_attribute("auto", "true"))
    , hide_in_list_     (oc.hierarchy_class->check_attribute("hide", "true"))
{
    typedef chart_presentation_base this_t;

    view->conn_holder() 
        <<  view->subscribe_parent_changed(boost::bind(&this_t::on_hierarchy_changed, this))
        <<  view->subscribe_child_appended(boost::bind(&this_t::on_child_appended, this, _1))
        <<  view->subscribe_child_removing(boost::bind(&this_t::on_child_removing, this, _1))
        <<  view->subscribe_state_modified(boost::bind(&this_t::on_modified, this))
        <<  view->subscribe_name_changed(boost::bind(&this_t::on_name_changed, this));

    view->conn_holder() 
        << dynamic_cast<system_session*>(oc.sys)->subscribe_time_factor_changed(boost::bind(&this_t::time_factor_changed, this, _1, _2));
}

void chart_presentation_base::fill_attr(app::attr_panel_ptr attrs)
{
    app::attr_item * itm = attrs->add_item("name");
    edit_name_ = view_->name();
    itm->add(&edit_name_, "name", true);
    attr_changed_conn_ = attrs->subscribe_changed(boost::bind(&chart_presentation_base::name_edited, this));
}

std::string chart_presentation_base::panel_class_name() const
{
    return is_auto_ ? "_SYSTEM" : view_->hierarchy_class()->name() ;
}

std::string chart_presentation_base::panel_client_name() const
{
    return view_->name() ;
}
bool chart_presentation_base::is_atc_arm() const
{             
    return atc_chart_;
}

bool chart_presentation_base::no_periodic_update() const
{
    return !(chart_sys_->doc()->in_session()) || cg::eq_zero(cur_time_factor_);
}

unsigned int chart_presentation_base::id() const
{
    return view_->object_id() ;
}

app::panel_client* chart_presentation_base::client_parent() const
{
    return dynamic_cast<app::panel_client*>(view_->parent().lock().get()) ;
}

std::vector<app::panel_client*> chart_presentation_base::client_children() const
{
    kernel::object_info_vector const& children = view_->objects() ;

    std::vector<app::panel_client*> out(!removing_child_ ? children.size() : children.size() - 1) ;
    auto it = out.begin() ;

    for (unsigned int i = 0, size = children.size(); i != size; ++i)
    {
        if (removing_child_ == children[i])
            continue ;

        *it++ = dynamic_cast<app::panel_client*>(children[i].get()) ;
    }

    return out ;
}

void chart_presentation_base::on_panel_name_changed()
{
    update_in_list();
}

void chart_presentation_base::on_parent_changed()
{
    on_hierarchy_changed() ;
}

void chart_presentation_base::on_child_appended(kernel::object_info_ptr /*child*/)
{
    on_hierarchy_changed() ;
}

void chart_presentation_base::on_child_removing(kernel::object_info_ptr child)
{
    removing_child_ = child ;
    on_hierarchy_changed() ;
    removing_child_.reset() ;
}

void chart_presentation_base::on_modified()
{
    if (app::editor_document *ed_doc = dynamic_cast<app::editor_document*>(chart_sys_->doc()))
        ed_doc->set_modified(true) ;
}

void chart_presentation_base::on_hierarchy_changed()
{
    if (hide_in_list_)
        return ;

    if (!view_->parent().lock() && view_->hierarchy_class()->parent()->name() == "root")
    {
        list_reg_ = boost::none ;
        list_reg_ = boost::in_place(chart_sys_->doc()->list(), this) ;
    }
}

void chart_presentation_base::on_name_changed()
{
    edit_name_ = view_->name();
    // TODO: update attributes

    on_panel_name_changed() ;
}

void chart_presentation_base::name_edited()
{
    if (edit_name_ != view_->name())
        view_->set_name(edit_name_);
}

void chart_presentation_base::update_in_list()
{
    // TODO: Moroz! correct selection change in list
    if (list_reg_ && !hide_in_list_)
    {
        list_reg_ = boost::none ;
        list_reg_ = boost::in_place(chart_sys_->doc()->list(), this) ;
    }
}


void chart_presentation_base::time_factor_changed(double /*time*/, double factor)
{
    cur_time_factor_ = factor;
}

}