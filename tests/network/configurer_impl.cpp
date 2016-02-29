#include "stdafx.h"
#include "configurer_impl.h"
#include "reflection/proc/prop_tree.h"
#include "config/config.h"

namespace 
{

    const char* cfg_file_name_hosts = "net_config_hosts.info";
    const char* cfg_file_name_apps  = "net_config_apps.info";


} // anonymous


namespace net_layer
{

    configurator_impl::configurator_impl(optional<unsigned> key)
    {
        load_config();
    }

    configurator_impl::~configurator_impl()
    {
        save_config();
    }

    void configurator_impl::load_config(std::string const& filename, configuration_t& cfg)
    {
        using namespace boost::property_tree;

        ptree pt;
        info_parser::read_info(filename, pt);

        prop_tree::read(pt, cfg);
    }

    bool configurator_impl::save_config(std::string const& filename, configuration_t const& cfg)
    {
        using namespace boost::property_tree;

        ptree pt;
        prop_tree::write(pt, cfg);

        try
        {
            info_parser::write_info(filename, pt);
        }
        catch(std::exception const&)
        {
            return false;
        }

        return true;
    }

    void configurator_impl::load_config()
    {
        using namespace boost::property_tree;

        // open file 
        if (fs::is_regular_file(cfg_file_name_hosts))
        {
            ptree pt;
            info_parser::read_info(cfg_file_name_hosts, pt);

            prop_tree::read(pt, hosts_, "hosts", true);
         }


        if (fs::is_regular_file(cfg_file_name_apps))
        {
            ptree pt;
            info_parser::read_info(cfg_file_name_apps, pt);

            prop_tree::read(pt, apps_);
        }
    }

    void configurator_impl::save_config() const
    {
        using namespace boost::property_tree;

        ptree pt;
        // info_parser::write_info(cfg_file_name_hosts, pt);
    }
    
    hosts_t const& configurator_impl::hosts() const
    {
        return hosts_;
    }

    applications_t const& configurator_impl::applications() const
    {
        return apps_;
    }

    configuration_t const& configurator_impl::configuration() const
    {
        return cfg_;
    }

    configurator_ptr create_configurator(boost::optional<unsigned> key)
    {
        return boost::make_shared<configurator_impl>(key);
    }


}