#pragma once

#include "../aircraft_common.h"
#include "../aircraft_shassis_impl.h"
#include "../../common/airports_manager.h"
#include "../../common/phys_sys.h"
#include "../../common/aircraft_fms.h"

namespace aircraft
{
    namespace sync_fsm
    {
        //! состояние снихронизации... это переход между физической моделью (столкновений) и проводкой
        FIXME("Завести завели, а не используем чего так?")
        enum state
        {
            sync_none,
            sync_with_phys,
            sync_with_fms,
            sync_transition_phys_fms,
            sync_transition_fms_phys,
        };

        //! состояние???
        struct state_t
        {
            virtual ~state_t() {}
            virtual void update(double time, double dt) = 0;
            virtual void on_zone_destroyed( size_t /*id*/ ) {}
            virtual void on_fast_session( bool /*fast*/ ) {}
            virtual void deinit() {}
        };

        typedef polymorph_ptr<state_t> state_ptr;

        //! интерфейс самолета???
        struct self_t
        {
            virtual ~self_t() {}

            virtual geo_position fms_pos() const = 0;
            virtual airports_manager::info_ptr get_airports_manager() const = 0;
            virtual phys::control_ptr phys_control() const = 0;
            virtual nodes_management::manager_ptr get_nodes_manager() const = 0;
            virtual aircraft_fms::info_ptr get_fms_info() const = 0;
            //virtual meteo::meteo_cursor_ptr get_meteo_cursor() const = 0;
            virtual bool tow_attached() const = 0;
            virtual shassis_support_ptr get_shassis() const = 0;
			virtual rotors_support_ptr get_rotors() const = 0;
            virtual geo_position get_root_pos() const = 0;
            virtual bool is_fast_session() const = 0;

            virtual void set_desired_nm_pos  (geo_point_3 const& pos) = 0;
            virtual void set_desired_nm_orien(quaternion const& orien) = 0;
            virtual void switch_sync_state(state_ptr state) = 0;
            virtual void freeze_position() = 0;
            virtual void set_phys_aircraft(phys_aircraft_ptr phys_aircraft) = 0;
            virtual void set_nm_angular_smooth(double val) = 0;
        };

        inline static double phys_height() { return 80.0; }

    }
}