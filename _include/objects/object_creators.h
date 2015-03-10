#pragma once

namespace aircraft
{
    using namespace kernel;
    
    object_info_ptr create(fake_objects_factory* sys,const aircraft::settings_t& sett);
}