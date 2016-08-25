#pragma once

#include "common/font_descr.h"

namespace labels_management
{

    struct settings_t
    {
        bool auto_avoidance;
        font_descr font;

        settings_t()
            : auto_avoidance(false)
            , font("Courier", 13)
        {
        }
    };

    REFL_STRUCT(settings_t)
        REFL_ENTRY(auto_avoidance)
        REFL_ENTRY(font)
    REFL_END()



    struct labels_manager;
    struct label_provider
    {
        virtual ~label_provider() {}

        virtual labels_manager * get_labels_manager() const = 0;
        virtual void set_labels_manager( labels_manager * ) = 0;
        virtual void set_font( std::string const & , unsigned ) = 0;
        virtual void set_visible (bool) = 0;
        virtual bool get_visible () = 0;
    };

    typedef polymorph_ptr<label_provider> labels_provider_ptr;
    typedef ph_set<labels_provider_ptr>::set_t labels_provider_list_t;
    
    struct label_provider_getter
    {
        virtual labels_provider_ptr get_label_provider() const = 0;
    };

    struct labels_manager
    {
        virtual ~labels_manager() {}

        virtual void insert( labels_provider_ptr ) = 0;
        virtual void erase ( labels_provider_ptr ) = 0;

        virtual labels_provider_list_t const & labels_list() const = 0;
        virtual labels_provider_list_t       & labels_list() = 0;

        virtual void set_settings( settings_t const & ) = 0;
        virtual const settings_t & get_settings() = 0;
    };

    typedef polymorph_ptr<labels_manager> labels_manager_ptr;

}
