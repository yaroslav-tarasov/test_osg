#pragma once

namespace aircraft_physless
{

	// Дикая реализация state на какую-то замену fms
	namespace ums
	{

		struct state_t
		{
			state_t()
				: course(0)
				, fuel_mass  (1)
				, TAS   (0)
				//, cfg   (CFG_CR)
			{}

			state_t(cg::geo_point_3 const& pos, double course, double fuel_mass, double TAS/*, air_config_t cfg*/)
				: pos   (pos)
				, course(course)
				, fuel_mass (fuel_mass)
				, TAS   (TAS)
				//, cfg   (cfg)
			{}

			cg::geo_point_3 pos;
			double course;
			double fuel_mass;
			double TAS;
			//air_config_t cfg;
		};

		struct pilot_state_t
		{
			pilot_state_t(state_t const& dyn_state)
				: dyn_state(dyn_state)
			{}

			pilot_state_t()
			{}

			state_t                 dyn_state;
		};
	}

	struct state_t : ums::pilot_state_t
	{
		state_t();
		state_t(ums::pilot_state_t const& ps, double pitch, double roll, uint32_t version);
		cpr orien() const;

		double pitch;
		double roll;
	};

	inline state_t::state_t()
		: pitch(0)
		, roll(0)
	{}

	inline state_t::state_t(ums::pilot_state_t const& ps, double pitch, double roll, uint32_t version)
		: pilot_state_t(ps)
		, pitch(pitch)
		, roll(roll)
	{}

	inline cpr state_t::orien() const
	{
		return cpr(dyn_state.course, pitch, roll);
	}

}

