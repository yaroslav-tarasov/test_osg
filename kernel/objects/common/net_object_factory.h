#pragma once

namespace kernel
{
    using namespace net_layer::msg;

    obj_create_data pack_object      ( system* csys, create_msg const& msg);
    object_info_ptr create_object    ( system* csys, create_msg const& msg);
    object_info_ptr create_cloud_zone( system* csys, update_cloud_zone_msg const& msg);
}