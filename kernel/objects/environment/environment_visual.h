#pragma once
#include "environment_view.h"
#include "kernel/kernel_fwd.h"
#include "objects/ani.h"

namespace environment
{

    struct visual
        : kernel::visual_presentation
        , view
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        visual(kernel::object_create_t const& oc, dict_copt dict);

        // base_view_presentation
    protected:
        void on_object_destroying(kernel::object_info_ptr obj) override;

        // base_presentation
    private:
        void update( double time );

    private:
        void on_parent_changed() override;

    private:
        void on_settings_changed  ();
        void on_start_time_changed();

    private:
        kernel::visual_system* sys_;

    private:
        optional<double> last_update_time_ ;
        double current_time_ ;

    private:
        bool is_visible_ ;

    private:
        ani_object::info_ptr   ani_;
        ani::airport_info_ptr  airport_;
    };

} // nodes_management
