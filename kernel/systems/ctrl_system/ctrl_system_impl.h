#pragma once 
#include "kernel/systems/ctrl_system.h"
 #include "kernel/systems/impl/system_base.h"

namespace kernel
{

	struct ctrl_system_impl
		: ctrl_system
		, system_base

	{

		ctrl_system_impl(msg_service& service);

	private:
		void update       (double time) override;

	private:
		DECL_LOGGER("control_system");

	private:
		void            object_destroying(object_info_ptr object);

	private:
		scoped_connection object_destroying_connection_;



	};

}