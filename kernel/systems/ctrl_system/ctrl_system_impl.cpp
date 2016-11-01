#include "ctrl_system_impl.h"


namespace kernel
{


ctrl_system_impl::ctrl_system_impl(msg_service& service)
	: system_base(sys_ext_ctrl, service, "objects.xml")
	, object_destroying_connection_(this->subscribe_object_destroying(boost::bind(&ctrl_system_impl::object_destroying, this, _1)))
{
	LogInfo("Create Control Subsystem");
}

void ctrl_system_impl::update(double time)
{
	system_base::update(time);
}

void ctrl_system_impl::object_destroying(object_info_ptr object)
{
}


}