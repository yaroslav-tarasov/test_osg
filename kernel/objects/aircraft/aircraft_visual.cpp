#include "stdafx.h"
#include "precompiled_objects.h"
#include "aircraft_visual.h"

namespace aircraft
{

	object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new visual(oc, dict));
	}

	AUTO_REG_NAME(aircraft_visual, visual::create);

	visual::visual( kernel::object_create_t const& oc, dict_copt dict )
		: view(oc,dict)
	{
        visual_system* vsys = dynamic_cast<visual_system*>(sys_);
        
        nm::visit_sub_tree(get_nodes_manager()->get_node_tree_iterator(root()->node_id()), [this](nm::node_info_ptr n)->bool
        {
            if (boost::starts_with(n->name(), "engine"))
            {
                this->engine_node_ = n;
                return false;
            }
            return true;
        });
	}

    void visual::update(double time)
    {
        view::update(time);
        update_len(time);

    }
}


