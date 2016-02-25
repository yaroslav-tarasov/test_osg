#pragma once 

#include "network/msg_base.h"
using network::gen_msg;

#include "test_msg_enum.h"

// FIXME messages and namespaces

namespace net_layer
{

    namespace msg
    {

        typedef std::vector< network::endpoint > endpoints;

        struct vis_peers
            : network::msg_id<id_vis_peers>
        {
            vis_peers(const endpoints& eps = endpoints())
                : eps(eps)
            {
            }

            endpoints eps;
        };

        REFL_STRUCT(vis_peers)
            REFL_ENTRY(eps )
        REFL_END()

    }


}