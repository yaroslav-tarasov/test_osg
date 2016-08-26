#pragma once

#include "common/labels_management.h"

#include "labels_manager_view.h"

#include "async_services/async_timer.h"
#include "common/randgen.h"


namespace labels_manager
{

using namespace labels_management;

struct visual
    : view
    , labels_manager
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    ~visual();

private:
    visual(kernel::object_create_t const& oc, dict_copt dict);

    // base_presentation
private:
    void update( double /*time*/ );

    // base_view
private:
    void on_object_created(object_info_ptr object) override;
    void on_object_destroying(object_info_ptr object) override;


    // labels_manager
private:
    virtual void insert( labels_provider_ptr p ) override;
    virtual void erase ( labels_provider_ptr p ) override;

    virtual labels_provider_list_t const & labels_list() const override { return labels_; }
    virtual labels_provider_list_t       & labels_list() override       { return labels_; }

    virtual void set_settings( settings_t const & ns ) override { new_settings_ = ns; on_new_settings(); }
    virtual const settings_t & get_settings() override          { return settings_; }

    // own
private:
    void update_impl();

    void add_label( object_info_ptr object ) ;
    void remove_label( object_info_ptr object ) ;
    
    // own
private:
    void on_new_settings();
    void on_gui_ready();

    // gui
private:
    void on_check_callsign ();

private:
    async_timer update_timer_;

private:
    labels_provider_list_t  labels_;
    unsigned                rolled_id_;
    simplerandgen           rndgen_;

private:
    typedef ph_map<uint32_t, labels_provider_ptr>::map_t labels_provider_list_t;
    labels_provider_list_t                               air_managed_providers_;

private:
    kernel::visual_system     * vis_sys_;

private:
    settings_t          settings_;
    settings_t          new_settings_;

};

}
