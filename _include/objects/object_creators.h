#pragma once

namespace aircraft
{
    using namespace kernel;
    
    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const /*cg::geo_point_3*/geo_position& init_pos);
}

namespace vehicle
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const /*cg::geo_point_3*/geo_position& init_pos);
}

namespace simple_route
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const cg::geo_point_3& init_pos);
}

namespace aircraft_physless
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const aircraft::settings_t& sett,const /*cg::geo_point_3*/geo_position& init_pos);
}


namespace airport
{

    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett);
}