#include "model_system_impl.h"
#if 0
#include "python/py_primitives.h"
#include "python/utils.h"
#endif

namespace kernel
{


	FIXME(Имя файла объектов в конструкторе это очень на круто)
     model_system_impl::model_system_impl(msg_service& service, string const& script)
		: system_base( sys_model, service, "objects.xml" )
		, time_factor_( 0)
		, calc_step_  (cfg().model_params.msys_step)
		//, py_engine_  (this)
		, ex_script_  (script)

		, skip_post_update_(true)
	{
		LogInfo("Create Model Subsystem");
		// subscribe_session_stopped(boost::bind(&model_system_impl::on_ses_stopped, this));
	}

	double model_system_impl::calc_step() const
	{
		return calc_step_ * (cg::eq_zero(time_factor_) ? 1. : time_factor_);
	}

}

