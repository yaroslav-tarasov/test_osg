#pragma once

#include "common/aircraft.h"
#include "common/aircraft_fms.h"
#include "common/meteo_proxy.h"
#include "objects/ada.h"
#include "objects/ani.h"
#include "objects/fpl.h"
#include "fms/fms_instruments.h"

namespace aircraft
{
namespace aircraft_fms
{

//! данные о ВС для проводки - сериализуемые данные из упражнения
struct craft_fms_data 
{
    craft_fms_data()
        : payload_(1)
    {
    }

protected:
    //! состояние ВС (координаты крен тангаж скорость и т.д.)
    state_t               state_;
    //! тип ВС
    std::string           aircraft_kind_;
    //! настройки разрешения автоматического перехода между фазами полета
    settings_t            settings_;
    //! масса полезного груза
    double                payload_;
    //! характеристики ВС
    optional<ada::data_t> aircraft_data_;

    REFL_INNER(craft_fms_data)
        REFL_ENTRY(state_)
        REFL_ENTRY(aircraft_kind_)
        REFL_ENTRY(settings_)
        REFL_ENTRY(payload_)
        REFL_ENTRY(aircraft_data_)
    REFL_END()
};

//! представление ВС в объектной системе
struct view
    : base_view_presentation            // базовый класс для любого вида
    , obj_data_holder<craft_fms_data>   // сериализуемые данные из упражнения (координаты скорость крен тип и прочее)
    , control                           // получение и установка свойств проводимого ВС, команды на выполнение маневров
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view( kernel::object_create_t const& oc, dict_copt dict );

// base_view_presentation
protected:
    void update     (double time) override;
    void on_object_created( object_info_ptr object ) override;
    void on_object_destroying( object_info_ptr object ) override;
    void on_parent_changed() override;

// info
protected:
    state_t const& get_state() const override;
    double get_mass() const override;
    double get_payload_mass() const override;
    fms::manual_controls_t const& get_controls()  const override;
    fms::instrument_ptr           get_instrument() const override;
    fms::instrument_ptr           get_next_instrument() const override;
    fms::plan_t const&            get_plan() const  override;

    double         length_fwd() const override;
    double         length_bwd() const override;

    optional<ada::data_t> const& fsettings() const override;

    fms::procedure_model_ptr procedure_model() const override;
    fms::operation_model_ptr operation_model() const override;

    point_3 ground_velocity()  const override;
    double max_fuel() const override;
    ani::airport_info_ptr closest_airport() const override;
    optional<double> get_desired_course() const override;
    optional<double> get_desired_height() const override;
    optional<double> get_desired_CAS() const override;
    optional<double> get_desired_ROCD() const override;

    // control
protected:
    void set_state          (state_t const& st) override;
    void set_controls       (fms::manual_controls_t const& ctrl) override;
    void set_plan           (fms::plan_t const& plan) override;
    void transition         (fms::transition_t t) override;
    void fuel_discharge     (double part) override;
    void set_aircraft_kind  (std::string const &kind) override;
    void set_payload        (double payload) override;
    void instrument_direction(double course) override;
    void instrument_turn     (double course, optional<bool> ccw, optional<double> roll) override;
    bool instrument_point    (ani::point_pos const& pnt) override;
    void instrument_lock     (string const& airport, ani::runway_id runway) override;
    void instrument_approach (string const& airport, ani::runway_id runway) override;
    void instrument_go_around(string const& airport, ani::runway_id runway) override;
    void instrument_star     (ani::sidstar_id id) override;
    void instrument_holding  (ani::point_id id, fms::holding::config_t const &cfg) override;
    void instrument_cancel_next() override;
    void instrument_fpl_offset(double value, optional<geo_point_2> start, optional<geo_point_2> end) override;
    void instrument_point_fly_over(bool fly_over) override;
    void instrument_point_height_mode(fms::instruments::point::height_mode_t hm) override;
    void set_desired_height  (optional<double> height) override;
    void set_desired_CAS     (optional<double> CAS) override;
    void set_desired_ROCD    (optional<double> ROCD) override;

protected:
    void set_state          (state_t const& st, bool sure);

protected:
    void set_auto_transition(fms::transition_t i, bool isauto);
#if 1    
    meteo::meteo_proxy_ptr get_meteo_proxy() const;
#endif

protected:
    virtual void on_fms_settings_changed();
    virtual void on_plan_changed();

protected:
    virtual void on_state   (msg::state_msg const& msg);
    virtual void on_controls(fms::manual_controls_t const& ctrl);
    void on_plan(std::vector<binary::bytes_t> const& data);
    
    void on_kind    (string const& msg);
    void on_payload (double payload);
    void on_settings(settings_t const& settings);
    
    virtual void on_procedure_changed();
    virtual void on_assigned_fpl_changed();

    fpl::info_ptr get_fpl() const;
    fms::procedure_ptr get_procedure() const;
    fms::instruments::environment const& get_instruments_env() const;

private:
    void make_aircraft_info();

private:
    //! характеристики ВС
    ada::info_ptr               ada_;
    fms::procedure_model_ptr    procedure_model_;
    meteo::meteo_proxy_ptr      meteo_proxy_;
    //meteo::meteo_cursor_ptr     meteo_cursor_;

    //! 
    fms::instruments::environment instruments_env_;

    //! план полета
    fpl::info_ptr      fpl_;
    fms::procedure_ptr procedure_;

    //! 
    scoped_connection procedure_changed_conn_;
    scoped_connection assigned_fpl_changed_conn_;

    fms::manual_controls_t controls_;
    ani::airport_info_ptr closest_airport_;

    //! еще план
    fms::plan_t plan_;

protected:
    ani_object::info_ptr ani_;

};

}
}

