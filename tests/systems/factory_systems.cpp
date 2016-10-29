#include "stdafx.h"

#include "kernel/systems/systems_base.h"
#if 0
#include "kernel/systems/fake_system.h"
#endif
#include "kernel/object_class.h"

#include "kernel/systems.h"
#include "kernel/msg_proxy.h"

#include "factory_systems.h"
#include "systems_impl.h"
#include "systems_impl2.h"

systems_ptr get_systems(systems::type  type, systems::remote_send_f rs)
{
    static boost::recursive_mutex guard;
    boost::lock_guard<boost::recursive_mutex> lock(guard);

    static systems_ptr cc = (type == systems::FIRST_IMPL)? boost::dynamic_pointer_cast<systems>(boost::make_shared<first::impl>(rs))
                                                         : boost::dynamic_pointer_cast<systems>(boost::make_shared<second::impl>(rs));
    return cc->get_this();
}


