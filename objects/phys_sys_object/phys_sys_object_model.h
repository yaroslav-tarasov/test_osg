#pragma once

#include "phys_sys_object_view.h"
#include "common/phys_sys.h"
//#include "common/model_debug_drawer.h"
#include "common/airport.h"


namespace phys
{

    struct model
        : model_presentation
        , view
        , control
    {
        static object_info_ptr create(kernel::object_create_t const& oc/*, dict_copt dict*/);

        ~model();

    private:
        model(kernel::object_create_t const& oc/*, dict_copt dict*/);

        // base_presentation
    private:
        void update( double /*time*/ );

        void on_object_created(object_info_ptr object) override;
        void on_object_destroying(object_info_ptr object) override;

        // object::control
    private:
        system_ptr get_system(size_t zone);
        optional<size_t> get_zone(geo_point_3 const & pos) const;
        optional<size_t> get_zone(std::string const& airport) const;
        geo_base_3 const& get_base(size_t zone) const;
        string zone_name(size_t id) const;

    private:
        void on_exercise_loaded();


    private:
        model_system * sys_;
        double last_time_;
        scoped_connection exercise_loaded_connection_;


        struct zone_t
        {
            zone_t() {}

            zone_t(phys::system_ptr sys, geo_point_3 const& base, std::string const& airport)
                :sys(sys)
                , base(base)
                , airport(airport)
            {}

            phys::system_ptr sys;
            geo_base_3 base;
            std::string airport;
        };

        struct airport_zone_predicate
        {   
            airport_zone_predicate(std::string const& name)
                : name_(name)
            {}

            bool operator()(zone_t const& z)
            {
                return z.airport == name_;
            }

        private:
            string name_;
        };

        fixed_id_vector<zone_t> zones_;
        FIXME(Debug drawer)
        //mdd::info_ptr mod_debug_;
    };

}