#pragma once

#include "kernel/systems/mod_system.h"
#include "kernel/systems/impl/system_base.h"
#if 0
#include "python_engine.h"
#endif

namespace kernel
{

	struct model_system_impl
		: model_system
		, system_base
	{
		model_system_impl(msg_service& service, string const& script);

		// model_system
	public:
		double calc_step() const override;

	private:
		double time_factor_;
		double calc_step_;

	private:
		DECL_LOGGER("model_system");

	private:
		// py_engine py_engine_; 
		string    ex_script_;

	private:
		bool             skip_post_update_;
		optional<double> last_update_time_;
	};

} // kernel
