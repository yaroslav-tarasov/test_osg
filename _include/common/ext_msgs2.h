#pragma once 

#include "network/msg_base.h"
using network::gen_msg;

#include "ext_msg_enum.h"

// FIXME messages and namespaces

namespace net_layer
{

    namespace msg
    {

        typedef std::vector< network::endpoint > endpoints;

        struct vis_peers_msg
            : network::msg_id<id_vis_peers>
        {
            vis_peers_msg(const endpoints& eps = endpoints())
                : eps(eps)
            {
            }

            endpoints eps;
        };

        REFL_STRUCT(vis_peers_msg)
            REFL_ENTRY(eps )
        REFL_END()

    }


}