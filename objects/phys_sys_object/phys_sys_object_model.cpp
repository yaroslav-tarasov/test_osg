#include "stdafx.h"
#include "precompiled_objects.h"

#include "phys_sys_object_model.h"


namespace phys
{
object_info_ptr model::create(kernel::object_create_t const& oc , dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

model::model(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    , last_time_(0)
    , exercise_loaded_connection_(oc.sys->subscribe_exercise_loaded(boost::bind(&model::on_exercise_loaded, this)))
{ 
    FIXME(Тоже магия)
    zones_.insert(0, zone_t(create_phys_system(), cg::geo_point_3(), "noname"));
}

model::~model()
{
    Assert(zones_.empty());
}

void model::update( double time )
{
    double dt = time - last_time_;

    double const calc_step = /*0.1*/0.01;
    size_t count = cg::max(1, cg::floor(dt / calc_step));
    double real_calc_step = dt / count;

FIXME(Ну очень интересная физика)
#if 1
    for (size_t i = 0; i < count; ++i)
    {
        for (auto it = zones_.begin(); it != zones_.end(); ++it)
            (*it).sys->update(real_calc_step);
    }
#else
    for (auto it = zones_.begin(); it != zones_.end(); ++it)
        (*it).sys->update(dt);
#endif

    last_time_ = time;
}

void model::on_object_created(object_info_ptr object)
{
    view::on_object_created(object);
    FIXME(Debug drawer)
/*    if (mdd::info_ptr mod_debug = object)
    {
        mod_debug_ = mod_debug;

        for (auto it = zones_.begin(); it != zones_.end(); ++it)
            it->sys->set_debug_renderer(mod_debug->get_renderer());
    }
    else*/ if (airport::info_ptr airport = object)
    {
        auto it = std::find_if(zones_.begin(), zones_.end(), airport_zone_predicate(airport->name()));
        if (it == zones_.end())
        {
            auto phys = create_phys_system();
            uint32_t id = zones_.next_id();
            zones_.insert(id, zone_t(phys, airport->pos(), airport->name()));
            
            FIXME(Debug drawer)
            //if (mod_debug_)
            //    phys->set_debug_renderer(mod_debug_->get_renderer());

            zone_created_signal_(id);
        }
    }
}

void model::on_object_destroying(object_info_ptr object)
{
    view::on_object_destroying(object);
    FIXME(Debug drawer)
/*    if (mod_debug_ == object)
    {
        for (auto it = zones_.begin(); it != zones_.end(); ++it)
            it->sys->set_debug_renderer();

        mod_debug_.reset();
    }
    else */if (airport::info_ptr airport = object)
    {
        auto it = std::find_if(zones_.begin(), zones_.end(), airport_zone_predicate(airport->name()));
        if (it != zones_.end())
        {
            uint32_t id = it.id();
            zones_.erase(id);
            zone_destroyed_signal_(id);
        }
    }

}

system_ptr model::get_system(size_t zone)
{
    FIXME(Тоже магия)
    zone = 0;
    
    return zones_[zone].sys;
}

optional<size_t> model::get_zone(geo_point_3 const & pos) const
{
    optional<size_t> zone;
    
    FIXME(Магическое число)  
        return 1;

    for (auto it = zones_.begin(); it != zones_.end(); ++it)
    {
        if (cg::distance(pos, it->base) < 10000)
        {
            if (!zone || cg::distance(pos, it->base) < cg::distance(pos, zones_[*zone].base))
                zone = it.id();
        }
    }

    return zone;
}

optional<size_t> model::get_zone(std::string const& airport) const
{
    FIXME(Магическое число)  
        return 1;

    auto it = std::find_if(zones_.begin(), zones_.end(), airport_zone_predicate(airport));
    return it != zones_.end() ? optional<size_t>(std::distance(zones_.begin(), it)) : boost::none;
}

geo_base_3 const& model::get_base(size_t zone) const
{
    static auto _base = ::get_base();
    FIXME(Магическое число)  
        return _base;

    return zones_[zone].base;
}

string model::zone_name(size_t id) const
{
    return zones_[id].airport;
}

void model::on_exercise_loaded()
{
    for (auto it = zones_.begin(); it != zones_.end(); ++it)
        it->sys->update(0.1);
}


AUTO_REG_NAME(phys_sys_model, model::create);

} // phys_sys
