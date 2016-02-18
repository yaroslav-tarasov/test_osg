#include "stdafx.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

#include "kernel/systems.h"
#include "kernel/msg_proxy.h"

#include "factory_systems.h"
#include "systems_impl.h"
#include "systems_impl2.h"

systems_ptr get_systems(systems::type  type)
{
    static boost::recursive_mutex guard;
    boost::lock_guard<boost::recursive_mutex> lock(guard);

    static systems_ptr cc = (type == systems::FIRST_IMPL)? boost::dynamic_pointer_cast<systems>(make_shared<first::impl>())
                                                         : boost::dynamic_pointer_cast<systems>(make_shared<second::impl>());
    return cc->get_this();
}


