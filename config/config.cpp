#include "stdafx.h"
#include "config.h"
#include "reflection/proc/prop_tree.h"

using boost::property_tree::ptree;
using namespace boost::property_tree;

void save_cfg(cfg_t const& config);

struct cfg_holder
{
    cfg_holder()
    {
        if (fs::is_regular_file("config.info"))
        {
            ptree pt;
            info_parser::read_info("config.info", pt);

            prop_tree::read(pt, cfg_, false);
        }
        else
        {
            LogWarn("No config file found, using default") ;
            save_cfg(cfg_);
        }
    }

    cfg_t const& cfg() const
    {
        return cfg_;
    }

private:
    cfg_t cfg_;
};


cfg_t const& cfg()
{
    static cfg_holder holder;
    return holder.cfg();
}

void save_cfg(cfg_t const& config)
{
    ptree pt;
    prop_tree::write(pt, config);

    info_parser::write_info("config.info", pt);
}