#pragma once

namespace aircraft
{
    using namespace kernel;
    
    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett);
}

namespace vehicle
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett);
}
