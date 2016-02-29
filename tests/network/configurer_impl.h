#pragma once

#include "configurer.h"

namespace net_layer
{

    struct configurator_impl
        : configurator
    {
        configurator_impl(optional<unsigned> key);
        ~configurator_impl();

        hosts_t const&          hosts()         const;
        configuration_t const&  configuration() const;
        applications_t  const&  applications () const;

        //size_t app_id_by_name(std::string const& app_name) const;

        bool save_config(std::string const& filename, configuration_t const& cfg);
        void load_config(std::string const& filename, configuration_t& cfg);

    private:
        void load_config();
        void save_config() const;

    private:
        DECL_LOGGER("configuration_impl");

    private:
        applications_t              apps_;
        hosts_t                    hosts_;
        configuration_t              cfg_;
    };

    configurator_ptr create_configurator(boost::optional<unsigned> key);

} // net_layer