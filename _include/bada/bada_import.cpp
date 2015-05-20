#include "stdafx.h"

#include "bada/bada_fwd.h"
#include "ada/ada.h"
#include "bada_import.h"
#include "bada/bada.h"
#include "fms/fms.h"

#include "nfi/lib_loader.h"

//#ifdef _DEBUG
//    #pragma comment(lib, "bada.lib")
//#else 
//    #pragma comment(lib, "bada.lib")
//#endif

#ifdef _DEBUG
#define BADA_DLL "badad"
#else 
#define BADA_DLL "bada"
#endif

typedef bada::proxy_ptr (*create_proxy_fn_t)(std::string const& bada_path);

namespace ada
{
    ada::data_t fill_data(std::string const& bada_path, std::string const& aircraft_kind )
    {   
        ada::data_t ac_settings;

        create_proxy_fn_t create_proxy_fn = reinterpret_cast<create_proxy_fn_t> (lib_loader().get_symbol(BADA_DLL,"create_proxy"));
		
		if (create_proxy_fn)
		{
			auto proxy = create_proxy_fn(bada_path);
			//auto proxy = bada::create_proxy(bada_path);

			auto syn_data        = proxy->get_synonim_data(aircraft_kind) ;
            
            if(!syn_data)
                syn_data = proxy->get_synonim_data("A319")  ;

			auto air_data        = proxy->get_aircraft_data(syn_data->getSynonim()) ;
			auto air_global_data = proxy->get_global_data() ;


			ac_settings.base_model        = syn_data->getSynonim();
			ac_settings.manufacturer      = syn_data->getManufacturer();
			ac_settings.model_name        = syn_data->getModelName();
			ac_settings.S                 = air_data->get_S();
			ac_settings.span              = air_data->get_span();
			ac_settings.length            = air_data->get_length();
			ac_settings.engine            = air_data->getEngineType();
			ac_settings.turbulence        = air_data->getWakeCategory();
			ac_settings.cd_0_cruise       = air_data->get_Cd0_cr();
			ac_settings.cd_2_cruise       = air_data->get_Cd2_cr();
			ac_settings.cd_0_approach     = air_data->get_Cd0_ap();
			ac_settings.cd_2_approach     = air_data->get_Cd2_ap();
			ac_settings.cd_0_landing      = air_data->get_Cd0_ld();
			ac_settings.cd_0_landing_gear = air_data->get_Cd0_ldg();
			ac_settings.cd_2_landing      = air_data->get_Cd2_ld();
			ac_settings.cf_1              = air_data->get_Cf_1();
			ac_settings.cf_2              = air_data->get_Cf_2() * cg::kt2mps();
			ac_settings.cf_3              = air_data->get_Cf_3() / 60.;
			ac_settings.cf_4              = air_data->get_Cf_4() * cg::feet2meter();
			ac_settings.cf_cruise         = air_data->get_Cfr();

			switch(ac_settings.engine)
			{
			case bada::jet:
				ac_settings.cf_1 /= (60. * 1000.);
				break;
			case bada::turboprop:
				ac_settings.cf_1 /= (60. * 1000. * cg::kt2mps());
				break;
			case bada::piston:
				ac_settings.cf_1 /= 60.;
				break;
			}

			ac_settings.nominal_ba_to_ld       = 15.; // TODO: make difference to military objects
			ac_settings.nominal_ba             = 35.;
			ac_settings.max_ba_to_ld           = 25.;
			ac_settings.max_ba_hold            = 35.;
			ac_settings.max_ba                 = 45.;
			ac_settings.max_operating_height     = air_data->get_hmo() * cg::feet2meter();
			ac_settings.max_height               = air_data->get_hmax() * cg::feet2meter();
			ac_settings.temperature_gradient     = air_data->get_Gt() * cg::feet2meter();
			ac_settings.mass_gradient            = air_data->get_Gw() * cg::feet2meter();
			ac_settings.max_mass                 = air_data->get_m_max() * 1000;
			ac_settings.min_mass                 = air_data->get_m_min() * 1000;
			ac_settings.max_payload_mass         = air_data->get_m_pyld() * 1000;
			ac_settings.ref_mass                 = air_data->get_m_ref() * 1000;

			if (ac_settings.engine == bada::jet ||ac_settings.engine == bada::piston)
				ac_settings.ct_1 = air_data->get_Ctc_1();
			else // turboprop
				ac_settings.ct_1 = air_data->get_Ctc_1() * cg::kt2mps();

			ac_settings.ct_2  = air_data->get_Ctc_2() * cg::feet2meter();

			if (ac_settings.engine == bada::jet)
				ac_settings.ct_3 = air_data->get_Ctc_3() / cg::sqr(cg::feet2meter());
			else if (ac_settings.engine == bada::turboprop)
				ac_settings.ct_3 = air_data->get_Ctc_3();
			else // piston
				ac_settings.ct_3 = air_data->get_Ctc_3() * cg::kt2mps();

			ac_settings.ct_4                     = air_data->get_Ctc_4();
			ac_settings.ct_5                     = air_data->get_Ctc_5();
			ac_settings.ct_descent_low           = air_data->get_Ct_des_low();
			ac_settings.descent_height_level     = air_data->get_Hp_des() * cg::feet2meter();
			ac_settings.ct_descent_high          = air_data->get_Ct_des_high();
			ac_settings.ct_descent_approach      = air_data->get_Ct_des_app();
			ac_settings.ct_descent_landing       = air_data->get_Ct_des_ld();
			ac_settings.V_1[fms::climb_stage]    = air_data->getDataCLS().getStandardCAS().first * cg::kt2mps();
			ac_settings.V_1[fms::cruise_stage]   = air_data->getDataCRS().getStandardCAS().first * cg::kt2mps();
			ac_settings.V_1[fms::descent_stage]  = air_data->getDataDS().getStandardCAS().first * cg::kt2mps();
			ac_settings.V_2[fms::climb_stage]    = air_data->getDataCLS().getStandardCAS().second * cg::kt2mps();
			ac_settings.V_2[fms::cruise_stage]   = air_data->getDataCRS().getStandardCAS().second * cg::kt2mps();
			ac_settings.V_2[fms::descent_stage]  = air_data->getDataDS().getStandardCAS().second * cg::kt2mps();
			ac_settings.M[fms::climb_stage]      = air_data->getDataCLS().getMach();
			ac_settings.M[fms::cruise_stage]     = air_data->getDataCRS().getMach();
			ac_settings.M[fms::descent_stage]    = air_data->getDataDS().getMach();

			// jet
			ac_settings.v_d_cl[0]       = 5 * cg::kt2mps();
			ac_settings.v_d_cl[1]       = 10 * cg::kt2mps();
			ac_settings.v_d_cl[2]       = 30 * cg::kt2mps();
			ac_settings.v_d_cl[3]       = 60 * cg::kt2mps();
			ac_settings.v_d_cl[4]       = 80 * cg::kt2mps();
			// turboprop/piston
			ac_settings.v_d_cl[5]       = 20 * cg::kt2mps();
			ac_settings.v_d_cl[6]       = 30 * cg::kt2mps();
			ac_settings.v_d_cl[7]       = 35 * cg::kt2mps();

			// jet/turboprop
			ac_settings.v_d_des[0]      = 5 * cg::kt2mps();
			ac_settings.v_d_des[1]      = 10 * cg::kt2mps();
			ac_settings.v_d_des[2]      = 20 * cg::kt2mps();
			ac_settings.v_d_des[3]      = 50 * cg::kt2mps();
			// piston
			ac_settings.v_d_des[4]      = 5 * cg::kt2mps();
			ac_settings.v_d_des[5]      = 10 * cg::kt2mps();
			ac_settings.v_d_des[6]      = 20 * cg::kt2mps();

			ac_settings.v_stall_to = air_data->get_Vstall_to() * cg::kt2mps();
			ac_settings.v_stall_ic = air_data->get_Vstall_ic() * cg::kt2mps();
			ac_settings.v_stall_cr = air_data->get_Vstall_cr() * cg::kt2mps();
			ac_settings.v_stall_ap = air_data->get_Vstall_ap() * cg::kt2mps();
			ac_settings.v_stall_ld = air_data->get_Vstall_ld() * cg::kt2mps();

			ac_settings.v_rw_backtrack = 35 * cg::kt2mps();
			ac_settings.v_taxi         = 15 * cg::kt2mps();
			ac_settings.v_apron        = 10 * cg::kt2mps();
			ac_settings.v_gate         = 5 * cg::kt2mps();

			ac_settings.c_v_min    = air_global_data->getCVmin(0).second;
			ac_settings.c_v_min_to = 1.2;
			ac_settings.h_descent  = air_data->get_Hp_des() * cg::feet2meter();

			ac_settings.h_max_to = 400 * cg::feet2meter();
			ac_settings.h_max_ic = 2000 * cg::feet2meter();
			ac_settings.h_max_ap = 8000 * cg::feet2meter();
			ac_settings.h_max_ld = 3000 * cg::feet2meter();

			ac_settings.takeoff_length = 0.75 * air_data->get_TOL();
			ac_settings.landing_length = air_data->get_LDL();
			ac_settings.max_cas = air_data->get_Vmo() * cg::kt2mps();
			ac_settings.max_mach = air_data->get_Mmo();
		}

        return ac_settings;
    }

} // namespace ada



