#pragma region precompile
#include "kernel/object_class.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems/impl/messages.h"
#pragma  endregion
#include "model_system/model_system_impl.h"
#include "visual_system/visual_system_impl.h"
#include "kernel/systems/systems_base.h"
#include "kernel/msg_proxy.h"
#include "kernel/systems/mod_system.h"
#include "ctrl_system/ctrl_system_impl.h"

#if 0
#include "nfi/nfi.h"
#endif

namespace kernel
{

struct systems_factory_impl
    : systems_factory 
{
	system_ptr create_model_system(msg_service& service, std::string const& script) 
	{
		LogInfo("Creating MODEL system");
		return kernel::system_ptr(boost::make_shared<model_system_impl>(boost::ref(service), boost::ref(script)));
	}

	system_ptr create_visual_system(msg_service& service, av::IVisualPtr vis, vis_sys_props const& vsp ) 
	{
		LogInfo("Creating VISUAL system");
		return kernel::system_ptr(boost::make_shared<visual_system_impl>(boost::ref(service),  vis, boost::ref(vsp)));
	}

	system_ptr create_ctrl_system( msg_service& service ) 
	{
		LogInfo("Creating CTRL system");
		return kernel::system_ptr(boost::make_shared<ctrl_system_impl>(boost::ref(service)));
	}

};

systems_factory_ptr create_system_factory()
{   
    return make_shared<systems_factory_impl>();
}

AUTO_REG(create_system_factory);

} // kernel
