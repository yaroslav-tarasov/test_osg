#pragma once

#include "net_layer/net_configurator_data.h"

namespace net_layer
{

    struct configurator
    {
        virtual ~configurator(){}

        //virtual applications_t const& apps        () = 0;

        //virtual size_t app_id_by_name(std::string const& app_name) const = 0;

        //virtual bool save_config(std::string const& filename, configuration_t const& cfg) = 0;
        virtual void load_config(std::string const& filename, configuration_t& cfg)       = 0;

        virtual hosts_t const&  hosts()         const =0;

        virtual configuration_t const&  configuration() const =0;
        virtual applications_t  const&  applications () const =0;
    };

    typedef boost::shared_ptr<configurator> configurator_ptr;


    configurator_ptr create_configurator(boost::optional<unsigned> key);

}